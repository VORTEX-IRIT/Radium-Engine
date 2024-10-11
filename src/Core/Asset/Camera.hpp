#pragma once

#include <Core/Math/Math.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Observable.hpp>
namespace Ra {
namespace Core {
namespace Asset {

/// \brief Camera class storing the Camera frame and the projection properties
/// The view direction is -z in camera space.
/// \note Both orthonormal and non-orthonormal frames are supported. However, in the current
/// implementation suspect a bug in fitZRange which does not behave as expected for non-orthogonal
/// frames..
class RA_CORE_API Camera
{
  public:
    /// Define the projection type.
    enum class ProjType { ORTHOGRAPHIC, PERSPECTIVE };

    Camera( Scalar height = 1_ra, Scalar width = 1_ra );

    ~Camera() = default;

    Camera& operator=( const Camera& rhs );

    /// Return the frame of the camera.
    /// Where Y is the up vector and -Z is the direction vector.
    inline Core::Transform getFrame() const;

    /// Set the frame of the camera to \b frame.
    inline void setFrame( const Core::Transform& frame );

    /// Return the position.
    inline Core::Vector3 getPosition() const;

    /// Set the position of the camera to \b position.
    inline void setPosition( const Core::Vector3& position );

    /// Return the direction the camera is looking at.
    inline Core::Vector3 getDirection() const;

    /// Set the direction of the camera to \b direction.
    /// The other vectors will be rotated accordingly.
    void setDirection( const Core::Vector3& direction );

    /// Return the up vector.
    inline Core::Vector3 getUpVector() const;

    /// Set the up vector of the camera to \b upVector.
    /// The other vectors will be rotated accordingly.
    inline void setUpVector( const Core::Vector3& upVector );

    inline Core::Vector3 getRightVector() const;

    //
    // Utility functions
    //

    // Note : in all screen pixels coordinates function, Y is taken to be in standard UI-libs style
    // coordinates, i.e. Y=0 is the top line and Y=height is the bottom line.

    /// Return a ray emanating from the camera, passing by the point given by
    /// screen coordinates x and y.
    Core::Ray getRayFromScreen( const Core::Vector2& pix ) const;

    /// Return the screen coordinates + depth of the given point p (in world coordinates).
    Core::Vector3 projectToScreen( const Core::Vector3& p ) const;

    /// Return NDC cordinate in the view NDC cube \f$ [-1,1]^3 \f$
    Core::Vector3 projectToNDC( const Core::Vector3& p ) const;

    /// Return the 3D point in world space corresponding to screen coordinates pix.
    /// pix.x() and pix.y() are in pixel coordinates \f$ \in [0, getWidth()] \times [0, getHeight()]
    /// \f$. The returned point lies on z near plane.
    Core::Vector3 unProjectFromScreen( const Core::Vector2& pix ) const;

    /// Return the 3D point in world space corresponding to screen pixels pix.x(), pix.y(), at depth
    /// pix.z()
    /// x and y are in pixel coordinates (from (0,0) to width,height, z is in 0, 1 (0 near, 1 far
    /// plane)
    Core::Vector3 unProjectFromScreen( const Core::Vector3& pix ) const;

    /// Return the 3D point in world space corresponding to NDC cube \f$ [-1,1]^3 \f$  point pix.
    Core::Vector3 unProjectFromNDC( const Core::Vector3& pix ) const;

    //
    // Getters and setters for projection matrix parameters.
    //

    /// Return the projection type.
    inline ProjType getType() const;

    /// Set the projection type to \b projectionType.
    inline void setType( const ProjType& projectionType );

    /// Return the zoom factor.
    inline Scalar getZoomFactor() const;

    /// Set the zoom factor to \b zoomFactor.
    inline void setZoomFactor( const Scalar& zoomFactor );

    /// Return the horizontal Field Of View.
    /// \note Meaningless for orthogonal projection.
    inline Scalar getFOV() const;

    /// Set the Field Of View to 'fov' in the x (horizontal) direction.
    /// If you have an vertical field of view, you can convert it to horizontal as
    /// \code{.cpp}
    /// Scalar fovx = 2_ra * std::atan( radiumCam->getAspect() * std::tan( cam.yfov / 2_ra ) );
    ///        if ( fovxDiv2 < 0_ra ) { fovxDiv2 = Ra::Core::Math::PiDiv2; }
    /// \endcode
    /// \note Meaningless for orthogonal projection.
    /// \warning Trigger a rebuild of the projection matrix.
    inline void setFOV( Scalar fov );

    inline Scalar getMinZNear() const;
    inline Scalar getMinZRange() const;

    /// Return the projection matrix.
    inline Core::Matrix4 getProjMatrix() const;
    inline Core::Matrix4 getViewMatrix() const;

    /// Update the projection matrix according to the current parameters.
    void updateProjMatrix();

    /// set projection to \b projMatrix.
    /// \see perspective(), ortho().
    inline void setProjMatrix( Core::Matrix4 projMatrix );

