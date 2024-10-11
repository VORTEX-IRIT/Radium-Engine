#include <Gui/ParameterSetEditor/ParameterSetEditor.hpp>

#include <Core/Containers/VariableSetEnumManagement.hpp>
#include <Engine/Data/Material.hpp>
#include <Gui/Widgets/ControlPanel.hpp>

#include <QString>
#include <QWidget>

#include <limits>
#include <memory>
#include <type_traits>

using json = nlohmann::json;

namespace Ra {
using namespace Engine;
namespace Gui {

namespace internal {
class RenderParameterUiBuilder
{
  public:
    using types = Engine::Data::RenderParameters::BindableTypes;

    RenderParameterUiBuilder( VariableSetEditor* pse, const json& constraints ) :
        m_pse { pse }, m_constraints { constraints } {}

    void operator()( const std::string& name, bool& p, Core::VariableSet&& /* params */ ) {
        auto onBoolParameterChanged = [pse = this->m_pse, &p, nm = name]( bool val ) {
            p = val;
            emit pse->parameterModified( nm );
        };
        if ( m_constraints.contains( name ) ) {
            if ( m_constraints[name]["editable"] ) {
                const auto& m           = m_constraints[name];
                std::string description = m.contains( "description" ) ? m["description"] : "";
                std::string nm          = m.contains( "name" ) ? std::string { m["name"] } : name;
                m_pse->addOption( nm, onBoolParameterChanged, p, description );
            }
        }
        else if ( m_pse->showUnspecified() ) {
            m_pse->addOption( name, onBoolParameterChanged, p );
        }
    }

    template <typename TParam, std::enable_if_t<std::is_arithmetic<TParam>::value, bool> = true>
    void operator()( const std::string& name, TParam& p, Core::VariableSet&& params ) {
        using namespace Ra::Core::VariableSetEnumManagement;
        if ( getEnumConverter<TParam>( params, name ) ) {
            m_pse->addEnumWidget( name, p, params, m_constraints );
        }
        else {
            // case number
            m_pse->addNumberWidget( name, p, params, m_constraints );
        }
    }

    template <typename TParam,
              typename TAllocator,
              std::enable_if_t<std::is_arithmetic<TParam>::value, bool> = true>
    void operator()( const std::string& name,
                     std::vector<TParam, TAllocator>& p,
                     Core::VariableSet&& params ) {
        m_pse->addVectorWidget( name, p, params, m_constraints );
    }

    void
    operator()( const std::string& name, Ra::Core::Utils::Color& p, Core::VariableSet&& params ) {
        auto onColorParameterChanged =
            [pse = this->m_pse, &params, nm = name]( const Ra::Core::Utils::Color& val ) {
                params.setVariable( nm, val );
                emit pse->parameterModified( nm );
            };
        if ( m_constraints.contains( name ) ) {
            const auto& m           = m_constraints[name];
            std::string description = m.contains( "description" ) ? m["description"] : "";
            std::string nm          = m.contains( "name" ) ? std::string { m["name"] } : name;
            m_pse->addColorInput( nm, onColorParameterChanged, p, m["maxItems"] == 4, description );
        }
        else if ( m_pse->showUnspecified() ) {
            m_pse->addColorInput( name, onColorParameterChanged, p );
        }
    }

    template <template <typename, int...> typename M, typename T, int... dim>
    void operator()( const std::string& name, M<T, dim...>& p, Core::VariableSet&& params ) {
        m_pse->addMatrixWidget( name, p, params, m_constraints );
    }

    void operator()( const std::string& /*name*/,
                     Data::RenderParameters::TextureInfo& /*p*/,
                     Core::VariableSet&& /*params*/ ) {
        // textures are not yet editable
    }

    template <typename T>
    void operator()( const std::string& name,
                     std::reference_wrapper<T>& p,
                     Core::VariableSet&& /*params*/ ) {
        m_pse->addLabel( name );
        if constexpr ( std::is_assignable_v<Core::VariableSet, typename std::decay<T>::type> ) {
            if constexpr ( std::is_const_v<T> ) {
                p.get().visit( *this,
                               const_cast<Core::VariableSet&>(
                                   static_cast<const Core::VariableSet&>( p.get() ) ) );
            }
            else { p.get().visit( *this, static_cast<Core::VariableSet&>( p.get() ) ); }
        }
    }

