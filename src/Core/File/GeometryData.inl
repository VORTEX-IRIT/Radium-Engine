#include <Core/File/GeometryData.hpp>

#include <Core/Log/Log.hpp>

#include <iterator>
#include <algorithm>

namespace Ra {
    namespace Asset {


        ////////////////
        /// MATERIAL ///
        ////////////////

        /// QUERY
        inline bool MaterialData::hasDiffuse() const
        {
            return m_hasDiffuse;
        }

        inline bool MaterialData::hasSpecular() const
        {
            return m_hasSpecular;
        }

        inline bool MaterialData::hasShininess() const
        {
            return m_hasShininess;
        }

        inline bool MaterialData::hasOpacity() const
        {
            return m_hasOpacity;
        }

        inline bool MaterialData::hasDiffuseTexture() const
        {
            return m_hasTexDiffuse;
        }

        inline bool MaterialData::hasSpecularTexture() const
        {
            return m_hasTexSpecular;
        }

        inline bool MaterialData::hasShininessTexture() const
        {
            return m_hasTexShininess;
        }

        inline bool MaterialData::hasNormalTexture() const
        {
            return m_hasTexNormal;
        }

        inline bool MaterialData::hasOpacityTexture() const
        {
            return m_hasTexOpacity;
        }



        /////////////////////
        /// GEOMETRY DATA ///
        /////////////////////

        /// NAME
        inline void GeometryData::setName( const std::string& name )
        {
            m_name = name;
        }

        /// TYPE
        inline GeometryData::GeometryType GeometryData::getType() const
        {
            return m_type;
        }

        inline void GeometryData::setType( const GeometryType& type )
        {
            m_type = type;
        }

        /// FRAME
        inline Core::Transform GeometryData::getFrame() const
        {
            return m_frame;
        }

        inline void GeometryData::setFrame( const Core::Transform& frame )
        {
            m_frame = frame;
        }

        /// DATA
        inline uint GeometryData::getVerticesSize() const
        {
            return m_vertex.size();
        }

        inline const GeometryData::Vector3Array& GeometryData::getVertices() const
        {
            return m_vertex;
        }

        inline GeometryData::Vector3Array& GeometryData::getVertices()
        {
            return m_vertex;
        }

        template < typename Container >
        inline void GeometryData::setVertices( const Container &vertexList )
        {
            const uint size = vertexList.size();
            m_vertex.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_vertex[i] = Core::Vector3(vertexList[i].template cast<Core::Vector3::Scalar>());
            }
        }

        inline GeometryData::Vector2uArray& GeometryData::getEdges()
        {
            return m_edge;
        }

        inline const GeometryData::Vector2uArray& GeometryData::getEdges() const
        {
            return m_edge;
        }