    /// Return the Z Near plane distance from the camera.
    inline Scalar getZNear() const;

    /// Set the Z Near plane distance to  \b zNear.
    inline void setZNear( Scalar zNear );

    /// Return the Z Far plane distance from the camera.
    inline Scalar getZFar() const;

    /// Set the Z Far plane distance to \b zFar.
    inline void setZFar( Scalar zFar );

    /// Return the width of the viewport.
    inline Scalar getWidth() const;

    /// Return the height of the viewport.
    inline Scalar getHeight() const;

    /// Return the aspect ratio of the viewport.
    inline Scalar getAspect() const;

    /// Set xmag and ymag for orthographic camera
    inline void setXYmag( Scalar xmag, Scalar ymag );

    /// Get xmag and ymag for orthographic camera
    inline std::pair<Scalar, Scalar> getXYmag() const;

    /// Change the viewport size. Also compute aspectRatio.
    void setViewport( Scalar width, Scalar height );

    void applyTransform( const Core::Transform& T );

    /// Fit the Z-range of the camera to the scene's aabb. This will maximize z-buffer precision
    void fitZRange( const Core::Aabb& aabb );

    const Scalar m_minZNear { 0.01_ra };
    const Scalar m_minZRange { 0.01_ra };

    /// Compute a project projection matrix as describe here
    /// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#cameras
    /// adapted to horizontal fov
    /// equivalent to scale = tan(m_fov*.5)*m_zNear
    /// frustum(-n*tan(f/2), n*tan(f/2), -n*tan(f/2)/aspect, n*tan(f/2)/aspect, n, f);
    /// \param a : aspect ratio as width/height, i.e. getAspect()
    /// \param y : fov in the horizontal direction, i.e. getFOV()
    /// \param n : z near, i.e. getZNear()
    /// \param f : z far, i.e. getZFar()
    static Core::Matrix4 perspective( Scalar a, Scalar y, Scalar n, Scalar f );

    /// \brief Build a projection matrix from the parameters of the view volume
    /// \see perspective()
    /// Implements the algorithm described here
    /// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
    /// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml adapted to our
    /// representation of camera
    /// \param l : left
    /// \param r : right
    /// \param b : bottom
    /// \param t : top
    /// \param n : z near
    /// \param f : z far
    static Core::Matrix4 frustum( Scalar l, Scalar r, Scalar b, Scalar t, Scalar n, Scalar f );

    /// \brief Build a projection matrix from the parameters of the view volume
    /// Implements the algorithms described here
    /// https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix
    /// https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glOrtho.xml adapted to our
    /// representation of camera
    /// \param l : left
    /// \param r : right
    /// \param b : bottom
    /// \param t : top
    /// \param n : z near
    /// \param f : z far
    static Core::Matrix4 ortho( Scalar l, Scalar r, Scalar b, Scalar t, Scalar n, Scalar f );

  private:
    /// Camera frame (inverse of the view matrix).
    /// This represent the transformation from view space to world space.
    Core::Transform m_frame { Core::Transform::Identity() };

    Core::Matrix4 m_projMatrix { Core::Matrix4::Identity() }; ///< Projection matrix

    Scalar m_width { 1_ra };  ///< Viewport width (in pixels)
    Scalar m_height { 1_ra }; ///< Viewport height (in pixels)
    Scalar m_aspect {
        1_ra }; ///< Aspect ratio, i.e. width/height. Precomputed for updateProjMatrix.

    ProjType m_projType { ProjType::PERSPECTIVE }; ///< Projection type
    Scalar m_zoomFactor { 1_ra };                  ///< Zoom factor (modifies the field of view)
    Scalar m_zNear { 0.1_ra };                     ///< Z Near plane distance
    Scalar m_zFar { 1000_ra };                     ///< Z Far plane distance

    /// \name Perspective projection parameters
    ///\{
    Scalar m_fov { Core::Math::PiDiv4 }; ///< Horizontal Field Of View
    ///\}

    /// \name Orthographic projection parameters
    ///\{
    Scalar m_xmag { 1_ra };
    Scalar m_ymag { 1_ra };
    ///\}
};

inline Core::Transform Camera::getFrame() const {
    return m_frame;
}

inline void Camera::setFrame( const Core::Transform& frame ) {
    m_frame = frame;
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
    return ( -m_frame.linear().block<3, 1>( 0, 2 ) ).normalized();
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
    // update m_xmag and m_ymag according to the perspective parameters (heuristic)
    m_xmag = 2_ra * std::tan( m_fov / 2_ra );
    m_ymag = m_xmag;
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

inline void Camera::setXYmag( Scalar xmag, Scalar ymag ) {
    m_xmag = xmag;
    m_ymag = ymag;
    // update m_fov according to  orthographic parameters (heuristic)
    m_fov = std::atan2( m_xmag * 2, 1_ra );
    updateProjMatrix();
}

inline std::pair<Scalar, Scalar> Camera::getXYmag() const {
    return { m_xmag, m_ymag };
}

} // namespace Asset
} // namespace Core
} // namespace Ra
