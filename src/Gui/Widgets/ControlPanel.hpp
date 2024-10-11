#pragma once
#include <Core/Types.hpp>
#include <Core/Utils/Color.hpp>
#include <Gui/RaGui.hpp>
#include <Gui/Widgets/ConstrainedNumericSpinBox.hpp>
#include <Gui/Widgets/VectorEditor.hpp>

#include <QBoxLayout>
#include <QFrame>
#include <QVBoxLayout>

#include <functional>
#include <stack>
#include <string>

#include <QLabel>

namespace Ra {
namespace Gui {
namespace Widgets {
/**
 * Simple panel for Qt gui.
 * A control panel will expose all configurable options the object its decorate to allow the user to
 * interact with it.
 */
class RA_GUI_API ControlPanel : public QFrame
{
    Q_OBJECT
  public:
    explicit ControlPanel( const std::string& name, bool hline = true, QWidget* parent = nullptr );
    ControlPanel( const ControlPanel& )            = delete;
    ControlPanel& operator=( const ControlPanel& ) = delete;
    ControlPanel( ControlPanel&& )                 = delete;
    ControlPanel& operator=( ControlPanel&& )      = delete;
    ~ControlPanel() override                       = default;

    /**Methods to populate the panel with dedicated ui elements.
     */
    /** \{ */

    /** Add an option to the panel
     *  An option is an on/off checkbox to activate a state of the renderer.
     * \param name The name of the option
     * \param callback The function to call when the state changed
     * \param set The initial value of the option
     */
    void addOption( const std::string& name,
                    std::function<void( bool )> callback,
                    bool set                   = false,
                    const std::string& tooltip = "" );

    /** Add a label.
     * Shows text.
     * \param text The text to show
     */
    void addLabel( const std::string& text );

    /** Add a button.
     * This button, when pressed, will call the associated callback.
     * \param name The name of the button
     * \param callback The function to call when the state changed
     * \param tooltip The tooltip text
     */
    void addButton( const std::string& name,
                    std::function<void()> callback,
                    const std::string& tooltip = "" );

    /** Add a color input button.
     * This button, when pressed, will open a color chooser and call the associated callback.
     * The button is also painted with the chosen color.
     * \param name The name of the color to choose
     * \param callback The function to call when the state changed. The color give to the callback
     * is in linearRGB color space.
     * \param color The initial value of the color, in linearRGB color space.
     * \param withAlpha allow to enter alpha channel value
     * \param tooltip The tooltip text
     */
    void addColorInput( const std::string& name,
                        const std::function<void( const Ra::Core::Utils::Color& clr )>& callback,
                        Ra::Core::Utils::Color color = Ra::Core::Utils::Color::Black(),
                        bool withAlpha               = true,
                        const std::string& tooltip   = "" );

    /** Add a scalar input spinbox.
     *  This input will return a scalar value within  given bounds.
     * \tparam T The type of value to edit (default Scalar)
     * \param name The name of the scalar value
     * \param callback The function to call when the state changed
     * \param initial The initial value of the scalar
     * \param min The min bound of the value
     * \param max The max bound of the value
     * \param tooltip The tooltip text
     * \param dec The display precision (decimals) of the value
     */
    template <typename T = Scalar>
    void addNumberInput( const std::string& name,
                         std::function<void( T )> callback,
                         T initial,
                         T min                      = std::numeric_limits<T>::lowest(),
                         T max                      = std::numeric_limits<T>::max(),
                         const std::string& tooltip = "",
                         int dec                    = 3 );

    /** Add a checking scalar (real number) input spinbox.
     *  This input will return a scalar value that meet the conditions of a given predicate.
     * \tparam T The type of value to edit (default Scalar)
     * \param name The name of the scalar value
     * \param callback The function to call when the state changed
     * \param initial The initial value of the scalar
     * \param predicate defines a set of conditions that the scalar should meet
     * \param tooltip The tooltip text
     * \param dec The display precision (decimals) of the value
     */
    template <typename T = Scalar>
    void addConstrainedNumberInput( const std::string& name,
                                    std::function<void( T )> callback,
                                    Scalar initial,
                                    std::function<bool( T )> predicate,
                                    const std::string& tooltip = "",
                                    int dec                    = 3 );

    /** Add an horizontal slider input.
     *  This input will return a scalar value within  given bounds.
     * \param name The name of the scalar value
     * \param callback The function to call when the state changed
     * \param initial The initial value of the scalar
     * \param min The min bound of the value
     * \param max The max bound of the value
     * \param tooltip The tooltip text
     */
    void addSliderInput( const std::string& name,
                         std::function<void( int )> callback,
                         int initial                = 0,
                         int min                    = 0,
                         int max                    = 100,
                         const std::string& tooltip = "" );

    /** Add an horizontal power slider input.
     *  This input will return a scalar value within  given bounds.
     * \param name The name of the scalar value
     * \param callback The function to call when the state changed
     * \param initial The initial value of the color
     * \param min The min bound of the value
     * \param max The max bound of the value
     * \param tooltip The tooltip text
     */
    void addPowerSliderInput( const std::string& name,
                              std::function<void( double )> callback,
                              double initial             = 0,
                              double min                 = 0,
                              double max                 = 100,
                              const std::string& tooltip = "" );

    /** Add a vector input.
     *  This input will return a vector.
     * \param name The name of the vector
     * \param callback The function to call when the state changed
     * \param initial The initial value of the vector
     * \param tooltip The tooltip text
     */
    template <typename T = Scalar>
    void addVectorInput( const std::string& name,
                         std::function<void( const std::vector<T>& )> callback,
                         const std::vector<T>& initial,
                         const std::string& tooltip = "" );

