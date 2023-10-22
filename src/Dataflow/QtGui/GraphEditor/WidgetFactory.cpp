#include <Dataflow/QtGui/GraphEditor/WidgetFactory.hpp>

#include <Dataflow/QtGui/GraphEditor/GraphEditorView.hpp>

#include <Core/Utils/Color.hpp>
#include <Gui/Widgets/ControlPanel.hpp>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Enumerator.hpp>

#include <Engine/Data/EnvironmentTexture.hpp>
#include <PowerSlider/PowerSlider.hpp>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {

using namespace Ra::Dataflow::Core;
using namespace Ra::Gui::Widgets;

using namespace Ra::Engine::Data;

namespace WidgetFactory {
using WidgetFunctionPair = std::pair<WidgetCreatorFunc, WidgetUpdaterFunc>;

std::unordered_map<std::type_index, WidgetFunctionPair> widgetsfunctions;

void registerWidgetInternal( std::type_index typeIdx,
                             WidgetCreatorFunc widgetCreator,
                             WidgetUpdaterFunc widgetUpdater ) {
    if ( widgetsfunctions.find( typeIdx ) == widgetsfunctions.end() ) {
        widgetsfunctions[typeIdx] = { std::move( widgetCreator ), std::move( widgetUpdater ) };
    }
    else {
        LOG( Ra::Core::Utils::logWARNING )
            << "WidgetFactory: trying to add an already existing widget builder for type "
            << simplifiedDemangledType( typeIdx ) << ".";
    }
}

QWidget* createWidget( EditableParameterBase* editableParameter ) {
    if ( widgetsfunctions.find( editableParameter->getType() ) != widgetsfunctions.end() ) {
        return widgetsfunctions[editableParameter->getType()].first( editableParameter );
    }
    else {
        // TODO, when PR #1027 will be merged, demangle the type name
        LOG( Ra::Core::Utils::logWARNING )
            << "WidgetFactory : no defined widget builder for type "
            << simplifiedDemangledType( editableParameter->getType() ) << ".";
    }
    return nullptr;
}

bool updateWidget( QWidget* widget, EditableParameterBase* editableParameter ) {
    if ( widgetsfunctions.find( editableParameter->getType() ) != widgetsfunctions.end() ) {
        return widgetsfunctions[editableParameter->getType()].second( widget, editableParameter );
    }
    else {
        LOG( Ra::Core::Utils::logWARNING )
            << "WidgetFactory: no defined widget updater for type "
            << simplifiedDemangledType( editableParameter->getType() ) << ".";
    }
    return false;
}

void initializeWidgetFactory() {
    /*
     * Environment map "edition" widget
     */
    WidgetFactory::registerWidget<std::shared_ptr<EnvironmentTexture>>(
        []( EditableParameterBase* editableParameter ) {
            auto editable = dynamic_cast<EditableParameter<std::shared_ptr<EnvironmentTexture>>*>(
                editableParameter );

            auto controlPanel = new ControlPanel( editable->getName(), false );
            auto envmpClbck   = [editable, controlPanel]( const std::string& files ) {
                if ( files.empty() ) { editable->m_data = nullptr; }
                else {
                    // for now, only skyboxes are managed
                    editable->m_data = std::make_shared<EnvironmentTexture>( files, true );
                    auto slider      = controlPanel->findChild<PowerSlider*>( "strength" );
                    if ( slider ) { editable->m_data->setStrength( slider->value() / 100. ); }
                }
            };
            // TODO : display the name of the envmap image somewhere
            controlPanel->addFileInput(
                "files", envmpClbck, "../", "Images (*.png *.jpg *.pfm *.exr *hdr)" );

            auto strengthClbk = [editable]( double v ) {
                if ( editable->m_data ) {
                    auto* env = editable->m_data.get();
                    if ( env ) { env->setStrength( v / 100. ); }
                }
            };
            float s_init = 100.;
            if ( editable->m_data ) { s_init = editable->m_data->getStrength() * 100.; }
            controlPanel->addPowerSliderInput( "strength", strengthClbk, s_init, 0., 100 );
            controlPanel->setVisible( true );
            return controlPanel;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<std::shared_ptr<EnvironmentTexture>>*>(
                editableParameter );
            auto slider = widget->findChild<PowerSlider*>( "strength" );
            if ( slider && editable->m_data ) {
                slider->setValue( editable->m_data->getStrength() * 100. );
                return true;
            }
            else { return slider != nullptr; }
        } );

    /*
     * Scalar edition
     */
    WidgetFactory::registerWidget<Scalar>(
        []( EditableParameterBase* editableParameter ) {
            auto editable    = dynamic_cast<EditableParameter<Scalar>*>( editableParameter );
            auto powerSlider = new PowerSlider();
            powerSlider->setObjectName( editable->getName().c_str() );
            // editable->m_data = 0.0_ra;
            powerSlider->setValue( editable->m_data );
            const auto& constraints = editable->getConstraints();
            Scalar minValue         = 0_ra;
            Scalar maxValue         = 1000_ra;
            if ( constraints.contains( "min" ) ) { minValue = Scalar( constraints["min"] ); }
            if ( constraints.contains( "max" ) ) { maxValue = Scalar( constraints["max"] ); }
            powerSlider->setRange( minValue, maxValue );
            PowerSlider::connect( powerSlider,
                                  &PowerSlider::valueChanged,
                                  [editable]( Scalar value ) { editable->m_data = value; } );
            return powerSlider;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<Scalar>*>( editableParameter );
            auto slider   = widget->findChild<PowerSlider*>( editableParameter->getName().c_str() );
            if ( slider ) {
                slider->setValue( editable->m_data );
                return true;
            }
            else { return false; }
        } );
    /*
     * int edition
     */
    WidgetFactory::registerWidget<int>(
        []( EditableParameterBase* editableParameter ) {
            auto editable           = dynamic_cast<EditableParameter<int>*>( editableParameter );
            const auto& constraints = editable->getConstraints();
            Scalar minValue         = 0_ra;
            Scalar maxValue         = 1000_ra;
            if ( constraints.contains( "min" ) ) { minValue = Scalar( constraints["min"] ); }
            if ( constraints.contains( "max" ) ) { maxValue = Scalar( constraints["max"] ); }

            auto powerSlider = new PowerSlider();
            powerSlider->setObjectName( editable->getName().c_str() );
            // editable->m_data = 0.0_ra;
            powerSlider->setValue( editable->m_data );
            powerSlider->setRange( minValue, maxValue );
            powerSlider->setSingleStep( 1 );
            PowerSlider::connect( powerSlider,
                                  &PowerSlider::valueChanged,
                                  [editable]( Scalar value ) { editable->m_data = int( value ); } );
            return powerSlider;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<int>*>( editableParameter );
            auto slider   = widget->findChild<PowerSlider*>( editableParameter->getName().c_str() );
            if ( slider ) {
                slider->setValue( editable->m_data );
                return true;
            }
            else { return false; }
        } );
    /*
     * Boolean edition
     */
    WidgetFactory::registerWidget<bool>(
        []( EditableParameterBase* editableParameter ) {
            auto editable = dynamic_cast<EditableParameter<bool>*>( editableParameter );
            auto checkBox = new QCheckBox();
            checkBox->setObjectName( editable->getName().c_str() );
            checkBox->setCheckState( editable->m_data ? Qt::CheckState::Checked
                                                      : Qt::CheckState::Unchecked );
            QCheckBox::connect( checkBox, &QCheckBox::stateChanged, [editable]( int state ) {
                if ( state == Qt::Unchecked ) { editable->m_data = false; }
                else if ( state == Qt::Checked ) { editable->m_data = true; }
            } );
            return checkBox;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<bool>*>( editableParameter );
            auto checkBox = widget->findChild<QCheckBox*>( editableParameter->getName().c_str() );
            if ( checkBox ) {
                checkBox->setCheckState( editable->m_data ? Qt::Checked : Qt::Unchecked );
                return true;
            }
            else { return false; }
        } );

    /*
     * Color edition
     */
    WidgetFactory::registerWidget<Ra::Core::Utils::Color>(
        []( EditableParameterBase* editableParameter ) {
            auto editable =
                dynamic_cast<EditableParameter<Ra::Core::Utils::Color>*>( editableParameter );
            auto controlPanel = new ControlPanel( editable->getName(), false );
            auto clrCbk       = [editable]( const Ra::Core::Utils::Color& clr ) {
                editable->m_data = clr;
            };
            controlPanel->addColorInput( editable->getName(), clrCbk, editable->m_data, true );
            controlPanel->setVisible( true );
            return controlPanel;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable =
                dynamic_cast<EditableParameter<Ra::Core::Utils::Color>*>( editableParameter );
            auto button = widget->findChild<QPushButton*>( editable->getName().c_str() );
            if ( button ) {
                // todo, update the color on the button and make the dialog to take its
                //  initial color from the button. Once dont, return true ...
                auto srgbColor = Ra::Core::Utils::Color::linearRGBTosRGB( editable->m_data );
                auto clrBttn =
                    QColor::fromRgbF( srgbColor[0], srgbColor[1], srgbColor[2], srgbColor[3] );

                auto lum = 0.2126_ra * Scalar( clrBttn.redF() ) +
                           0.7151_ra * Scalar( clrBttn.greenF() ) +
                           0.0721_ra * Scalar( clrBttn.blueF() );
                QString qss = QString( "background-color: %1" ).arg( clrBttn.name() );
                if ( lum > 1_ra / 3_ra ) { qss += QString( "; color: #000000" ); }
                else { qss += QString( "; color: #FFFFFF" ); }
                button->setStyleSheet( qss );
                return true;
            }
            LOG( Ra::Core::Utils::logWARNING )
                << " Unable to find the button \"Choose color\" for \"" << editable->getName()
                << "\" ";
            return false;
        } );

    /*
     * Button edition
     */
    WidgetFactory::registerWidget<DataflowGraph>(
        []( EditableParameterBase* editableParameter ) {
            auto editable = dynamic_cast<EditableParameter<DataflowGraph>*>( editableParameter );
            auto button   = new QPushButton( "Show graph" );
            button->setObjectName( editableParameter->getName().c_str() );
            QPushButton::connect( button, &QPushButton::clicked, [editable]() {
                // Display a window to see the graph
                /*auto graph = reinterpret_cast<DataflowGraph*>( editable );
                auto nodeEditor          = new GraphEditorView( nullptr );

                nodeEditor->editGraph( nullptr );
                nodeEditor->editGraph( graph );

                nodeEditor->resize( 900, 500 );
                nodeEditor->move( 450, 260 );
                nodeEditor->show(); */
            } );

            return button;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<DataflowGraph>*>( editableParameter );
            auto button   = widget->findChild<QPushButton*>( editable->getName().c_str() );
            if ( button ) { return true; }
            else { return false; }
        } );

#if HAS_TRANSFER_FUNCTION
    /*
     * Transfer function
     */
    WidgetFactory::registerWidget<std::array<float, 256 * 4>>(
        []( EditableParameterBase* editableParameter ) {
            auto editableTransferFunction =
                dynamic_cast<EditableParameter<std::array<float, 256 * 4>>*>( editableParameter );
            // TODO, give a name to the widget so that they can be updated automatically when
            // loading a node
            auto button         = new QPushButton( "Open widget" );
            auto transferEditor = new TransferEditor();
            TransferEditor::connect(
                transferEditor,
                &TransferEditor::propertiesChanged,
                [editableTransferFunction, transferEditor]() {
                    int pos = 0;
                    for ( int i = 0; i < 256; i++ ) {
                        unsigned int color = transferEditor->colorAt( i );
                        editableTransferFunction->m_data.at( pos ) =
                            (unsigned char)( ( 0x00ff0000 & color ) >> 16 ) / 255.f;
                        editableTransferFunction->m_data.at( pos + 1 ) =
                            (unsigned char)( ( 0x0000ff00 & color ) >> 8 ) / 255.f;
                        editableTransferFunction->m_data.at( pos + 2 ) =
                            (unsigned char)( ( 0x000000ff & color ) ) / 255.f;
                        editableTransferFunction->m_data.at( pos + 3 ) =
                            (unsigned char)( ( 0xff000000 & color ) >> 24 ) / 255.f;
                        pos = pos + 4;
                    }
                } );
            QPushButton::connect(
                button, &QPushButton::clicked, [transferEditor]() { transferEditor->show(); } );
            return button;
        },
        []( QWidget*, EditableParameterBase* ) -> bool { return false; } );
#endif
    /*
     * String enumerator
     */
    WidgetFactory::registerWidget<Enumerator<std::string>>(
        []( EditableParameterBase* editableParameter ) {
            auto editable =
                dynamic_cast<EditableParameter<Enumerator<std::string>>*>( editableParameter );
            auto selector = new QComboBox();
            selector->setObjectName( editable->getName().c_str() );
            for ( const auto& e : editable->m_data ) {
                selector->addItem( e.c_str() );
            }
            QComboBox::connect(
                selector, &QComboBox::currentTextChanged, [editable]( const QString& string ) {
                    editable->m_data.set( string.toStdString() );
                } );
            return selector;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable =
                dynamic_cast<EditableParameter<Enumerator<std::string>>*>( editableParameter );
            auto comboBox = widget->findChild<QComboBox*>( editable->getName().c_str() );
            if ( comboBox ) {
                comboBox->setCurrentText( editable->m_data.get().c_str() );
                return true;
            }
            else { return false; }
        } );

    /*
     * String
     */
    WidgetFactory::registerWidget<std::string>(
        []( EditableParameterBase* editableParameter ) {
            auto editable = dynamic_cast<EditableParameter<std::string>*>( editableParameter );
            auto line     = new QLineEdit();
            line->setObjectName( editable->getName().c_str() );
            QLineEdit::connect( line, &QLineEdit::textEdited, [editable]( const QString& string ) {
                editable->m_data = string.toStdString();
            } );
            return line;
        },
        []( QWidget* widget, EditableParameterBase* editableParameter ) -> bool {
            auto editable = dynamic_cast<EditableParameter<std::string>*>( editableParameter );
            auto line     = widget->findChild<QLineEdit*>( editable->getName().c_str() );
            if ( line ) {
                line->setText( editable->m_data.c_str() );
                return true;
            }
            else { return false; }
        } );
    /*
     * Ra::Engine::Data::ShaderConfiguration --> Code Editor
     */
}

} // namespace WidgetFactory

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
