#ifndef RADIUMENGINE_TRANSLATE_GIZMO_HPP_
#define RADIUMENGINE_TRANSLATE_GIZMO_HPP_

#include <GuiBase/Viewer/Gizmo/Gizmo.hpp>

namespace Ra {
namespace Gui {

/// A TranslateGizmo manipulates the translational part of a transformation.
class TranslateGizmo : public Gizmo {
  public:
    // Draw a frame with : 3 RGB arrows and 3 RGB squares
    TranslateGizmo( Engine::Component* c, const Core::Transform& worldTo, const Core::Transform& t,
                    Mode mode );

    void updateTransform( Gizmo::Mode mode, const Core::Transform& worldTo,
                          const Core::Transform& t ) override;
    void selectConstraint( int drawableIndex ) override;
    void setInitialState( const Engine::Camera& cam, const Core::Vector2& initialXY ) override;
    Core::Transform mouseMove( const Engine::Camera& cam, const Core::Vector2& nextXY,
                               bool stepped = false ) override;

  private:
    Ra::Core::Vector3 m_startPoint;   ///< The picked 3D point on the gizmo.
    Ra::Core::Vector3 m_initialTrans; ///< The translation matrix at edition start.
    Core::Vector2 m_initialPix;       ///< The pixel position when edition starts.
    int m_selectedAxis;               ///< The axis to translate on.
    int m_selectedPlane;              ///< The plane to translate on.
    bool m_start;                     ///< Did the edition start.
};

} // namespace Gui
} // namespace Ra

#endif // RADIUMENGINE_TRANSLATE_GIZMO_HPP_
