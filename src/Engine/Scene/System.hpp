#pragma once

#include <Engine/RaEngine.hpp>

#include <memory>
#include <vector>

namespace Ra {
namespace Core {
class TaskQueue;
namespace Asset {
class FileData;
}
} // namespace Core

namespace Engine {
struct FrameInfo;

namespace Scene {

class Component;
class Entity;

/** Systems are responsible of updating a specific subset of the components of each entity.
 * They can provide factory methods to create components, but their main role is to keep a
 * list of "active" components associated to an entity.
 * At each frame, each system loaded into the engine will be queried for tasks.
 * The goal of the tasks is to update the active components during the frame.
 */
class RA_ENGINE_API System
{
    friend class Component;

  public:
    System()          = default;
    virtual ~System() = default;

    /**
     * Factory method for component creation from file data.
     * From a given file and the corresponding entity, the system will create the
     * corresponding components, add them to the entity, and register the component.
     * \note : Issue #325 - As this method register components and might also manage each component
     * outside the m_components vectors (e.g in a buffer on the GPU) the methods, the
     * registerComponent and unregister*Component must be virtual method that could be overriden.
     */
    virtual void handleAssetLoading( Entity* entity, const Core::Asset::FileData* data ) {
        CORE_UNUSED( entity );
        CORE_UNUSED( data );
    }

    /**
     * \brief Pure virtual method to be overridden by any system.
     * Must register in taskQueue the operations that must be done ate each frame
     *
     * \param taskQueue The queue to fill
     * \param frameInfo Information about the current frame (\see FrameInfo)
     */
    virtual void generateTasks( Core::TaskQueue* taskQueue,
                                const Engine::FrameInfo& frameInfo ) = 0;

    /** Returns the components stored for the given entity.
     *
     * \param entity
     * \return the vector of the entity's components
     */
    std::vector<Component*> getEntityComponents( const Entity* entity );

    /** Register an already configured component to the system.
     * \param entity The entity owning the component
     * \param component The component to add to the system
     * \note the component is added to the system without any compatibility check.
     */
    void addComponent( Entity* entity, Component* component ) {
        registerComponent( entity, component );
    }

  protected:
    /**
     * Registers a component belonging to an entity, making it active within the system.
     * \note If a system overrides this function, it must call the inherited method.
     * \param entity
     * \param component
     */
    virtual void registerComponent( const Entity* entity, Component* component );

    /**
     * Unregisters a component. The system will not update it.
     * \note If a system overrides this function, it must call the inherited method.
     * \param entity
     * \param component
     */
    virtual void unregisterComponent( const Entity* entity, Component* component );

    /**
     * Removes all components belonging to a given entity.
     * \note If a system overrides this function, it must call the inherited method.
     * \param entity
     */
    virtual void unregisterAllComponents( const Entity* entity );

  protected:
    /// List of active components.
    std::vector<std::pair<const Entity*, Component*>> m_components;
};

} // namespace Scene
} // namespace Engine
} // namespace Ra
