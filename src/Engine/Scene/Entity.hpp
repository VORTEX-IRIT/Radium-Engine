#pragma once

#include <Engine/RaEngine.hpp>

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <Core/Types.hpp>
#include <Core/Utils/IndexedObject.hpp>
#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Engine {
namespace Scene {

class Component;
class System;

/// An entity is an scene element. It ties together components with a transform.
class RA_ENGINE_API Entity : public Core::Utils::IndexedObject
{
  public:
    explicit Entity( const std::string& name = "" );

    // Entities are not copyable.
    Entity( const Entity& entity )     = delete;
    Entity& operator=( const Entity& ) = delete;

    ~Entity() override;

    // Name
    inline const std::string& getName() const;
    inline void rename( const std::string& name );

    // Transform
    inline void setTransform( const Core::Transform& transform );
    inline void setTransform( const Core::Matrix4& transform );
    const Core::Transform& getTransform() const;
    const Core::Matrix4& getTransformAsMatrix() const;

    void swapTransformBuffers();

    /// get a ref to transformation observers to add/remove an observer
    inline Core::Utils::Observable<const Entity*>& transformationObservers() const;

    // Components
    /// Add a component to the given entity. Component ownership is transfered to the entity.
    void addComponent( Component* component );

    /// Deletes a component with a given name.
    void removeComponent( const std::string& name );

    /// Get component with a given name.
    Component* getComponent( const std::string& name );
    const Component* getComponent( const std::string& name ) const;

    const std::vector<std::unique_ptr<Component>>& getComponents() const;

    /// Get component belonging to a given system.
    // Component* getComponent( const System& system);

    inline uint getNumComponents() const;

    virtual Core::Aabb computeAabb();

    void invalidateAabb();

  private:
    Core::Transform m_transform;
    Core::Transform m_doubleBufferedTransform;
    mutable std::mutex m_transformMutex;

    std::vector<std::unique_ptr<Component>> m_components;

    std::string m_name {};
    bool m_transformChanged { false };

    bool m_isAabbValid { false };
    Core::Aabb m_aabb;

    /// Listeners on Entity Transformation changes
    mutable Core::Utils::Observable<const Entity*> m_transformationObservers;

    ///\todo add an index;
};

inline const std::string& Entity::getName() const {
    return m_name;
}

inline void Entity::rename( const std::string& name ) {
    m_name = name;
}

inline void Entity::setTransform( const Core::Transform& transform ) {
    m_transformChanged        = true;
    m_doubleBufferedTransform = transform;
}

inline void Entity::setTransform( const Core::Matrix4& transform ) {
    setTransform( Core::Transform( transform ) );
}

inline const Core::Transform& Entity::getTransform() const {
    // Radium-V2 : why a mutex on read ? there is no lock on write on this!
    std::lock_guard<std::mutex> lock( m_transformMutex );
    return m_transform;
}

inline const Core::Matrix4& Entity::getTransformAsMatrix() const {
    // Radium-V2 : why a mutex on read ? there is no lock on write on this!
    std::lock_guard<std::mutex> lock( m_transformMutex );
    return m_transform.matrix();
}

inline uint Entity::getNumComponents() const {
    return uint( m_components.size() );
}

inline Core::Utils::Observable<const Entity*>& Entity::transformationObservers() const {
    return m_transformationObservers;
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
