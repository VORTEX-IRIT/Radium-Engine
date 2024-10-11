#pragma once

#include <Core/Asset/Camera.hpp>
#include <Engine/Scene/CameraComponent.hpp>
#include <Gui/Viewer/Gizmo/Gizmo.hpp>

namespace Ra {
namespace Gui {

/// A RotateGizmo manipulates the rotational part of a transformation.
class RotateGizmo : public Gizmo
{
  public:
    // Draw a frame with : 3 RGB arrows
    RotateGizmo( Engine::Scene::Component* c,
                 const Core::Transform& worldTo,
                 const Core::Transform& t,
                 Mode mode );

    void updateTransform( Gizmo::Mode mode,
                          const Core::Transform& worldTo,
                          const Core::Transform& t ) override;
    void selectConstraint( int drawableIndex ) override;
    bool isSelected() override { return m_selectedAxis != -1; }

    void setInitialState( const Core::Asset::Camera& cam, const Core::Vector2& initialXY ) override;
    Core::Transform mouseMove( const Core::Asset::Camera& cam,
                               const Core::Vector2& nextXY,
                               bool stepped,
                               bool whole ) override;

  private:
    Core::Vector2 m_initialPix; ///< The pixel location when editing starts.
    int m_selectedAxis;         ///< The axis to rotate around.
    bool m_start;               ///< Has the manipulation started.
    bool m_stepped;             ///< Is the manipulation stepped.
    Scalar m_totalAngle;        ///< The total editing angle.
    Core::Matrix3 m_initialRot; ///< The rotation matrix at editing start.
};

} // namespace Gui
} // namespace Ra
