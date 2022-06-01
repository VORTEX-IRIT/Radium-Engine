#include "MaterialParameterEditor.hpp"

// include the Material Definition
#include <Data/RenderParameters.hpp>
#include <Engine/Data/Material.hpp>

#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QString>
#include <QWidget>

#include <limits>
#include <memory>

namespace Ra::Gui {

MaterialParameterEditor::MaterialParameterEditor( QWidget* parent ) : QWidget( parent ) {
    // setup the GUI
    auto verticalLayout = new QVBoxLayout( this );
    auto matInfoGroup   = new QGroupBox();
    matInfoGroup->setTitle( "Selected Material Informations" );
    matInfoGroup->setCheckable( false );
    auto matInfoForm = new QFormLayout( matInfoGroup );

    m_matNameLabel         = new QLabel();
    m_matInstanceNameLabel = new QLabel();
    m_matProperties        = new QPlainTextEdit();
    m_matProperties->setReadOnly( true );

    matInfoForm->addRow( tr( "Material Instance Name :" ), m_matInstanceNameLabel );
    matInfoForm->addRow( tr( "Material Name :" ), m_matNameLabel );
    matInfoForm->addRow( tr( "Material Properties :" ), m_matProperties );

    matInfoGroup->setMaximumHeight( 150 );
    verticalLayout->addWidget( matInfoGroup );

    auto matParamsGroup = new QGroupBox();
    matParamsGroup->setTitle( "Edit Material Parameters" );
    matParamsGroup->setEnabled( true );
    matParamsGroup->setCheckable( false );

    verticalLayout->addWidget( matParamsGroup );
    m_matParamsLayout        = new QVBoxLayout( matParamsGroup );
    m_parametersControlPanel = new Widgets::ControlPanel( "Material Parameters", false );
    m_matParamsLayout->addWidget( m_parametersControlPanel );
}

using json = nlohmann::json;

template <typename T>
void MaterialParameterEditor::addEnumParameterWidget( const std::string& key,
                                                      T initial,
                                                      Ra::Engine::Data::RenderParameters& params,
                                                      const json& metadata ) {
    auto m     = metadata[key];
    auto items = std::vector<std::string>();
    items.reserve( m["values"].size() );
    for ( const auto& value : m["values"] ) {
        items.push_back( value );
    }
    auto onEnumParameterChanged = [this, &params, &key]( T value ) {
        params.addParameter( key, value );
        emit materialParametersModified();
    };
    std::string description = m.contains( "description" ) ? m["description"] : "";
    m_parametersControlPanel->addComboBox(
        m["name"], onEnumParameterChanged, initial, items, description );
}

template <typename T>
void MaterialParameterEditor::addNumberParameterWidget( const std::string& key,
                                                        T initial,
                                                        Ra::Engine::Data::RenderParameters& params,
                                                        const json& metadata ) {
    auto onNumberParameterChanged = [this, &params, &key]( double value ) {
        params.addParameter( key, T( value ) );
        emit materialParametersModified();
    };
    if ( metadata.contains( key ) ) {
        auto m                  = metadata[key];
        T min                   = std::numeric_limits<T>::min();
        T max                   = std::numeric_limits<T>::max();
        int dec                 = std::numeric_limits<T>::is_integer ? 0 : 3;
        std::string description = m.contains( "description" ) ? m["description"] : "";

        if ( m.contains( "oneOf" ) ) {
            // the variable has mutiple valid bounds
            auto bounds = std::vector<std::pair<T, T>>();
            bounds.reserve( m["oneOf"].size() );
            for ( const auto& bound : m["oneOf"] ) {
                int mini = bound.contains( "minimum" ) ? T( bound["minimum"] ) : min;
                int maxi = bound.contains( "maximum" ) ? T( bound["maximum"] ) : max;
                bounds.push_back( std::pair( mini, maxi ) );
            }
            auto predicate = [bounds]( double value ) {
                bool valid = false;
                auto it    = bounds.begin();
                while ( !valid && it != bounds.end() ) {
                    valid = value >= ( *it ).first && value <= ( *it ).second;
                    ++it;
                }
                return valid;
            };
            m_parametersControlPanel->addCheckingScalarInput(
                m["name"], onNumberParameterChanged, initial, predicate, dec, description );
        }
        else if ( m.contains( "minimum" ) && m.contains( "maximum" ) ) {
            min = m["minimum"] > min ? T( m["minimum"] ) : min;
            max = m["maximum"] < max ? T( m["maximum"] ) : max;
            m_parametersControlPanel->addPowerSliderInput(
                m["name"], onNumberParameterChanged, initial, min, max, description );
        }
        else {
            min = m.contains( "minimum" ) ? T( m["minimum"] ) : min;
            max = m.contains( "maximum" ) ? T( m["maximum"] ) : max;
            m_parametersControlPanel->addScalarInput(
                m["name"], onNumberParameterChanged, initial, min, max, dec, description );
        }
    }
    else if ( m_showUnspecified ) {
        m_parametersControlPanel->addScalarInput( key, onNumberParameterChanged, initial );
    }
}

template <typename T>
void MaterialParameterEditor::addVectorParameterWidget( const std::string& key,
                                                        const std::vector<T>& initial,
                                                        Ra::Engine::Data::RenderParameters& params,
                                                        const json& metadata ) {
    auto onVectorParameterChanged = [this, &params, &key]( const std::vector<double>& value ) {
        std::vector<T> vecT( value.begin(), value.end() );
        params.addParameter( key, vecT );
        emit materialParametersModified();
    };

    std::vector<double> init( initial.begin(), initial.end() );
    if ( metadata.contains( key ) ) {
        auto m                  = metadata[key];
        int dec                 = std::numeric_limits<T>::is_integer ? 0 : 3;
        std::string description = m.contains( "description" ) ? m["description"] : "";

        m_parametersControlPanel->addVectorInput(
            m["name"], onVectorParameterChanged, init, dec, description );
    }
    else if ( m_showUnspecified ) {
        m_parametersControlPanel->addVectorInput( key, onVectorParameterChanged, init );
    }
}

template <typename T>
void MaterialParameterEditor::addMatrixParameterWidget( const std::string& key,
                                                        const T& initial,
                                                        Ra::Engine::Data::RenderParameters& params,
                                                        const json& metadata ) {
    auto onMatrixParameterChanged = [this, &params, &key]( const Ra::Core::MatrixN& value ) {
        auto mat3 = T( value );
        params.addParameter( key, mat3 );
        emit materialParametersModified();
    };

    if ( metadata.contains( key ) ) {
        auto m                  = metadata[key];
        std::string description = m.contains( "description" ) ? m["description"] : "";

        m_parametersControlPanel->addMatrixInput(
            m["name"], onMatrixParameterChanged, initial, 3, description );
    }
    else if ( m_showUnspecified ) {
        m_parametersControlPanel->addMatrixInput( key, onMatrixParameterChanged, initial );
    }
}

void MaterialParameterEditor::setupFromMaterial(
    std::shared_ptr<Ra::Engine::Data::Material> material ) {
    auto editable = std::dynamic_pointer_cast<Ra::Engine::Data::EditableMaterial>( material );
    m_matInstanceNameLabel->setText( QString::fromStdString( material->getInstanceName() ) );
    m_matNameLabel->setText( QString::fromStdString( material->getMaterialName() ) );
    m_matProperties->clear();
    for ( auto& prop : material->getPropertyList() ) {
        m_matProperties->appendPlainText( QString::fromStdString( prop ) );
    }
    // move the cursor to the top
    auto tmpCursor = m_matProperties->textCursor();
    tmpCursor.movePosition( QTextCursor::Start );
    m_matProperties->setTextCursor( tmpCursor );

    auto& params  = material->getParameters();
    auto metadata = editable->getParametersMetadata();

    // clear the old control panel
    m_matParamsLayout->removeWidget( m_parametersControlPanel );
    delete m_parametersControlPanel;
    m_parametersControlPanel = new Widgets::ControlPanel( "Material Parameters", false );
    m_matParamsLayout->addWidget( m_parametersControlPanel );

    m_parametersControlPanel->beginLayout( QBoxLayout::TopToBottom );

    // Add widgets to edit bool parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::BoolParameter>() ) {
        auto onBoolParameterChanged = [this, &params, key = key]( bool val ) {
            params.addParameter( key, val );
            emit materialParametersModified();
        };
        if ( metadata.contains( key ) ) {
            if ( metadata[key]["editable"] ) {
                const auto& m           = metadata[key];
                std::string description = m.contains( "description" ) ? m["description"] : "";
                m_parametersControlPanel->addOption(
                    m["name"], onBoolParameterChanged, value.m_value, description );
            }
        }
        else if ( m_showUnspecified ) {
            m_parametersControlPanel->addOption( key, onBoolParameterChanged, value.m_value );
        }
    }

