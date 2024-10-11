#pragma once

#include <Engine/RaEngine.hpp>

#include <Core/Geometry/Obb.hpp>
#include <Core/Geometry/Spline.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Color.hpp>

#include <memory>

namespace Ra {
namespace Engine {
namespace Rendering {
class RenderObject;

}

namespace Scene {
class Component;
}

namespace Data {
class Mesh;
class AttribArrayDisplayable;
class LineMesh;

/// A set of convenient functions to instantiate simple displays such as points, lines, etc.
/// note that objects will be drawn in their entity's local frame.
/// For "instant" debug drawing, \see DebugDisplay.
namespace DrawPrimitives {
using MeshPtr                   = std::shared_ptr<Mesh>;
using LineMeshPtr               = std::shared_ptr<LineMesh>;
using AttribArrayDisplayablePtr = std::shared_ptr<AttribArrayDisplayable>;

///\{
/// Renturn a render object to display the given geometry
RA_ENGINE_API Rendering::RenderObject* Primitive( Scene::Component* comp, const MeshPtr& mesh );
RA_ENGINE_API Rendering::RenderObject* Primitive( Scene::Component* comp, const LineMeshPtr& mesh );
RA_ENGINE_API Rendering::RenderObject* Primitive( Scene::Component* comp,
                                                  const AttribArrayDisplayablePtr& mesh );
///\}

/// Displays given point shown as the crossing of 3 lines of length 'scale'
RA_ENGINE_API LineMeshPtr Point( const Core::Vector3& point,
                                 const Core::Utils::Color& color,
                                 Scalar scale = 0.1f );

/// Displays given line
RA_ENGINE_API LineMeshPtr Line( const Core::Vector3& a,
                                const Core::Vector3& b,
                                const Core::Utils::Color& color );

/// Displays given vector shown as an arrow originating from 'start'
RA_ENGINE_API LineMeshPtr Vector( const Core::Vector3& start,
                                  const Core::Vector3& v,
                                  const Core::Utils::Color& color );

/// Displays given ray as a straight line.
RA_ENGINE_API LineMeshPtr Ray( const Core::Ray& ray,
                               const Core::Utils::Color& color,
                               Scalar len = 1000_ra );

/// Displays given triangle ABC, either in wireframe (fill = false)
/// or filled with the color(fill = true).
RA_ENGINE_API AttribArrayDisplayablePtr Triangle( const Core::Vector3& a,
                                                  const Core::Vector3& b,
                                                  const Core::Vector3& c,
                                                  const Core::Utils::Color& color,
                                                  bool fill = false );

/// Displays a strip of n quads, starting at A and with directions X and Y.
RA_ENGINE_API MeshPtr QuadStrip( const Core::Vector3& a,
                                 const Core::Vector3& x,
                                 const Core::Vector3& y,
                                 uint quads,
                                 const Core::Utils::Color& color );

/// Displays circle computed with given center and radius,
/// in plane normal to given vector in wireframe
/// \note normal must be a normalized vector.
RA_ENGINE_API LineMeshPtr Circle( const Core::Vector3& center,
                                  const Core::Vector3& normal,
                                  Scalar radius,
                                  uint segments,
                                  const Core::Utils::Color& color );

/// Displays arc of a circle computed with given center, radius and angle
/// in plane normal to given vector in wireframe
/// \note normal must be a normalized vector.
RA_ENGINE_API LineMeshPtr CircleArc( const Core::Vector3& center,
                                     const Core::Vector3& normal,
                                     Scalar radius,
                                     Scalar angle,
                                     uint segments,
                                     const Core::Utils::Color& color );

/// Displays geodesic sphere computed with given center and radius
RA_ENGINE_API MeshPtr Sphere( const Core::Vector3& center,
                              Scalar radius,
                              const Core::Utils::Color& color );

/// Displays uv sphere computed with given center and radius
/// \param generateTexCoord: ask texture coordinate generation
RA_ENGINE_API MeshPtr ParametricSphere( const Core::Vector3& center,
                                        Scalar radius,
                                        const Core::Utils::Color& color,
                                        bool generateTexCoord = false );

/// Displays a capsule computed with given endpoints and radius.
RA_ENGINE_API MeshPtr Capsule( const Core::Vector3& p1,
                               const Core::Vector3& p2,
                               Scalar radius,
                               const Core::Utils::Color& color );

/// Displays disk (filled circle) computed with given center and radius,
/// in plane normal to given vector in wireframe
/// \note normal must be a normalized vector.
RA_ENGINE_API MeshPtr Disk( const Core::Vector3& center,
                            const Core::Vector3& normal,
                            Scalar radius,
                            uint segments,
                            const Core::Utils::Color& color );

/// Displays a normal vector emanating from the given point as a vector arrow
/// and a normal plane of size 'scale'.
RA_ENGINE_API LineMeshPtr Normal( const Core::Vector3& point,
                                  const Core::Vector3& normal,
                                  const Core::Utils::Color& color,
                                  Scalar scale = 0.1f );

/// Displays a 3D frame representing the given transform.
/// Each axis has length 'scale' and are in usual colors
/// (X,Y,Z = red, green blue)
/// Remainder : the transform will be drawn relative
/// to the component's entity transform.
RA_ENGINE_API MeshPtr Frame( const Core::Transform& frameFromEntity, Scalar scale = 0.1f );

/// Create a res*res square grid centered on center,
/// in plane defined by the vectors x and y.
RA_ENGINE_API MeshPtr Grid( const Core::Vector3& center,
                            const Core::Vector3& x,
                            const Core::Vector3& y,
                            const Core::Utils::Color& color,
                            Scalar cellSize = 1.f,
                            uint res        = 10 );

/// Display a wireframe AABB
RA_ENGINE_API MeshPtr AABB( const Core::Aabb& aabb, const Core::Utils::Color& color );

/// Display a wireframe OBB
RA_ENGINE_API MeshPtr OBB( const Core::Geometry::Obb& obb, const Core::Utils::Color& color );

RA_ENGINE_API MeshPtr Spline( const Core::Geometry::Spline<3, 3>& spline,
                              uint pointCount,
                              const Core::Utils::Color& color,
                              Scalar scale = 1.0f );

/// Display a line strip.
RA_ENGINE_API MeshPtr LineStrip( const Core::Vector3Array& vertices,
                                 const Core::Vector4Array& colors );

} // namespace DrawPrimitives
} // namespace Data
} // namespace Engine
} // namespace Ra
