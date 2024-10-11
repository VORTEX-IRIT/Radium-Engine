#pragma once

#include <Engine/RaEngine.hpp>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <Core/Utils/IndexMap.hpp>
#include <Core/Utils/Singleton.hpp>

namespace Ra {
namespace Engine {
namespace Scene {

class Entity;

class RA_ENGINE_API EntityManager
{
  public:
    /// CONSTRUCTOR
    EntityManager();

    // Entity manager is non-copyable
    EntityManager( const EntityManager& )            = delete;
    EntityManager& operator=( const EntityManager& ) = delete;

    /// DESTRUCTOR
    virtual ~EntityManager();

    /**
     * \brief Create (if does not exist) an entity given its name
     * \param name The name of the entity to get or create. If
     * empty, the entity will be created with a generic name "Entity_xx"
     * \return The entity.
     */
    Entity* createEntity( const std::string& name = "" );

    /**
     * \brief Check wether an entity with a given name exists or not.
     * \param name The name of the entity to find
     * \return true if the entity exists, false otherwise
     */
    bool entityExists( const std::string& name ) const;

    /**
     * \brief Remove an entity given its index. Also deletes the pointer.
     * \param idx Index if the entity to remove.
     */
    void removeEntity( Core::Utils::Index idx );

    /**
     * \brief Remove a given entity. Also deletes the pointer.
     * \param entity The entity to remove.
     */
    void removeEntity( Entity* entity );

    /**
     * \brief Get an entity given its index.
     * \param idx Index of the component to retrieve.
     * \return The entity if found in the map, nullptr otherwise.
     */
    Entity* getEntity( Core::Utils::Index idx ) const;

    /**
     * \brief Get all entities from the manager.
     * This might be usefull to be able to display and navigate through them
     * in a GUI for example.
     * \return A list containing all entities from the manager.
     */
    std::vector<Entity*> getEntities() const;

    void swapBuffers();

    /**
     * \brief Get an entity given its name.
     * \param name Name of the entity to retrieve.
     * \return The entity if found in the map, nullptr otherwise.
     */
    Entity* getEntity( const std::string& name ) const;

    /**
     * \brief Removes all entities.
     */
    void deleteEntities();

  private:
    Core::Utils::IndexMap<std::unique_ptr<Entity>> m_entities;
    std::map<std::string, Core::Utils::Index> m_entitiesName;
};

} // namespace Scene
} // namespace Engine
} // namespace Ra