    // Add widgets to edit int parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::IntParameter>() ) {
        if ( metadata.contains( key ) && metadata[key]["type"] == "enum" ) {
            addEnumParameterWidget( key, value.m_value, params, metadata );
        }
        else {
            // case number
            addNumberParameterWidget( key, value.m_value, params, metadata );
        }
    }

    // Add widgets to edit unsigned parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::UIntParameter>() ) {
        if ( metadata.contains( key ) && metadata[key]["type"] == "enum" ) {
            addEnumParameterWidget( key, value.m_value, params, metadata );
        }
        else {
            // case number
            addNumberParameterWidget( key, value.m_value, params, metadata );
        }
    }

    // Add widgets to edit scalar parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::ScalarParameter>() ) {
        addNumberParameterWidget( key, value.m_value, params, metadata );
    }

    // Add widgets to edit color parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::ColorParameter>() ) {
        auto onColorParameterChanged =
            [this, &params, key = key, &value = value]( const Ra::Core::Utils::Color& val ) {
                auto color    = Ra::Core::Utils::Color( val );
                color.alpha() = value.m_value.alpha();
                params.addParameter( key, color );
                emit materialParametersModified();
            };
        auto onAlphaChanged = [this, &params, key = key, &value = value]( Scalar val ) {
            auto color    = Ra::Core::Utils::Color( value.m_value );
            color.alpha() = val;
            params.addParameter( key, color );
            emit materialParametersModified();
        };
        if ( metadata.contains( key ) ) {
            const auto& m           = metadata[key];
            std::string description = m.contains( "description" ) ? m["description"] : "";
            m_parametersControlPanel->addColorInput(
                m["name"], onColorParameterChanged, value.m_value, description );
            if ( m["maxItems"] == 4 ) {
                m_parametersControlPanel->addPowerSliderInput(
                    "alpha", onAlphaChanged, value.m_value.alpha(), 0.0, 1.0 );
            }
        }
        else if ( m_showUnspecified ) {
            m_parametersControlPanel->addColorInput( key, onColorParameterChanged, value.m_value );
            m_parametersControlPanel->addPowerSliderInput(
                "alpha", onAlphaChanged, value.m_value.alpha(), 0.0, 1.0 );
        }
    }

    // populate widgets for Ints parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::IntsParameter>() ) {
        addVectorParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for UInts parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::UIntsParameter>() ) {
        addVectorParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Scalars parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::ScalarsParameter>() ) {
        addVectorParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Vec2 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Vec2Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Vec3 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Vec3Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Vec4 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Vec4Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Mat2 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Mat2Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Mat3 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Mat3Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    // populate widgets for Mat4 parameters
    for ( auto& [key, value] :
          params.getParameterSet<Ra::Engine::Data::RenderParameters::Mat4Parameter>() ) {
        addMatrixParameterWidget( key, value.m_value, params, metadata );
    }

    m_parametersControlPanel->addStretch();
    m_parametersControlPanel->endLayout();
    m_parametersControlPanel->setVisible( true );
}

void MaterialParameterEditor::showUnspecified( bool enable ) {
    m_showUnspecified = enable;
}
} // namespace Ra::Gui
