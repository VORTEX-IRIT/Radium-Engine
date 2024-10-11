#pragma once
#include <Gui/RaGui.hpp>

#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <nlohmann/json.hpp>

#include <Gui/ParameterSetEditor/ParameterSetEditor.hpp>

namespace Ra {
namespace Engine {
namespace Data {
class Material;
class RenderParameters;
} // namespace Data
} // namespace Engine

namespace Gui {

/**
 * \brief Simple Widget for Material/RenderParameter editing
 * A material editor will show information about the selected material and expose a control panel
 * containing all of its editable parameters to allow the user to edit them.
 */
class RA_GUI_API MaterialParameterEditor : public QWidget
{
    Q_OBJECT
  public:
    /** Constructors and destructor
     */
    /** \{ */
    explicit MaterialParameterEditor( QWidget* parent = nullptr );
    MaterialParameterEditor( const MaterialParameterEditor& )            = delete;
    MaterialParameterEditor& operator=( const MaterialParameterEditor& ) = delete;
    MaterialParameterEditor( MaterialParameterEditor&& )                 = delete;
    MaterialParameterEditor&& operator=( MaterialParameterEditor&& )     = delete;
    ~MaterialParameterEditor() override                                  = default;
    /** \} */

    /**
     * \brief Update the different UI element with the given renderParameter, using the given
     * constraints.
     * \param params the parameter set to edit
     * \param constraints the parameter
     * constraints descriptor
     * \param name (optional) the name to display in top of the editor
     */
    void setupFromParameters( Engine::Data::RenderParameters& params,
                              const nlohmann::json& constraints,
                              const std::string& name = "" );

    /**
     * Update the different UI element with the given material.
     * This helper function calls setupFromParameters with the given material parameter set and
     * constraints but also displays the properties of the material and populate the widget
     * with widgets to edit its editable parameters.
     * \param material The pointer to the material to edit
     */
    void setupFromMaterial( std::shared_ptr<Ra::Engine::Data::Material> material );

    /**
     * Wether to show parameters without associated metadata
     *
     * \param enable
     */
    void showUnspecified( bool enable );

  signals:
    /**
     * Signal emitted whenever a parameter is modified
     */
    void materialParametersModified( const std::string& name );

  private:
    /// The label containing the instance name of the material
    QLabel* m_matInstanceNameLabel;
    /// The label containing the name of the material
    QLabel* m_matNameLabel;
    /// The text box containing the properties of the material
    QPlainTextEdit* m_matProperties;
    /// The layout containing the control panel
    QVBoxLayout* m_matParamsLayout;
    /// The control panel re-generated whenever setupFromMaterial is called
    VariableSetEditor* m_parametersControlPanel;
    /// wether to show the unspecified materials
    bool m_showUnspecified = false;

    // matInfoGroup
    QGroupBox* m_matInfoGroup;
};

} // namespace Gui
} // namespace Ra