        template < typename Container >
        inline void GeometryData::setEdges( const Container& edgeList )
        {
            const uint size = edgeList.size();
            m_edge.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_edge[i] = Core::Vector2ui(edgeList[i].template cast<Core::Vector2ui::Scalar>());
            }
        }

        inline const GeometryData::VectorNuArray& GeometryData::getFaces() const
        {
            return m_faces;
        }

        inline GeometryData::VectorNuArray& GeometryData::getFaces()
        {
            return m_faces;
        }

        template < typename Container >
        inline void GeometryData::setFaces( const Container& faceList )
        {
            const uint size = faceList.size();
            m_faces.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_faces[i] = Core::VectorNui(faceList[i].template cast<Core::VectorNui::Scalar>());
            }
        }

        inline GeometryData::VectorNuArray& GeometryData::getPolyhedra()
        {
            return m_polyhedron;
        }

        inline const GeometryData::VectorNuArray& GeometryData::getPolyhedra() const
        {
            return m_polyhedron;
        }

        template < typename Container >
        inline void GeometryData::setPolyhedron( const Container& polyList )
        {
            const uint size = polyList.size();
            m_polyhedron.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_polyhedron[i] = Core::VectorNui(polyList[i].template cast<Core::VectorNui::Scalar>());
            }
        }

        inline GeometryData::Vector3Array& GeometryData::getNormals()
        {
            return m_normal;
        }

        inline const GeometryData::Vector3Array& GeometryData::getNormals() const
        {
            return m_normal;
        }

        template < typename Container >
        inline void GeometryData::setNormals( const Container& normalList )
        {
            const uint size = normalList.size();
            m_normal.resize( size );
        #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_normal[i] = Core::Vector3(normalList[i].template cast<Core::Vector3::Scalar>());
            }
        }

        inline GeometryData::Vector3Array& GeometryData::getTangents()
        {
            return m_tangent;
        }

        inline const GeometryData::Vector3Array& GeometryData::getTangents() const
        {
            return m_tangent;
        }

        template < typename Container >
        inline void GeometryData::setTangents( const Container& tangentList )
        {
            const uint size = tangentList.size();
            m_tangent.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_tangent[i] = Core::Vector3(tangentList[i].template cast<Core::Vector3::Scalar>());
            }
        }

        inline GeometryData::Vector3Array& GeometryData::getBiTangents()
        {
            return m_bitangent;
        }

        inline const GeometryData::Vector3Array& GeometryData::getBiTangents() const
        {
            return m_bitangent;
        }

        template < typename Container >
        inline void GeometryData::setBitangents( const Container& bitangentList )
        {
            const uint size = bitangentList.size();
            m_bitangent.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_bitangent[i] = Core::Vector3(bitangentList[i].template cast<Core::Vector3::Scalar>());
            }
        }

        inline GeometryData::Vector3Array& GeometryData::getTexCoords()
        {
            return m_texCoord;
        }

        inline const GeometryData::Vector3Array& GeometryData::getTexCoords() const
        {
            return m_texCoord;
        }

        template < typename Container >
        inline void GeometryData::setTextureCoordinates( const Container& texCoordList )
        {
            const uint size = texCoordList.size();
            m_texCoord.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_texCoord[i] = Core::Vector3(texCoordList[i].template cast<Core::Vector3::Scalar>());
            }
        }

        inline GeometryData::ColorArray& GeometryData::getColors()
        {
            return m_color;
        }

        inline const GeometryData::ColorArray& GeometryData::getColors() const
        {
            return m_color;
        }

        template < typename Container >
        inline void GeometryData::setColors( const Container& colorList )
        {
            const uint size = colorList.size();
            m_color.resize( size );
            #pragma omp parallel for
            for( int i = 0; i < int(size); ++i ) {
                // unnecessary call to copy constructor and cast are removed at compile time
                m_color[i] = Core::Color(colorList[i].template cast<Core::Color::Scalar>());
            }
        }

        inline GeometryData::WeightArray& GeometryData::getWeights()
        {
            return m_weights;
        }

        inline const GeometryData::WeightArray& GeometryData::getWeights() const
        {
            return m_weights;
        }

        inline void GeometryData::setWeights(const WeightArray& weightList)
        {
            m_weights = std::move(weightList);
        }

        inline const MaterialData& GeometryData::getMaterial() const
        {
            return m_material;
        }

        inline void GeometryData::setMaterial( const MaterialData& material )
        {
            m_material = material;
            m_hasMaterial = true;
        }

        /// DUPLICATES
        inline std::vector<uint>& GeometryData::getDuplicateTable()
        {
            return m_duplicateTable;
        }
        inline const std::vector<uint>& GeometryData::getDuplicateTable() const
        {
            return m_duplicateTable;
        }

        inline void GeometryData::setDuplicateTable( const std::vector<uint>& table )
        {
            m_duplicateTable = table;
        }

        inline void GeometryData::setLoadDuplicates( const bool status )
        {
            m_loadDuplicates = status;
        }

        /// QUERY
        inline bool GeometryData::isPointCloud() const
        {
            return ( m_type == POINT_CLOUD );
        }

        inline bool GeometryData::isLineMesh() const
        {
            return ( m_type == LINE_MESH );
        }

        inline bool GeometryData::isTriMesh() const
        {
            return ( m_type == TRI_MESH );
        }

        inline bool GeometryData::isQuadMesh() const
        {
            return ( m_type == QUAD_MESH );
        }

        inline bool GeometryData::isPolyMesh() const
        {
            return ( m_type == POLY_MESH );
        }

        inline bool GeometryData::isTetraMesh() const
        {
            return ( m_type == TETRA_MESH );
        }

        inline bool GeometryData::isHexMesh() const
        {
            return ( m_type == HEX_MESH );
        }

        inline bool GeometryData::hasVertices() const
        {
            return !m_vertex.empty();
        }

        inline bool GeometryData::hasEdges() const
        {
            return !m_edge.empty();
        }

        inline bool GeometryData::hasFaces() const
        {
            return !m_faces.empty();
        }

        inline bool GeometryData::hasPolyhedra() const
        {
            return !m_polyhedron.empty();
        }

        inline bool GeometryData::hasNormals() const
        {
            return !m_normal.empty();
        }

        inline bool GeometryData::hasTangents() const
        {
            return !m_tangent.empty();
        }

        inline bool GeometryData::hasBiTangents() const
        {
            return !m_bitangent.empty();
        }

        inline bool GeometryData::hasTextureCoordinates() const
        {
            return !m_texCoord.empty();
        }

        inline bool GeometryData::hasColors() const
        {
            return !m_color.empty();
        }

        inline bool GeometryData::hasWeights() const
        {
            return !m_weights.empty();
        }

        inline bool GeometryData::hasMaterial() const
        {
            return m_hasMaterial;
        }

        inline bool GeometryData::isLoadingDuplicates() const
        {
            return m_loadDuplicates;
        }

        /// DEBUG
        inline void GeometryData::displayInfo() const
        {
            std::string type;
            switch( m_type ) {
                case UNKNOWN     : type = "UNKNOWN";       break;
                case POINT_CLOUD : type = "POINT CLOUD";   break;
                case LINE_MESH   : type = "LINE MESH";     break;
                case TRI_MESH    : type = "TRIANGLE MESH"; break;
                case QUAD_MESH   : type = "QUAD MESH";     break;
                case POLY_MESH   : type = "POLY MESH";     break;
                case TETRA_MESH  : type = "TETRA MESH";    break;
                case HEX_MESH    : type = "HEX MESH";      break;
            }
            LOG( logINFO ) << "======== MESH INFO ========";
            LOG( logINFO ) << " Name           : " << m_name;
            LOG( logINFO ) << " Type           : " << type;
            LOG( logINFO ) << " Vertex #       : " << m_vertex.size();
            LOG( logINFO ) << " Edge #         : " << m_edge.size();
            LOG( logINFO ) << " Face #         : " << m_faces.size();
            LOG( logINFO ) << " Normal ?       : " << ( ( m_normal.empty()    ) ? "NO" : "YES" );
            LOG( logINFO ) << " Tangent ?      : " << ( ( m_tangent.empty()   ) ? "NO" : "YES" );
            LOG( logINFO ) << " Bitangent ?    : " << ( ( m_bitangent.empty() ) ? "NO" : "YES" );
            LOG( logINFO ) << " Tex.Coord. ?   : " << ( ( m_texCoord.empty()  ) ? "NO" : "YES" );
            LOG( logINFO ) << " Color ?        : " << ( ( m_color.empty()     ) ? "NO" : "YES" );
            LOG( logINFO ) << " Material ?     : " << ( ( !m_hasMaterial      ) ? "NO" : "YES" );
            LOG( logINFO ) << " Has Dup. Vert. : " << ( ( m_duplicateTable.size() == m_vertex.size() ) ? "NO" : "YES" );

            if( m_hasMaterial ) {
                std::string kd;
                std::string ks;
                std::string ns;
                std::string op;

                if( m_material.hasDiffuse() ) {
                    Core::StringUtils::stringPrintf( kd, "%.3f %.3f %.3f %.3f",
                                                     m_material.m_diffuse.x(),
                                                     m_material.m_diffuse.y(),
                                                     m_material.m_diffuse.z(),
                                                     m_material.m_diffuse.w() );
                }

                if( m_material.hasSpecular() )
                {
                    Core::StringUtils::stringPrintf( ks, "%.3f %.3f %.3f %.3f",
                                                     m_material.m_specular.x(),
                                                     m_material.m_specular.y(),
                                                     m_material.m_specular.z(),
                                                     m_material.m_specular.w() );
                }

                if( m_material.hasShininess() )
                {
                    Core::StringUtils::stringPrintf( ns, "%.1f", m_material.m_shininess );
                }

                if (m_material.hasOpacity())
                {
                    Core::StringUtils::stringPrintf(op, "%.15f", m_material.m_opacity);
                }

                LOG( logINFO ) << "======== MATERIAL INFO ========";
                LOG( logINFO ) << " Kd             : " << ( m_material.hasDiffuse()          ? kd                        : "NO" );
                LOG( logINFO ) << " Ks             : " << ( m_material.hasSpecular()         ? ks                        : "NO" );
                LOG( logINFO ) << " Ns             : " << ( m_material.hasShininess()        ? ns                        : "NO" );
                LOG( logINFO ) << " Opacity        : " << ( m_material.hasOpacity()          ? op                        : "NO" );
                LOG( logINFO ) << " Kd Texture     : " << ( m_material.hasDiffuseTexture()   ? m_material.m_texDiffuse   : "NO" );
                LOG( logINFO ) << " Ks Texture     : " << ( m_material.hasSpecularTexture()  ? m_material.m_texSpecular  : "NO" );
                LOG( logINFO ) << " Ns Texture     : " << ( m_material.hasShininessTexture() ? m_material.m_texShininess : "NO" );
                LOG( logINFO ) << " Normal Texture : " << ( m_material.hasNormalTexture()    ? m_material.m_texNormal    : "NO" );
                LOG( logINFO ) << " Alpha Texture  : " << ( m_material.hasOpacityTexture()   ? m_material.m_texOpacity   : "NO" );
            }
        }


    } // namespace Asset
} // namespace Ra
