#pragma once
#include <Gui/RaGui.hpp>
#include <Gui/Widgets/QtTypeWrapper.hpp>

#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>
#include <string>
#include <vector>

namespace Ra {
namespace Gui {
namespace Widgets {

class RA_GUI_API VectorEditorSignals : public QWidget
{
    Q_OBJECT
  public:
    explicit VectorEditorSignals( QWidget* parent = nullptr ) : QWidget( parent ) {}
  signals:
    void valueChanged( const std::vector<int>& );
    void valueChanged( const std::vector<unsigned int>& );
    void valueChanged( const std::vector<float>& );
    void valueChanged( const std::vector<double>& );
};
/**
 * A Widget to edit vectors
 */
template <typename T = Scalar>
class VectorEditor : public VectorEditorSignals
{
  public:
    using WidgetType = typename QtSpinBox::getType<T>::Type;
    using SignalType = typename QtSpinBox::getType<T>::SignalType;
    /** Constructors
     */
    /** \{ */
    explicit VectorEditor( const std::vector<T>& vector, QWidget* parent = nullptr );
    VectorEditor( const VectorEditor& )            = delete;
    VectorEditor& operator=( const VectorEditor& ) = delete;
    VectorEditor( VectorEditor&& )                 = delete;
    VectorEditor&& operator=( VectorEditor&& )     = delete;
    /**\}*/

    /**
     * \return the value of the vector
     */
    const std::vector<T>& vector() const;

  private:
    std::vector<T> m_vector;
};

template <typename T>
VectorEditor<T>::VectorEditor( const std::vector<T>& vector, QWidget* parent ) :
    VectorEditorSignals( parent ), m_vector( vector ) {

    QVBoxLayout* layout = new QVBoxLayout;

    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 0 );
    for ( auto& elem : m_vector ) {
        auto spinbox = new WidgetType( this );
        spinbox->setMinimum( std::numeric_limits<T>::lowest() );
        spinbox->setMaximum( std::numeric_limits<T>::max() );
        spinbox->setValue( elem );
        if constexpr ( std::is_floating_point_v<T> ) { spinbox->setDecimals( 3 ); }
        layout->addWidget( spinbox );
        auto updateVectorOnChange = [this, &elem]( SignalType value ) {
            elem = T( value );
            emit valueChanged( m_vector );
        };
        connect(
            spinbox, QOverload<SignalType>::of( &WidgetType::valueChanged ), updateVectorOnChange );
    }
    setLayout( layout );
}

template <typename T>
const std::vector<T>& VectorEditor<T>::vector() const {
    return m_vector;
}
} // namespace Widgets
} // namespace Gui
} // namespace Ra
