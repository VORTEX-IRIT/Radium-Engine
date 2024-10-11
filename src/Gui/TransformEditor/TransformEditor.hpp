#pragma once

#include <Core/Types.hpp>
#include <Engine/Scene/ItemEntry.hpp>
#include <Gui/RaGui.hpp>
namespace Ra {
namespace Gui {
class RA_GUI_API TransformEditor
{
  public:
    TransformEditor() : m_transform( Core::Transform::Identity() ) {}
    virtual ~TransformEditor();

    /// Change the current editable object,
    virtual void setEditable( const Engine::Scene::ItemEntry& entry );

    /// Retrieve the transform from the editable and update the editor.
    virtual void updateValues() = 0;

  protected:
    // Helper to get the transform property from the editable.
    void getTransform();

    /// Helper to set the transform to the editable.
    void setTransform( const Ra::Core::Transform& tr );

    bool canEdit() const;

    Core::Transform getWorldTransform() const;

  protected:
    Core::Transform m_transform;            //! The transform being edited.
    Engine::Scene::ItemEntry m_currentEdit; //! The current item being edited.
};
} // namespace Gui
} // namespace Ra
