#pragma once

#include <memory>

#include <Gui/TransformEditor/TransformEditor.hpp>
#include <Gui/Utils/KeyMappingManager.hpp>
#include <Gui/Viewer/Gizmo/Gizmo.hpp>
#include <QMouseEvent>
#include <QObject>

namespace Ra {
namespace Engine {
namespace Scene {
struct ItemEntry;
} // namespace Scene
} // namespace Engine
} // namespace Ra
namespace Ra {
namespace Gui {
/// This class interfaces the gizmos with the ui commands.
/// It allows to change the gizmo type when editing an editable transform object
/// Note :  currently the scale gizmo is not implemented so it will just return a null pointer
class RA_GUI_API GizmoManager : public QObject,
                                public Gui::TransformEditor,
                                public KeyMappingManageable<GizmoManager>
{
    Q_OBJECT
    friend class KeyMappingManageable<GizmoManager>;

  public:
    enum GizmoType { NONE, TRANSLATION, ROTATION, SCALE };

    explicit GizmoManager( QObject* parent = nullptr );
    ~GizmoManager() = default;

  public:
    /// Receive mouse events and transmit them to the gizmos.
    virtual bool handleMousePressEvent( QMouseEvent* event,
                                        const Qt::MouseButtons& buttons,
                                        const Qt::KeyboardModifiers& modifiers,
                                        int key,
                                        const Core::Asset::Camera& cam );
    virtual bool handleMouseReleaseEvent( QMouseEvent* event );
    virtual bool handleMouseMoveEvent( QMouseEvent* event,
                                       const Qt::MouseButtons& buttons,
                                       const Qt::KeyboardModifiers& modifiers,
                                       int key,
                                       const Core::Asset::Camera& cam );

  public slots:
    /// Set the object being currently edited
    void setEditable( const Engine::Scene::ItemEntry& ent ) override;

    /// Destroy all gizmos
    void cleanup();

    /// Callback when a drawable is picked.
    void handlePickingResult( int drawableId );

    /// Change mode from local axis to global
    void setLocal( bool useLocal );

    /// Change gizmo type (rotation or translation)
    void changeGizmoType( GizmoType type );

    /// Retrieve the transform from the editable and update the gizmos.
    void updateValues() override;

  private:
    bool isValidAction( const Gui::KeyMappingManager::KeyMappingAction& action ) const;

    // Helper method to change the current gizmo
    void updateGizmo();

    // Returs the current gizmo
    Gizmo* currentGizmo();

  private:
    std::array<std::unique_ptr<Gizmo>, 3> m_gizmos; //! Owning pointers to the gizmos
    GizmoType m_currentGizmoType;                   //! Type of the gizmo
    Gizmo::Mode m_mode;                             //! Local/global axis mode.

    static void configureKeyMapping_impl();

#define KeyMappingGizmo                    \
    KMA_VALUE( GIZMOMANAGER_MANIPULATION ) \
    KMA_VALUE( GIZMOMANAGER_STEP )         \
    KMA_VALUE( GIZMOMANAGER_WHOLE )        \
    KMA_VALUE( GIZMOMANAGER_STEP_WHOLE )

#define KMA_VALUE( XX ) static KeyMappingManager::KeyMappingAction XX;
    KeyMappingGizmo
#undef KMA_VALUE
};
} // namespace Gui
} // namespace Ra
