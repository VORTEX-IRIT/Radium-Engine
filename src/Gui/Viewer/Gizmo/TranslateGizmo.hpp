#pragma once

#include <Core/Asset/Camera.hpp>
#include <Engine/Scene/CameraComponent.hpp>
#include <Gui/Viewer/Gizmo/Gizmo.hpp>

namespace Ra {
namespace Gui {

/// A TranslateGizmo manipulates the translational part of a transformation.
class TranslateGizmo : public Gizmo
{
  public:
    // Draw a frame with : 3 RGB arrows and 3 RGB squares
    TranslateGizmo( Engine::Scene::Component* c,
                    const Core::Transform& worldTo,
                    const Core::Transform& t,
                    Mode mode );

    void updateTransform( Gizmo::Mode mode,
                          const Core::Transform& worldTo,
                          const Core::Transform& t ) override;
    void selectConstraint( int drawableIndex ) override;
    bool isSelected() override { return m_selectedAxis != -1 || m_selectedPlane != -1; }
    void setInitialState( const Core::Asset::Camera& cam, const Core::Vector2& initialXY ) override;
    Core::Transform mouseMove( const Core::Asset::Camera& cam,
                               const Core::Vector2& nextXY,
                               bool stepped,
                               bool whole ) override;

  private:
    Ra::Core::Vector3 m_startPoint;   ///< The picked 3D point on the gizmo.
    Ra::Core::Vector3 m_initialTrans; ///< The translation matrix at editing start.
    Core::Vector2 m_initialPix;       ///< The pixel position when editing starts.
    int m_selectedAxis;               ///< The axis to translate on.
    int m_selectedPlane;              ///< The plane to translate on.
    bool m_start;                     ///< Did the editing start.
};

} // namespace Gui
} // namespace Ra
