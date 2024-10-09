#include <Engine/Data/Mesh.hpp>

#include <Core/Geometry/IndexedGeometry.hpp>
#include <Core/Utils/Attribs.hpp>
#include <Core/Utils/Log.hpp>
#include <Core/Utils/TypesUtils.hpp>
#include <Engine/Data/ShaderProgram.hpp>
#include <Engine/OpenGL.hpp>

#include <globjects/Buffer.h>
#include <globjects/VertexArray.h>

#include <numeric>

namespace Ra {
namespace Engine {
namespace Data {
using namespace Ra::Core::Utils;

// Dirty is initializes as false so that we do not create the vao while
// we have no data to send to the gpu.
AttribArrayDisplayable::AttribArrayDisplayable( const std::string& name,
                                                MeshRenderMode renderMode ) :
    Displayable( name ), m_renderMode { renderMode } {
    CORE_ASSERT( m_renderMode == RM_POINTS || m_renderMode == RM_LINES ||
                     m_renderMode == RM_LINE_LOOP || m_renderMode == RM_LINE_STRIP ||
                     m_renderMode == RM_TRIANGLES || m_renderMode == RM_TRIANGLE_STRIP ||
                     m_renderMode == RM_TRIANGLE_FAN || m_renderMode == RM_LINES_ADJACENCY ||
                     m_renderMode == RM_LINE_STRIP_ADJACENCY,
                 "Unsupported render mode" );

    updatePickingRenderMode();
}

size_t Mesh::getNumFaces() const {
    ///\todo fix this once we have explicit triangle fan and strip management.
    switch ( getRenderMode() ) {
    case MeshRenderMode::RM_TRIANGLE_STRIP:
        [[fallthrough]];
    case MeshRenderMode::RM_TRIANGLE_FAN:
        return ( getCoreGeometry().getIndices().size() - 1 ) * 3 + 1;
    case MeshRenderMode::RM_TRIANGLES:
        return getCoreGeometry().getIndices().size();
    default:
        return size_t( 0 );
    }
}

void Mesh::loadGeometry( const Core::Vector3Array& vertices, const std::vector<uint>& indices ) {
    // Do not remove this function to force everyone to use TriangleMesh.
    //  ... because we have some line meshes as well...
    /// \todo find an alternative
    Core::Geometry::TriangleMesh mesh;

    auto nIdx = indices.size();

    if ( indices.empty() ) {
        m_numElements = vertices.size();
        setRenderMode( RM_POINTS );
    }
    else
        m_numElements = nIdx;
    mesh.setVertices( vertices );

    // Check that when loading a TriangleMesh we actually have triangles or lines.
    CORE_ASSERT( m_renderMode != GL_TRIANGLES || nIdx % 3 == 0,
                 "There should be 3 indices per triangle " );
    CORE_ASSERT( m_renderMode != GL_LINES || nIdx % 2 == 0, "There should be 2 indices per line" );
    CORE_ASSERT( m_renderMode != GL_LINES_ADJACENCY || nIdx % 4 == 0,
                 "There should be 4 indices per line adjacency" );
    Core::Geometry::TriangleMesh::IndexContainerType mindices;
    for ( uint i = 0; i < indices.size(); i = i + 3 ) {
        // We store all indices in order. This means that for lines we have
        // (L00, L01, L10), (L11, L20, L21) etc. We fill the missing by wrapping around indices.
        mindices.push_back( { indices[i], indices[( i + 1 ) % nIdx], indices[( i + 2 ) % nIdx] } );
    }

    mesh.setIndices( std::move( mindices ) );
    m_dataDirty.clear();
    m_vbos.clear();

    ///\todo check line vs triangle here is a bug
    loadGeometry( std::move( mesh ) );
}

void AttribArrayDisplayable::updatePickingRenderMode() {
    switch ( getRenderMode() ) {
    case AttribArrayDisplayable::RM_POINTS: {
        Displayable::m_pickingRenderMode = PKM_POINTS;
        break;
    }
    case AttribArrayDisplayable::RM_LINES: // fall through
        [[fallthrough]];
    case AttribArrayDisplayable::RM_LINE_LOOP: // fall through
        [[fallthrough]];
    case AttribArrayDisplayable::RM_LINE_STRIP: {
        Displayable::m_pickingRenderMode = PKM_LINES;
        break;
    }
    case AttribArrayDisplayable::RM_LINES_ADJACENCY: // fall through
        [[fallthrough]];
    case AttribArrayDisplayable::RM_LINE_STRIP_ADJACENCY: {
        Displayable::m_pickingRenderMode = PKM_LINE_ADJ;
        break;
    }
    case AttribArrayDisplayable::RM_TRIANGLES:
        [[fallthrough]];
    case AttribArrayDisplayable::RM_TRIANGLE_STRIP:
        [[fallthrough]];
    case AttribArrayDisplayable::RM_TRIANGLE_FAN: {
        Displayable::m_pickingRenderMode = PKM_TRI;
        break;
    }
    default: {
        Displayable::m_pickingRenderMode = NO_PICKING;
        break;
    }
    }
}

void AttribArrayDisplayable::setDirty( const std::string& name ) {
    auto itr = m_handleToBuffer.find( name );
    if ( itr == m_handleToBuffer.end() ) {
        m_handleToBuffer[name] = m_dataDirty.size();
        m_dataDirty.push_back( true );
        m_vbos.emplace_back( nullptr );
    }
    else
        m_dataDirty[itr->second] = true;

    m_isDirty = true;
}

void AttribArrayDisplayable::setDirty( unsigned int index ) {
    if ( index < m_dataDirty.size() ) {
        m_dataDirty[index] = true;
        m_isDirty          = true;
    }
}

void AttribArrayDisplayable::setDirty( const Ra::Core::Geometry::MeshAttrib& type ) {
    auto name = Core::Geometry::getAttribName( type );
    auto itr  = m_handleToBuffer.find( name );
    if ( itr == m_handleToBuffer.end() ) {
        m_handleToBuffer[name] = m_dataDirty.size();
        m_dataDirty.push_back( true );
        m_vbos.emplace_back( nullptr );
    }
    else
        m_dataDirty[itr->second] = true;

    m_isDirty = true;
}
////////////////  MultiIndexedGeometry  ///////////////////////////////

GeometryDisplayable::GeometryDisplayable( const std::string& name ) : base( name ) {}

GeometryDisplayable::GeometryDisplayable( const std::string& name,
                                          typename Core::Geometry::MultiIndexedGeometry&& geom ) :
    base( name ) {
    loadGeometry( std::move( geom ) );
}

GeometryDisplayable::~GeometryDisplayable() {}

void GeometryDisplayable::loadGeometry( Core::Geometry::MultiIndexedGeometry&& mesh ) {
    m_geomLayers.clear();
    m_geom = std::move( mesh );
    setupCoreMeshObservers();

    if ( m_geom.containsLayer( Core::Geometry::LineIndexLayer::staticSemanticName ) ) {
        auto [key, layer] =
            m_geom.getFirstLayerOccurrence( Core::Geometry::LineIndexLayer::staticSemanticName );
        m_activeLayerKey = key;
        setRenderMode( AttribArrayDisplayable::RM_LINES );

        auto ok = addRenderLayer( key, AttribArrayDisplayable::RM_LINES );
        if ( !ok ) { LOG( logERROR ) << "loadGeometry could not add layer"; }
    }
    else if ( m_geom.containsLayer( Core::Geometry::TriangleIndexLayer::staticSemanticName ) ) {
        auto [key, layer] = m_geom.getFirstLayerOccurrence(
            Core::Geometry::TriangleIndexLayer::staticSemanticName );
        m_activeLayerKey = key;
        addRenderLayer( key, AttribArrayDisplayable::RM_TRIANGLES );
    }
    else if ( m_geom.containsLayer( Core::Geometry::QuadIndexLayer::staticSemanticName ) ) {
        auto [key, layer] =
            m_geom.getFirstLayerOccurrence( Core::Geometry::QuadIndexLayer::staticSemanticName );

        const auto& quadLayer = dynamic_cast<const Core::Geometry::QuadIndexLayer&>( layer );

        auto triangleLayer          = std::make_unique<Core::Geometry::TriangleIndexLayer>();
        triangleLayer->collection() = Core::Geometry::triangulate( quadLayer.collection() );

        LayerKeyType triangleKey = { triangleLayer->semantics(), "triangulation" };
        auto layerAdded = m_geom.addLayer( std::move( triangleLayer ), false, "triangulation" );

        if ( !layerAdded.first ) { LOG( logERROR ) << "failed to add triangleLayer"; }
        else {
            m_activeLayerKey = triangleKey;
            addRenderLayer( triangleKey, AttribArrayDisplayable::RM_TRIANGLES );
        }
    }
    else if ( m_geom.containsLayer( Core::Geometry::PolyIndexLayer::staticSemanticName ) ) {
        auto [key, layer] =
            m_geom.getFirstLayerOccurrence( Core::Geometry::PolyIndexLayer::staticSemanticName );

        const auto& polyLayer =
            dynamic_cast<const Core::Geometry::PolyIndexLayer&>( m_geom.getLayer( key ) );

        auto triangleLayer          = std::make_unique<Core::Geometry::TriangleIndexLayer>();
        triangleLayer->collection() = Core::Geometry::triangulate( polyLayer.collection() );
        LayerKeyType triangleKey    = { triangleLayer->semantics(), "triangulation" };
        auto layerAdded = m_geom.addLayer( std::move( triangleLayer ), false, "triangulation" );
        if ( !layerAdded.first ) { LOG( logERROR ) << "failed to add triangleLayer"; }
        else {
            m_activeLayerKey = triangleKey;
            addRenderLayer( triangleKey, AttribArrayDisplayable::RM_TRIANGLES );
        }
    }
    else { LOG( logERROR ) << "no valid layer found"; }
    m_isDirty = true;
}

void GeometryDisplayable::setupCoreMeshObservers() {
    int idx = 0;
    m_dataDirty.resize( m_geom.vertexAttribs().getNumAttribs() );
    m_vbos.resize( m_geom.vertexAttribs().getNumAttribs() );
    // here capture ref to idx to propagate idx incrementation
    m_geom.vertexAttribs().for_each_attrib( [&idx, this]( Ra::Core::Utils::AttribBase* b ) {
        auto name              = b->getName();
        m_handleToBuffer[name] = idx;
        m_dataDirty[idx]       = true;

        // create a identity translation if name is not already translated.
        addToTranslationTable( name );

        b->attach( AttribObserver( this, idx ) );
        ++idx;
    } );

    // add an observer on attrib manipulation.
    m_geom.vertexAttribs().attachMember( this, &GeometryDisplayable::addAttribObserver );
    m_isDirty = true;
}

void GeometryDisplayable::addToTranslationTable( const std::string& name ) {
    m_translationTable.insert( name, name );
}

void GeometryDisplayable::addAttribObserver( const std::string& name ) {
    // this observer is called each time an attrib is added or removed from m_mesh
    auto attrib = m_geom.getAttribBase( name );
    // if attrib not nullptr, then it's an attrib add, so attach an observer to it

    if ( attrib ) {
        auto itr = m_handleToBuffer.find( name );
        if ( itr == m_handleToBuffer.end() ) {
            m_handleToBuffer[name] = m_dataDirty.size();

            // if there isn't a matching yet, add identity
            if ( !m_translationTable.valueIfExists( name ) ) addToTranslationTable( name );

            m_dataDirty.push_back( true );
            m_vbos.emplace_back( nullptr );
        }
        auto idx = m_handleToBuffer[name];
        attrib->attach( AttribObserver( this, idx ) );
    }
    // else it's an attrib remove, do nothing, cleanup will be done in updateGL()
    else {}
}

void GeometryDisplayable::setAttribNameMatching( const std::string& meshAttribName,
                                                 const std::string& shaderAttribName ) {

    m_translationTable.replace( meshAttribName, shaderAttribName );
}

bool GeometryDisplayable::addRenderLayer( LayerKeyType key, base::MeshRenderMode renderMode ) {
    if ( !m_geom.containsLayer( key ) ) return false;
    auto it = m_geomLayers.find( key );
    if ( it != m_geomLayers.end() ) return false;

    auto& l = m_geomLayers.insert( { key, LayerEntryType() } ).first->second;

    l.observerId    = -1;
    l.renderMode    = renderMode;
    l.indices.dirty = true;

    setDirty( true );
    return true;
}

bool GeometryDisplayable::removeRenderLayer( LayerKeyType key ) {
    auto it = m_geomLayers.find( key );
    if ( it == m_geomLayers.end() ) return false;

    // the layer might have already been deleted
    if ( m_geom.containsLayer( key ) ) {
        auto& geomLayer = m_geom.getLayerWithLock( key );
        geomLayer.detach( it->second.observerId );
        m_geom.unlockLayer( key );
    }
    it->second.vao.reset();
    m_geomLayers.erase( it );

    return true;
}

void GeometryDisplayable::updateGL() {
    if ( m_isDirty ) {

        // create vao
        for ( auto& itr : m_geomLayers ) {
            const auto& abstractLayer = m_geom.getLayerWithLock( itr.first );
            auto& l                   = itr.second;

            if ( !l.vao ) { l.vao = globjects::VertexArray::create(); }

            auto& vbo = l.indices;
            if ( !vbo.buffer ) {
                vbo.buffer      = globjects::Buffer::create();
                vbo.dirty       = true;
                vbo.numElements = abstractLayer.getSize() * abstractLayer.getNumberOfComponents();
            }

            // upload data to gpu
            if ( vbo.dirty ) {
                vbo.buffer->setData( static_cast<gl::GLsizeiptr>( abstractLayer.getBufferSize() ),
                                     abstractLayer.dataPtr(),
                                     GL_STATIC_DRAW );
                vbo.dirty = false;
            }
            m_geom.unlockLayer( itr.first );
            l.vao->bind();
            l.vao->bindElementBuffer( vbo.buffer.get() );
            l.vao->unbind();
            GL_CHECK_ERROR;
        }

        auto func = [this]( Ra::Core::Utils::AttribBase* b ) {
            auto idx = m_handleToBuffer[b->getName()];

            if ( m_dataDirty[idx] ) {
                if ( !m_vbos[idx] ) { m_vbos[idx] = globjects::Buffer::create(); }
                m_vbos[idx]->setData( b->getBufferSize(), b->dataPtr(), GL_DYNAMIC_DRAW );
                m_dataDirty[idx] = false;
            }
        };

        m_geom.vertexAttribs().for_each_attrib( func );

        // cleanup removed attrib
        for ( auto buffer : m_handleToBuffer ) {
            // do not remove name from handleToBuffer to keep index ...
            // we could also update handleToBuffer, m_vbos, m_dataDirty
            if ( !m_geom.hasAttrib( buffer.first ) && m_vbos[buffer.second] ) {
                m_vbos[buffer.second].reset( nullptr );
                m_dataDirty[buffer.second] = false;
            }
        }

        GL_CHECK_ERROR;
        m_isDirty = false;
    }
}

void GeometryDisplayable::render( const ShaderProgram* prog ) {
    render( prog, m_activeLayerKey );
}

void GeometryDisplayable::render( const ShaderProgram* prog, const LayerKeyType& key ) {
    GL_CHECK_ERROR;
    if ( m_geomLayers.find( key ) != m_geomLayers.end() ) {
        if ( m_geomLayers[key].vao ) {
            m_geomLayers[key].vao->bind();
            GL_CHECK_ERROR;
            autoVertexAttribPointer( prog, key );
            GL_CHECK_ERROR;
            m_geomLayers[key].vao->drawElements(
                static_cast<GLenum>( m_geomLayers[key].renderMode ),
                GLsizei( m_geomLayers[key].indices.numElements ),
                GL_UNSIGNED_INT,
                nullptr );
            GL_CHECK_ERROR;
            m_geomLayers[key].vao->unbind();
            GL_CHECK_ERROR;
        }
        else {
            LOG( logERROR ) << getName() << "try to draw an invalid layer " << *key.first.begin()
                            << " [" << key.second << "]\n";
        }
    }
    else { LOG( logERROR ) << "layer was not added as a render layer"; }
}

void GeometryDisplayable::autoVertexAttribPointer( const ShaderProgram* prog,
                                                   const LayerKeyType& key ) {

    auto glprog           = prog->getProgramObject();
    gl::GLint attribCount = glprog->get( GL_ACTIVE_ATTRIBUTES );

    for ( GLint idx = 0; idx < attribCount; ++idx ) {
        const gl::GLsizei bufSize = 256;
        gl::GLchar name[bufSize];
        gl::GLsizei length;
        gl::GLint size;
        gl::GLenum type;
        glprog->getActiveAttrib( idx, bufSize, &length, &size, &type, name );
        auto loc = glprog->getAttributeLocation( name );

        auto attribNameOpt = m_translationTable.keyIfExists( name );
        if ( attribNameOpt ) {
            auto attribName = *attribNameOpt;
            auto attrib     = m_geom.getAttribBase( attribName );
            if ( attrib && attrib->getSize() > 0 ) {
                m_geomLayers[key].vao->enable( loc );
                auto binding = m_geomLayers[key].vao->binding( idx );

                binding->setAttribute( loc );
                CORE_ASSERT( m_vbos[m_handleToBuffer[attribName]].get(), "vbo is nullptr" );
#ifdef CORE_USE_DOUBLE
                binding->setBuffer( m_vbos[m_handleToBuffer[attribName]].get(),
                                    0,
                                    attrib->getNumberOfComponents() * sizeof( float ) );
#else

                binding->setBuffer(
                    m_vbos[m_handleToBuffer[attribName]].get(), 0, attrib->getStride() );
#endif
                binding->setFormat( attrib->getNumberOfComponents(), GL_SCALAR );
            }
            else { m_geomLayers[key].vao->disable( loc ); }
        }
        else { m_geomLayers[key].vao->disable( loc ); }
        GL_CHECK_ERROR;
    }
}

Ra::Core::Utils::optional<gl::GLuint> AttribArrayDisplayable::getVaoHandle() {
    if ( m_vao ) return m_vao->id();
    return {};
}
Ra::Core::Utils::optional<gl::GLuint>
AttribArrayDisplayable::getVboHandle( const std::string& name ) {
    auto idx = m_handleToBuffer.find( name );
    if ( idx != m_handleToBuffer.end() && m_vbos[idx->second] ) return m_vbos[idx->second]->id();
    return {};
}

void PointCloud::render( const ShaderProgram* prog ) {
    if ( m_vao ) {
        autoVertexAttribPointer( prog );
        m_vao->bind();
        m_vao->drawArrays(
            static_cast<GLenum>( m_renderMode ), 0, GLsizei( m_mesh.vertices().size() ) );
        m_vao->unbind();
    }
}

void PointCloud::loadGeometry( Core::Geometry::PointCloud&& mesh ) {
    loadGeometry_common( std::move( mesh ) );
}

void PointCloud::updateGL_specific_impl() {
    if ( !m_vao ) { m_vao = globjects::VertexArray::create(); }
}

} // namespace Data
} // namespace Engine
} // namespace Ra
