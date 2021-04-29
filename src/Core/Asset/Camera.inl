#pragma once

#include <Core/Asset/Camera.hpp>

namespace Ra {
namespace Core {
namespace Asset {

inline Core::Transform Camera::getFrame() const {
    return m_frame;
}

inline void Camera::setFrame( const Core::Transform& frame ) {
    m_frame = frame;
    notify();
}

inline Core::Vector3 Camera::getPosition() const {
    return ( m_frame.translation() );
}

inline void Camera::setPosition( const Core::Vector3& position ) {
    Core::Transform T = Core::Transform::Identity();
    T.translation()   = position - m_frame.translation();
    applyTransform( T );
}

inline Core::Vector3 Camera::getDirection() const {
    return ( -m_frame.affine().block<3, 1>( 0, 2 ) ).normalized();
}

inline Core::Vector3 Camera::getUpVector() const {
    return ( m_frame.affine().block<3, 1>( 0, 1 ) );
}

inline void Camera::setUpVector( const Core::Vector3& upVector ) {
    Core::Transform T = Core::Transform::Identity();
    T.rotate( Core::Quaternion::FromTwoVectors( getUpVector(), upVector ) );
    applyTransform( T );
}

inline Core::Vector3 Camera::getRightVector() const {
    return ( m_frame.affine().block<3, 1>( 0, 0 ) );
}

inline Scalar Camera::getFOV() const {
    return m_fov;
}

inline void Camera::setFOV( Scalar fov ) {
    m_fov = fov;
    updateProjMatrix();
}

inline Scalar Camera::getZNear() const {
    return m_zNear;
}

inline void Camera::setZNear( Scalar zNear ) {
    m_zNear = zNear;
    updateProjMatrix();
}

inline Scalar Camera::getZFar() const {
    return m_zFar;
}

inline void Camera::setZFar( Scalar zFar ) {
    m_zFar = zFar;
    updateProjMatrix();
}

inline Scalar Camera::getZoomFactor() const {
    return m_zoomFactor;
}

inline void Camera::setZoomFactor( const Scalar& zoomFactor ) {
    m_zoomFactor = zoomFactor;
    updateProjMatrix();
}

inline Scalar Camera::getWidth() const {
    return m_width;
}

inline Scalar Camera::getHeight() const {
    return m_height;
}

inline Scalar Camera::getAspect() const {
    return m_aspect;
}

inline Camera::ProjType Camera::getType() const {
    return m_projType;
}

inline void Camera::setType( const ProjType& projectionType ) {
    m_projType = projectionType;
}

inline Core::Matrix4 Camera::getViewMatrix() const {
    return getFrame().inverse().matrix();
}

inline Core::Matrix4 Camera::getProjMatrix() const {
    return m_projMatrix;
}

inline Scalar Camera::getMinZNear() const {
    return m_minZNear;
}

inline Scalar Camera::getMinZRange() const {
    return m_minZRange;
}

inline void Camera::setProjMatrix( Core::Matrix4 projMatrix ) {
    m_projMatrix = projMatrix;
}

inline Core::Vector2 Camera::project( const Core::Vector3& p ) const {
    Core::Vector4 point = Core::Vector4::Ones();
    point.head<3>()     = p;
    auto vpPoint        = getProjMatrix() * getViewMatrix() * point;

    return Core::Vector2( getWidth() * Scalar( 0.5 ) * ( vpPoint.x() + Scalar( 1 ) ),
                          getHeight() * Scalar( 0.5 ) * ( vpPoint.y() + Scalar( -1 ) ) );
}

inline Core::Vector3 Camera::unProject( const Core::Vector2& pix ) const {
    const Scalar localX = ( Scalar( 2 ) * pix.x() ) / getWidth() - Scalar( 1 );
    // Y is "inverted" (goes downwards)
    const Scalar localY = -( Scalar( 2 ) * pix.y() ) / getHeight() + Scalar( 1 );

    // Multiply the point in screen space by the inverted projection matrix
    // and then by the inverted view matrix ( = m_frame) to get it in world space.
    // NB : localPoint needs to be a vec4 to be multiplied by the proj matrix.
    const Core::Vector4 localPoint( localX, localY, -getZNear(), Scalar( 1 ) );
    const Core::Vector4 unproj = getProjMatrix().inverse() * localPoint;
    return getFrame() * unproj.head<3>();
}
} // namespace Asset
} // namespace Core
} // namespace Ra
