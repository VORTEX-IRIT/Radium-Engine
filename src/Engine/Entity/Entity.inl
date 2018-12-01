#include <Engine/Entity/Entity.hpp>

namespace Ra {
namespace Engine {

inline const std::string& Entity::getName() const {
    return m_name;
}

inline void Entity::rename( const std::string& name ) {
    m_name = name;
}

inline void Entity::setTransform( const Core::Transform& transform ) {
    m_transformChanged = true;
    m_doubleBufferedTransform = transform;
}

inline void Entity::setTransform( const Core::Matrix4& transform ) {
    setTransform( Core::Transform( transform ) );
}

inline const Core::Transform & Entity::getTransform() const {
    // FIXME : why a mutex on read ? there is no lock on write on this!
    //std::lock_guard<std::mutex> lock( m_transformMutex );
    return m_transform;
}

inline const Core::Matrix4 & Entity::getTransformAsMatrix() const {
    // FIXME : why a mutex on read ? there is no lock on write on this!
    //std::lock_guard<std::mutex> lock( m_transformMutex );
    return m_transform.matrix();
}

inline uint Entity::getNumComponents() const {
    return uint( m_components.size() );
}
} // namespace Engine

} // namespace Ra
