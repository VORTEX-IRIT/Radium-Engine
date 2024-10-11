#pragma once
#include <Gui/RaGui.hpp>

#include <QDoubleValidator>
#include <QWidget>

#include <Core/Types.hpp>
#include <stack>

namespace Ra {
namespace Gui {
namespace Widgets {
/**
 * A simple widget to edit matrices
 */
class RA_GUI_API MatrixEditor : public QWidget
{
    Q_OBJECT

  public:
    /** Constructors
     */
    /** \{ */
    explicit MatrixEditor( const Ra::Core::MatrixN& matrix, int dec, QWidget* parent = nullptr );
    MatrixEditor( const MatrixEditor& )            = delete;
    MatrixEditor& operator=( const MatrixEditor& ) = delete;
    MatrixEditor( MatrixEditor&& )                 = delete;
    MatrixEditor&& operator=( MatrixEditor&& )     = delete;
    /**\}*/

    /// Get the edited matrix
    const Ra::Core::MatrixN& matrix() const;

    /**
     * \return the number of decimal of the input
     */
    int decimals() const;

    /**
     * Set the precision of the input, in decimals
     * \param prec the precision in decimals
     */
    void setDecimals( int prec );

  signals:
    void valueChanged( const Ra::Core::MatrixN& );

  private:
    Ra::Core::MatrixN m_matrix;
};
} // namespace Widgets
} // namespace Gui
} // namespace Ra