    /** Add a matrix input.
     *  This input will return a matrix.
     * \param name The name of the matrix
     * \param callback The function to call when the state changed
     * \param initial The initial value of the matrix
     * \param dec The display precision (decimals) of the value
     * \param tooltip The tooltip text
     */
    void addMatrixInput( const std::string& name,
                         std::function<void( const Ra::Core::MatrixN& )> callback,
                         const Ra::Core::MatrixN& initial,
                         int dec                    = 3,
                         const std::string& tooltip = "" );

    /** Add a file dialog to the ui.
     * Allow the user to select a file to load according to given filters
     * \param name The name of the file property to expose
     * \param callback The function to call when the state changed
     * \param rootDirectory The initial directory of the file dialog
     * \param filters The filters to apply to filenames
     * \param tooltip The tooltip text
     */
    void addFileInput( const std::string& name,
                       std::function<void( std::string )> callback,
                       const std::string& rootDirectory,
                       const std::string& filters,
                       const std::string& tooltip = "" );

    /** Add a save file dialog to the ui.
     * Allow the user to select a file to load according to given filters
     * \param name The name of the file property to expose
     * \param callback The function to call when the state changed
     * \param rootDirectory The initial directory of the file dialog
     * \param filters The filters to apply to filenames
     * \param tooltip The tooltip text
     */
    void addFileOutput( const std::string& name,
                        std::function<void( std::string )> callback,
                        const std::string& rootDirectory,
                        const std::string& filters,
                        const std::string& tooltip = "" );

    /** Add a combobox to the panel
     * Allow the user to select values from a combobox
     * \param name The name of the option
     * \param callback The function to call when the index changed
     * \param initial The initial index of the combobox
     * \param items The menu items of the combobox
     * \param tooltip The tooltip text
     */
    void addComboBox( const std::string& name,
                      std::function<void( int )> callback,
                      int initial,
                      const std::vector<std::string>& items,
                      const std::string& tooltip = "" );
    void addComboBox( const std::string& name,
                      std::function<void( const QString& )> callback,
                      const std::string& initial,
                      const std::vector<std::string>& items,
                      const std::string& tooltip = "" );

    /** Add an arbitrary widget to the ui.
     * \param newWidget The widget to add to the ui.
     */
    void addWidget( QWidget* newWidget );

    /**\}*/

    void addSeparator();
    void addStretch( int stretch );

    void newLayout() {
        m_contentLayout = new QGridLayout();
        m_contentLayout->setObjectName( "new layout" );
        m_mainLayout->addLayout( m_contentLayout );
    }

  private:
    /// The layout to organise the ui elements
    QVBoxLayout* m_mainLayout;
    QGridLayout* m_contentLayout;
};

template <typename T>
void ControlPanel::addConstrainedNumberInput( const std::string& name,
                                              std::function<void( T )> callback,
                                              Scalar initial,
                                              std::function<bool( T )> predicate,
                                              const std::string& tooltip,
                                              int dec ) {
    auto inputLabel = new QLabel( tr( name.c_str() ) );

    auto inputField = new ConstrainedNumericSpinBox<T>();
    inputField->setValue( initial );
    inputField->setMinimum( std::numeric_limits<T>::lowest() );
    inputField->setMaximum( std::numeric_limits<T>::max() );
    connect( inputField,
             qOverload<typename QtSpinBox::getType<T>::SignalType>(
                 &ConstrainedNumericSpinBox<T>::valueChanged ),
             std::move( callback ) );
    inputField->setPredicate( predicate );

    if constexpr ( std::is_floating_point_v<T> ) { inputField->setDecimals( dec ); }

    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

template <typename T>
void ControlPanel::addNumberInput( const std::string& name,
                                   std::function<void( T )> callback,
                                   T initial,
                                   T min,
                                   T max,
                                   const std::string& tooltip,
                                   int dec ) {

    auto inputLabel = new QLabel( tr( name.c_str() ) );

    using WidgetType = typename QtSpinBox::getType<T>::Type;
    auto inputField  = new WidgetType();
    auto inputLayout = new QHBoxLayout();
    inputLayout->addStretch();
    inputLayout->addWidget( inputField );
    // to prevent overflow
    inputField->setRange(
        min,
        std::min( T( std::numeric_limits<typename QtSpinBox::getType<T>::SignalType>::max() ),
                  max ) );
    inputField->setValue( initial );
    connect( inputField,
             qOverload<typename QtSpinBox::getType<T>::SignalType>( &WidgetType::valueChanged ),
             std::move( callback ) );
    if constexpr ( std::is_floating_point_v<T> ) { inputField->setDecimals( dec ); }

    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addLayout( inputLayout, index, 1 );
}

template <typename T>
void ControlPanel::addVectorInput( const std::string& name,
                                   std::function<void( const std::vector<T>& )> callback,
                                   const std::vector<T>& initial,
                                   const std::string& tooltip ) {

    auto inputLabel = new QLabel( tr( name.c_str() ) );
    auto inputField = new VectorEditor<T>( initial );

    if ( !tooltip.empty() ) {
        auto tooltipString =
            QString( "<qt>%1</qt>" ).arg( QString( tooltip.c_str() ).toHtmlEscaped() );
        inputLabel->setToolTip( tooltipString );
        inputField->setToolTip( tooltipString );
    }
    connect( inputField,
             QOverload<const std::vector<T>&>::of( &VectorEditorSignals::valueChanged ),
             std::move( callback ) );

    auto index = m_contentLayout->rowCount();
    m_contentLayout->addWidget( inputLabel, index, 0 );
    m_contentLayout->addWidget( inputField, index, 1 );
}

} // namespace Widgets
} // namespace Gui
} // namespace Ra
