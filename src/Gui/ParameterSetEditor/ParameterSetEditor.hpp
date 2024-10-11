#pragma once
#include <Gui/RaGui.hpp>

#include <QGroupBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <nlohmann/json.hpp>

#include <Gui/Widgets/ControlPanel.hpp>

namespace Ra {

namespace Core {
class VariableSet;
}
namespace Engine {
namespace Data {
class Material;
class RenderParameters;
} // namespace Data
} // namespace Engine

namespace Gui {

/**
 * \brief Simple Widget for RenderParameter editing
 * The editor will expose a control panel
 * containing all of the editable parameters from a RenderParameter set.
 */
class RA_GUI_API VariableSetEditor : public Widgets::ControlPanel
{
    Q_OBJECT
  public:
    explicit VariableSetEditor( const std::string& name, QWidget* parent = nullptr );
    VariableSetEditor( const VariableSetEditor& )            = delete;
    VariableSetEditor& operator=( const VariableSetEditor& ) = delete;
    VariableSetEditor( VariableSetEditor&& )                 = delete;
    VariableSetEditor&& operator=( VariableSetEditor&& )     = delete;
    ~VariableSetEditor()                                     = default;

    /** \brief Update the different UI element with the given renderParameter, using the given
     * constraints.
     *
     * \param params the VariableSet to edit
     * \param constraints the parameter constraints descriptor
     */
    void setupUi( Core::VariableSet& params, const nlohmann::json& constraints );

    /** \brief Wether to show parameters without associated metadata
     *
     * \param enable
     */
    void setShowUnspecified( bool enable ) { m_showUnspecified = enable; }
    bool showUnspecified() { return m_showUnspecified; }

    /** \brief Add a combobox allowing to chose the value of an enumerator.
     *
     * \note Only un-scoped enum (i.e. implicitly convertible from and to integral type), with
     * enumerators without initializer.
     * \tparam T
     * \param name
     * \param initial
     * \param params
     * \param paramMetadata
     */
    template <typename T>
    void addEnumWidget( const std::string& name,
                        T& initial,
                        Core::VariableSet& params,
                        const nlohmann::json& paramMetadata );
    /**
     * \brief
     * \tparam T
     * \param name
     * \param initial
     * \param params
     * \param metadata
     */
    template <typename T>
    void addNumberWidget( const std::string& name,
                          T& initial,
                          Core::VariableSet& params,
                          const nlohmann::json& metadata );

    /**
     * \brief
     * \tparam T
     * \param key
     * \param initial
     * \param params
     * \param metadata
     */
    template <typename T>
    void addVectorWidget( const std::string& key,
                          std::vector<T>& initial,
                          Core::VariableSet& params,
                          const nlohmann::json& metadata );

    /**
     * \brief
     * \tparam T
     * \param key
     * \param initial
     * \param params
     * \param metadata
     */
    template <typename T>
    void addMatrixWidget( const std::string& key,
                          T& initial,
                          Core::VariableSet& params,
                          const nlohmann::json& metadata );

  signals:
    /**
     * Signal emitted whenever a parameter is modified
     */
    void parameterModified( const std::string& name );

  private:
    /// wether to show the unspecified materials
    bool m_showUnspecified = false;
};

} // namespace Gui
} // namespace Ra