  private:
    VariableSetEditor* m_pse { nullptr };
    const json& m_constraints;
};
} // namespace internal

VariableSetEditor::VariableSetEditor( const std::string& name, QWidget* parent ) :
    ControlPanel( name, !name.empty(), parent ) {}

template <typename T>
void VariableSetEditor::addEnumWidget( const std::string& key,
                                       T& initial,
                                       Core::VariableSet& params,
                                       const json& metadata ) {
    using namespace Ra::Core::VariableSetEnumManagement;
    auto m = metadata[key];

    std::string description = m.contains( "description" ) ? m["description"] : "";
    std::string nm          = m.contains( "name" ) ? std::string { m["name"] } : key;
    if ( auto ec = getEnumConverter<T>( params, key ) ) {
        auto items                        = ( *ec )->getEnumerators();
        auto onEnumParameterStringChanged = [this, &params, &key]( const QString& value ) {
            params.setVariable( key, value.toStdString() );
            emit parameterModified( key );
        };
        addComboBox( nm,
                     onEnumParameterStringChanged,
                     getEnumString( params, key, initial ),
                     items,
                     description );
    }
    else {
        LOG( Core::Utils::logWARNING )
            << "ParameterSet don't have converter for enum " << key << " use index<>int instead.";

        auto items = std::vector<std::string>();
        items.reserve( m["values"].size() );
        for ( const auto& value : m["values"] ) {
            items.push_back( value );
        }

        auto onEnumParameterIntChanged = [this, &params, &key]( T value ) {
            params.setVariable( key, value );
            emit parameterModified( key );
        };
        addComboBox( nm, onEnumParameterIntChanged, initial, items, description );
    }
}

template <typename T>
void VariableSetEditor::addNumberWidget( const std::string& key,
                                         T& initial,
                                         Core::VariableSet& /*params*/,
                                         const json& metadata ) {

    auto onNumberParameterChanged = [this, &initial, &key]( T value ) {
        initial = value;
        emit parameterModified( key );
    };
    if ( metadata.contains( key ) ) {
        auto m   = metadata[key];
        auto min = std::numeric_limits<T>::lowest();
        auto max = std::numeric_limits<T>::max();

        std::string description = m.contains( "description" ) ? m["description"] : "";
        std::string nm          = m.contains( "name" ) ? std::string { m["name"] } : key;

        if ( m.contains( "oneOf" ) ) {
            // the variable has multiple valid bounds
            std::vector<std::pair<T, T>> bounds;
            bounds.reserve( m["oneOf"].size() );
            for ( const auto& bound : m["oneOf"] ) {
                auto mini = bound.contains( "minimum" ) ? T( bound["minimum"] ) : min;
                auto maxi = bound.contains( "maximum" ) ? T( bound["maximum"] ) : max;
                bounds.emplace_back( mini, maxi );
            }
            auto predicate = [bounds]( T value ) {
                bool valid = false;
                auto it    = bounds.begin();
                while ( !valid && it != bounds.end() ) {
                    valid = value >= ( *it ).first && value <= ( *it ).second;
                    ++it;
                }
                return valid;
            };

            addConstrainedNumberInput<T>(
                nm, onNumberParameterChanged, initial, predicate, description );
        }
        else if ( m.contains( "minimum" ) && m.contains( "maximum" ) ) {
            min = T( m["minimum"] );
            max = T( m["maximum"] );
            if constexpr ( std::is_floating_point_v<T> ) {
                addPowerSliderInput( nm, onNumberParameterChanged, initial, min, max, description );
            }
            else { addSliderInput( nm, onNumberParameterChanged, initial, min, max, description ); }
        }
        else {
            min = m.contains( "minimum" ) ? T( m["minimum"] ) : min;
            max = m.contains( "maximum" ) ? T( m["maximum"] ) : max;
            addNumberInput<T>( nm, onNumberParameterChanged, initial, min, max, description );
        }
    }
    else if ( m_showUnspecified ) { addNumberInput<T>( key, onNumberParameterChanged, initial ); }
}

template <typename T>
void VariableSetEditor::addVectorWidget( const std::string& key,
                                         std::vector<T>& initial,
                                         Core::VariableSet& /*params*/,
                                         const json& metadata ) {
    auto onVectorParameterChanged = [this, &initial, &key]( const std::vector<T>& value ) {
        initial = value;
        emit parameterModified( key );
    };

    if ( metadata.contains( key ) ) {
        auto m                  = metadata[key];
        std::string description = m.contains( "description" ) ? m["description"] : "";
        addVectorInput<T>( m["name"], onVectorParameterChanged, initial, description );
    }
    else if ( m_showUnspecified ) { addVectorInput<T>( key, onVectorParameterChanged, initial ); }
}

template <typename T>
void VariableSetEditor::addMatrixWidget( const std::string& key,
                                         T& initial,
                                         Core::VariableSet& /*params*/,
                                         const json& metadata ) {
    auto onMatrixParameterChanged = [this, &initial, &key]( const Ra::Core::MatrixN& value ) {
        initial = T( value );
        emit parameterModified( key );
    };

    if ( metadata.contains( key ) ) {
        auto m                  = metadata[key];
        std::string description = m.contains( "description" ) ? m["description"] : "";
        addMatrixInput( m["name"], onMatrixParameterChanged, initial, 3, description );
    }
    else if ( m_showUnspecified ) { addMatrixInput( key, onMatrixParameterChanged, initial ); }
}

void VariableSetEditor::setupUi( Core::VariableSet& params, const nlohmann::json& constraints ) {

    internal::RenderParameterUiBuilder uiBuilder { this, constraints };
    params.visit( uiBuilder, params );
    addStretch( 0 );
    setVisible( true );
}

} // namespace Gui
} // namespace Ra
