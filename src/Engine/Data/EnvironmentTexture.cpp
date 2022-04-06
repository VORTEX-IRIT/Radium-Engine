#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

#include <cstring>

#include <Engine/Data/EnvironmentTexture.hpp>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
//#undef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#undef STB_IMAGE_WRITE_IMPLEMENTATION

#include <Core/Asset/Camera.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Math/Math.hpp>
#include <Core/Resources/Resources.hpp>

#include <Engine/Data/Mesh.hpp>
#include <Engine/Data/ShaderProgram.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/Data/ViewingParameters.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/CameraManager.hpp>

#include <tinyEXR/tinyexr.h>

namespace Ra {
namespace Engine {
namespace Data {
using namespace gl;

// Flip horizontally an image of w x h pixels with c commponents
template <typename T>
void flip_horizontally( T* img, size_t w, size_t h, size_t c ) {
#pragma omp parallel for
    for ( int r = 0; r < int( h ); ++r ) {
        for ( size_t l = 0; l < w / 2; ++l ) {
            T* from = img + ( l * c );
            T* to   = img + ( w - ( l + 1 ) ) * c;
            for ( size_t e = 0; e < c; ++e ) {
                std::swap( *( from + e ), *( to + e ) );
            }
        }
        img += w * c;
    }
}

// -------------------------------------------------------------------
class PfmReader
{
  public:
    typedef struct {
        float r, g, b;
    } HDRPIXEL;

    typedef struct {
        int width;
        int height;
        HDRPIXEL* pixels;
    } HDRIMAGE;
    /*
   Read a possibly byte swapped floating point number
   Assume IEEE format
   */
    static HDRIMAGE* open( std::string fileName ) {
        FILE* fptr = fopen( fileName.c_str(), "rb" );
        if ( nullptr == fptr ) {
            std::cerr << "could not open " << fileName << std::endl;
            return nullptr;
        }
        int width, height, color, swap;
        if ( !readHeader( fptr, width, height, color, swap ) ) {
            std::cerr << "bad header " << fileName << std::endl;
            return nullptr;
        }

        HDRPIXEL* pixels = new HDRPIXEL[width * height];
        for ( int i = 0; i < width * height; ++i ) {
            float* ptr = &( pixels[i].r );
            if ( !readFloat( fptr, ptr, swap ) ) {
                std::cerr << "could not read pixel " << i << std::endl;
            }
            if ( color == 3 ) {
                if ( !readFloat( fptr, ptr + 1, swap ) ) {
                    std::cerr << "could not read pixel " << i << std::endl;
                }
                if ( !readFloat( fptr, ptr + 2, swap ) ) {
                    std::cerr << "could not read pixel " << i << std::endl;
                }
            }
        }
        HDRIMAGE* ret = new HDRIMAGE;
        ret->width    = width;
        ret->height   = height;
        ret->pixels   = pixels;
        return ret;
    }

    static bool readFloat( FILE* fptr, float* n, int swap ) {
        unsigned char *cptr, tmp;

        if ( fread( n, 4, 1, fptr ) != 1 ) return false;
        if ( swap ) {
            cptr    = (unsigned char*)n;
            tmp     = cptr[0];
            cptr[0] = cptr[3];
            cptr[3] = tmp;
            tmp     = cptr[1];
            cptr[1] = cptr[2];
            cptr[2] = tmp;
        }

        return true;
    }

    static bool readHeader( FILE* fptr, int& width, int& height, int& color, int& swap ) {
        char buf[32];
        // read first line
        if ( NULL == fgets( buf, 32, fptr ) ) return false;
        // determine gray or color : PF -> color, Pf -> gray
        if ( 0 == strncmp( "PF", buf, 2 ) )
            color = 3;
        else if ( 0 == strncmp( "Pf", buf, 2 ) )
            color = 1;
        else
            return false;
        // read second line
        if ( NULL == fgets( buf, 32, fptr ) ) return false;
        // determine image size
        char* endptr = NULL;
        width        = strtol( buf, &endptr, 10 );
        if ( endptr == buf ) return false;
        height = strtol( endptr, (char**)NULL, 10 );
        // read third line
        if ( NULL == fgets( buf, 32, fptr ) ) return false;
        if ( endptr == buf ) return false;
        float aspect = strtof( buf, &endptr );
        if ( aspect > 0 )
            swap = 1;
        else
            swap = 0;
        return true;
    }
};

// -------------------------------------------------------------------
using namespace Ra::Core;

// -------------------------------------------------------------------
EnvironmentTexture::EnvironmentTexture( const std::string& mapName, bool isSkybox ) :
    m_name( mapName ),
    m_skyData { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
    m_isSkyBox( isSkybox ) {
    m_type = ( mapName.find( ';' ) != mapName.npos ) ? EnvironmentTexture::EnvMapType::ENVMAP_CUBE
                                                     : EnvironmentTexture::EnvMapType::ENVMAP_PFM;
    if ( m_type == EnvironmentTexture::EnvMapType::ENVMAP_PFM ) {
        auto ext = mapName.substr( mapName.size() - 3 );
        if ( ext != "pfm" ) { m_type = EnvironmentTexture::EnvMapType::ENVMAP_LATLON; }
    }
    initializeTexture();
}

void EnvironmentTexture::initializeTexture() {
    switch ( m_type ) {
    case EnvMapType::ENVMAP_PFM:
        setupTexturesFromPfm();
        break;
    case EnvMapType::ENVMAP_CUBE:
        setupTexturesFromCube();
        break;
    case EnvMapType::ENVMAP_LATLON:
        setupTexturesFromSphericalEquiRectangular();
        break;
    }
    computeSHMatrices();
    // make the envmap cube texture
    Ra::Engine::Data::TextureParameters params { m_name,
                                                 GL_TEXTURE_CUBE_MAP,
                                                 m_width,
                                                 m_height,
                                                 1,
                                                 GL_RGBA,
                                                 GL_RGBA,
                                                 GL_FLOAT,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_LINEAR_MIPMAP_LINEAR,
                                                 GL_LINEAR,
                                                 (void**)( m_skyData ) };
    m_skyTexture = std::make_unique<Ra::Engine::Data::Texture>( params );

    if ( m_isSkyBox ) {
        // make the skybox geometry
        Aabb aabb( Vector3 { -1_ra, -1_ra, -1_ra }, Vector3 { 1_ra, 1_ra, 1_ra } );
        Geometry::TriangleMesh skyMesh = Geometry::makeSharpBox( aabb );
        m_displayMesh                  = std::make_unique<Ra::Engine::Data::Mesh>( "skyBox" );
        m_displayMesh->loadGeometry( std::move( skyMesh ) );
    }
}

void EnvironmentTexture::setupTexturesFromPfm() {
    PfmReader::HDRIMAGE* img = PfmReader::open( m_name.c_str() );
    m_width                  = img->width / 3;
    m_height                 = img->height / 4;
#pragma omp parallel for
    for ( int imgIdx = 0; imgIdx < 6; ++imgIdx ) {
        m_skyData[imgIdx] = new float[m_width * m_height * 4];
        int xOffset       = 0;
        int yOffset       = 0;
        switch ( imgIdx ) {
        case 0: // X- side
            xOffset = 1 * m_width;
            yOffset = 0 * m_height;
            break;
        case 1: // X+ side
            xOffset = 1 * m_width;
            yOffset = 2 * m_height;
            break;
        case 2: // Y+ side
            xOffset = 1 * m_width;
            yOffset = 3 * m_height;
            break;
        case 3: // Y- side
            xOffset = 1 * m_width;
            yOffset = 1 * m_height;
            break;
        case 4: // Z+ side
            xOffset = 0 * m_width;
            yOffset = 2 * m_height;
            break;
        case 5: // Z- side
            xOffset = 2 * m_width;
            yOffset = 2 * m_height;
            break;
        default:
            xOffset = 0;
            yOffset = 0;
            break;
        }
        for ( size_t i = 0; i < m_height; ++i )
            for ( size_t j = 0; j < m_width; ++j ) {
                if ( imgIdx == 1 || imgIdx == 4 || imgIdx == 5 ) {
                    m_skyData[imgIdx][4 * ( ( m_height - 1 - i ) * m_width + j ) + 0] =
                        img->pixels[( i + yOffset ) * img->width + j + xOffset].r;
                    m_skyData[imgIdx][4 * ( ( m_height - 1 - i ) * m_width + j ) + 1] =
                        img->pixels[( i + yOffset ) * img->width + j + xOffset].g;
                    m_skyData[imgIdx][4 * ( ( m_height - 1 - i ) * m_width + j ) + 2] =
                        img->pixels[( i + yOffset ) * img->width + j + xOffset].b;
                    m_skyData[imgIdx][4 * ( ( m_height - 1 - i ) * m_width + j ) + 3] = 1.f;
                }
                else {
                    if ( imgIdx == 2 ) {
                        m_skyData[imgIdx]
                                 [4 * ( ( m_width - 1 - j ) * m_height + m_height - 1 - i ) + 0] =
                                     img->pixels[( i + yOffset ) * img->width + j + xOffset].r;
                        m_skyData[imgIdx]
                                 [4 * ( ( m_width - 1 - j ) * m_height + m_height - 1 - i ) + 1] =
                                     img->pixels[( i + yOffset ) * img->width + j + xOffset].g;
                        m_skyData[imgIdx]
                                 [4 * ( ( m_width - 1 - j ) * m_height + m_height - 1 - i ) + 2] =
                                     img->pixels[( i + yOffset ) * img->width + j + xOffset].b;
                        m_skyData[imgIdx]
                                 [4 * ( ( m_width - 1 - j ) * m_height + m_height - 1 - i ) + 3] =
                                     1.f;
                    }
                    else {
                        if ( imgIdx == 0 ) {
                            m_skyData[imgIdx][4 * ( i * m_width + m_width - 1 - j ) + 0] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].r;
                            m_skyData[imgIdx][4 * ( i * m_width + m_width - 1 - j ) + 1] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].g;
                            m_skyData[imgIdx][4 * ( i * m_width + m_width - 1 - j ) + 2] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].b;
                            m_skyData[imgIdx][4 * ( i * m_width + m_width - 1 - j ) + 3] = 1.f;
                        }
                        else {
                            // imgIdx == 3
                            m_skyData[imgIdx][4 * ( j * m_width + i ) + 0] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].r;
                            m_skyData[imgIdx][4 * ( j * m_width + i ) + 1] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].g;
                            m_skyData[imgIdx][4 * ( j * m_width + i ) + 2] =
                                img->pixels[( i + yOffset ) * img->width + j + xOffset].b;
                            m_skyData[imgIdx][4 * ( j * m_width + i ) + 3] = 1.f;
                        }
                    }
                }
            }
        flip_horizontally( m_skyData[imgIdx], m_width, m_height, 4 );
    }
    delete[] img->pixels;
    delete img;
}

void EnvironmentTexture::setupTexturesFromCube() {
    std::stringstream imgs( m_name );
    std::string imgname;
    while ( getline( imgs, imgname, ';' ) ) {
        int imgIdx { -1 };
        bool flipV { false };
        // is it a +X face ?
        if ( ( imgname.find( "posx" ) != imgname.npos ) ||
             ( imgname.find( "-X-plux" ) != imgname.npos ) ) {
            imgIdx = 0;
        }
        // is it a -X face ?
        if ( ( imgname.find( "negx" ) != imgname.npos ) ||
             ( imgname.find( "-X-minux" ) != imgname.npos ) ) {
            imgIdx = 1;
        }
        // is it a +Y face ?
        if ( ( imgname.find( "posy" ) != imgname.npos ) ||
             ( imgname.find( "-Y-plux" ) != imgname.npos ) ) {
            imgIdx = 2;
            flipV  = true;
        }
        // is it a -Y face ?
        if ( ( imgname.find( "negy" ) != imgname.npos ) ||
             ( imgname.find( "-Y-minux" ) != imgname.npos ) ) {
            imgIdx = 3;
            flipV  = true;
        }
        // is it a +Z face ? --> goes to the -Z of cubemap, need to flip horizontally
        if ( ( imgname.find( "posz" ) != imgname.npos ) ||
             ( imgname.find( "-Z-plux" ) != imgname.npos ) ) {
            imgIdx = 5;
        }
        // is it a -Z face ? --> goes to the +Z of cubemap, need to flip horizontally
        if ( ( imgname.find( "negz" ) != imgname.npos ) ||
             ( imgname.find( "-Z-minux" ) != imgname.npos ) ) {
            imgIdx = 4;
        }

        int n;
        int w;
        int h;
        stbi_set_flip_vertically_on_load( flipV );
        auto loaded       = stbi_loadf( imgname.c_str(), &w, &h, &n, 0 );
        m_width           = w;
        m_height          = h;
        m_skyData[imgIdx] = new float[m_width * m_height * 4];
        for ( size_t l = 0; l < m_height; ++l ) {
            for ( size_t c = 0; c < m_width; ++c ) {
                auto is                       = ( l * m_width + c );
                auto id                       = flipV ? is : ( l * m_width + ( m_width - c - 1 ) );
                m_skyData[imgIdx][id * 4 + 0] = loaded[is * n + 0];
                m_skyData[imgIdx][id * 4 + 1] = loaded[is * n + 1];
                m_skyData[imgIdx][id * 4 + 2] = loaded[is * n + 2];
                m_skyData[imgIdx][id * 4 + 3] = 0;
            }
        }
        stbi_image_free( loaded );
    }
}

void EnvironmentTexture::setupTexturesFromSphericalEquiRectangular() {
    auto ext = m_name.substr( m_name.size() - 3 );
    float* latlonPix { nullptr };
    int n, w, h;
    if ( ext == "exr" ) {
        const char* err { nullptr };
        int ret = LoadEXR( &latlonPix, &w, &h, m_name.c_str(), &err );
        n       = 4;
        if ( ret != TINYEXR_SUCCESS ) {
            if ( err ) {
                std::cerr << "Error reading " << m_name << " : " << err << std::endl;
                FreeEXRErrorMessage( err ); // release memory of error message.
            }
        }
    }
    else {
        stbi_set_flip_vertically_on_load( false );
        latlonPix = stbi_loadf( m_name.c_str(), &w, &h, &n, 4 );
    }
    int textureSize = 1;
    while ( textureSize < h ) {
        textureSize <<= 1;
    }
    textureSize >>= 1;
    // Bases to use to convert sphericalequirectangular images to cube faces
    // These bases allow to convert (u, v) cordinates of each faces to (x, y, z) in the frame of the
    // equirectangular map. : (x, y, z) = u*A[0] + v*A[1] + A[2]
    Vector3 bases[6][3] = { { { -1, 0, 0 }, { 0, 1, 0 }, { 0, 0, -1 } },
                            { { 1, 0, 0 }, { 0, -1, 0 }, { 0, 0, -1 } },
                            { { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 } },
                            { { 0, 0, -1 }, { 1, 0, 0 }, { 0, -1, 0 } },
                            { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 } },
                            { { 0, -1, 0 }, { -1, 0, 0 }, { 0, 0, -1 } } };
    auto sphericalPhi   = []( const Vector3& d ) {
        Scalar p = std::atan2( d.x(), d.y() );
        return ( p < 0 ) ? ( p + 2 * M_PI ) : p;
    };
    auto sphericalTheta = []( const Vector3& d ) { return std::acos( d.z() ); };
#pragma omp parallel for
    for ( int imgIdx = 0; imgIdx < 6; ++imgIdx ) {
        // Alllocate the images
        m_skyData[imgIdx] = new float[textureSize * textureSize * 4];
        // Fill in pixels
        for ( Scalar u = -1_ra; u < 1_ra; u += 2_ra / textureSize ) {
            for ( Scalar v = -1_ra; v < 1_ra; v += 2_ra / textureSize ) {
                Vector3 d = bases[imgIdx][0] + u * bases[imgIdx][1] + v * bases[imgIdx][2];
                d         = d.normalized();
                Vector2 st { w * sphericalPhi( d ) / ( 2 * M_PI ), h * sphericalTheta( d ) / M_PI };
                // TODO : use st to access and filter the original envmap
                // for now, no filtering is done. (eq to GL_NEAREST)
                int s  = int( st.x() );
                int t  = int( st.y() );
                int cu = int( ( u / 2 + 0.5 ) * textureSize );
                int cv = int( ( v / 2 + 0.5 ) * textureSize );

                m_skyData[imgIdx][4 * ( cv * textureSize + cu ) + 0] =
                    latlonPix[4 * ( t * w + s ) + 0];
                m_skyData[imgIdx][4 * ( cv * textureSize + cu ) + 1] =
                    latlonPix[4 * ( t * w + s ) + 1];
                m_skyData[imgIdx][4 * ( cv * textureSize + cu ) + 2] =
                    latlonPix[4 * ( t * w + s ) + 2];
                m_skyData[imgIdx][4 * ( cv * textureSize + cu ) + 3] = 1;
            }
        }
        flip_horizontally( m_skyData[imgIdx], textureSize, textureSize, 4 );
    }
    free( latlonPix );
    m_width = m_height = textureSize;
}

void EnvironmentTexture::computeSHMatrices() {
    for ( int i = 0; i < 9; i++ ) {
        for ( int j = 0; j < 3; j++ ) {
            m_shcoefs[i][j] = 0.f;
        }
    }
    /// @todo replace this integration to use a sphere sampler ...
    /// Must evaluate the elementary solid angle for each sample
    const float dtheta = 0.005;
    const float dphi   = 0.005;
    for ( float theta = 0.f; theta < M_PI; theta += dtheta ) {
        for ( float phi = 0.f; phi < 2.f * M_PI; phi += dphi ) {
            auto x          = std::sin( theta ) * std::cos( phi );
            auto y          = std::sin( theta ) * std::sin( phi );
            auto z          = std::cos( theta );
            float* thePixel = getPixel( x, y, z );
            updateCoeffs( thePixel, -x, y, z, std::sin( theta ) * dtheta * dphi );
        }
    }
    tomatrix();
}

void EnvironmentTexture::updateCoeffs( float* hdr, float x, float y, float z, float domega ) {
    /******************************************************************
       Update the coefficients (i.e. compute the next term in the
       integral) based on the lighting value hdr[3], the differential
       solid angle domega and cartesian components of surface normal x,y,z

       Inputs:  hdr = L(x,y,z) [note that x^2+y^2+z^2 = 1]
                i.e. the illumination at position (x,y,z)

                domega = The solid angle at the pixel corresponding to
                (x,y,z).  For these light probes, this is given by

                x,y,z  = Cartesian components of surface normal

       Notes:   Of course, there are better numerical methods to do
                integration, but this naive approach is sufficient for our
                purpose.
    *********************************************************************/
    for ( int col = 0; col < 3; col++ ) {
        float c; /* A different constant for each coefficient */

        /* L_{00}.  Note that Y_{00} = 0.282095 */
        c = 0.282095f;
        m_shcoefs[0][col] += hdr[col] * c * domega;

        /* L_{1m}. -1 <= m <= 1.  The linear terms */
        c = 0.488603f;
        m_shcoefs[1][col] += hdr[col] * ( c * y ) * domega; /* Y_{1-1} = 0.488603 y  */
        m_shcoefs[2][col] += hdr[col] * ( c * z ) * domega; /* Y_{10}  = 0.488603 z  */
        m_shcoefs[3][col] += hdr[col] * ( c * x ) * domega; /* Y_{11}  = 0.488603 x  */

        /* The Quadratic terms, L_{2m} -2 <= m <= 2 */

        /* First, L_{2-2}, L_{2-1}, L_{21} corresponding to xy,yz,xz */
        c = 1.092548f;
        m_shcoefs[4][col] += hdr[col] * ( c * x * y ) * domega; /* Y_{2-2} = 1.092548 xy */
        m_shcoefs[5][col] += hdr[col] * ( c * y * z ) * domega; /* Y_{2-1} = 1.092548 yz */
        m_shcoefs[7][col] += hdr[col] * ( c * x * z ) * domega; /* Y_{21}  = 1.092548 xz */

        /* L_{20}.  Note that Y_{20} = 0.315392 (3z^2 - 1) */
        c = 0.315392f;
        m_shcoefs[6][col] += hdr[col] * ( c * ( 3 * z * z - 1 ) ) * domega;

        /* L_{22}.  Note that Y_{22} = 0.546274 (x^2 - y^2) */
        c = 0.546274f;
        m_shcoefs[8][col] += hdr[col] * ( c * ( x * x - y * y ) ) * domega;
    }
}

void EnvironmentTexture::tomatrix( void ) {
    /* Form the quadratic form matrix (see equations 11 and 12 in paper) */
    int col;
    float c1, c2, c3, c4, c5;
    c1 = 0.429043;
    c2 = 0.511664;
    c3 = 0.743125;
    c4 = 0.886227;
    c5 = 0.247708;

    for ( col = 0; col < 3; col++ ) { /* Equation 12 */

        m_shMatrices[col]( 0, 0 ) = c1 * m_shcoefs[8][col]; /* c1 L_{22}  */
        m_shMatrices[col]( 0, 1 ) = c1 * m_shcoefs[4][col]; /* c1 L_{2-2} */
        m_shMatrices[col]( 0, 2 ) = c1 * m_shcoefs[7][col]; /* c1 L_{21}  */
        m_shMatrices[col]( 0, 3 ) = c2 * m_shcoefs[3][col]; /* c2 L_{11}  */

        m_shMatrices[col]( 1, 0 ) = c1 * m_shcoefs[4][col];  /* c1 L_{2-2} */
        m_shMatrices[col]( 1, 1 ) = -c1 * m_shcoefs[8][col]; /*-c1 L_{22}  */
        m_shMatrices[col]( 1, 2 ) = c1 * m_shcoefs[5][col];  /* c1 L_{2-1} */
        m_shMatrices[col]( 1, 3 ) = c2 * m_shcoefs[1][col];  /* c2 L_{1-1} */

        m_shMatrices[col]( 2, 0 ) = c1 * m_shcoefs[7][col]; /* c1 L_{21}  */
        m_shMatrices[col]( 2, 1 ) = c1 * m_shcoefs[5][col]; /* c1 L_{2-1} */
        m_shMatrices[col]( 2, 2 ) = c3 * m_shcoefs[6][col]; /* c3 L_{20}  */
        m_shMatrices[col]( 2, 3 ) = c2 * m_shcoefs[2][col]; /* c2 L_{10}  */

        m_shMatrices[col]( 3, 0 ) = c2 * m_shcoefs[3][col]; /* c2 L_{11}  */
        m_shMatrices[col]( 3, 1 ) = c2 * m_shcoefs[1][col]; /* c2 L_{1-1} */
        m_shMatrices[col]( 3, 2 ) = c2 * m_shcoefs[2][col]; /* c2 L_{10}  */
        m_shMatrices[col]( 3, 3 ) = c4 * m_shcoefs[0][col] - c5 * m_shcoefs[6][col];
        /* c4 L_{00} - c5 L_{20} */
    }
}

float* EnvironmentTexture::getPixel( float x, float y, float z ) {
    auto ma  = std::abs( x );
    int axis = ( x > 0 );
    auto tc  = axis ? z : -z;
    auto sc  = y;
    if ( std::abs( y ) > ma ) {
        ma   = std::abs( y );
        axis = 2 + ( y < 0 );
        tc   = -x;
        sc   = ( axis == 2 ) ? -z : z;
    }
    if ( std::abs( z ) > ma ) {
        ma   = std::abs( z );
        axis = 4 + ( z < 0 );
        tc   = ( axis == 4 ) ? -x : x;
        sc   = y;
    }
    auto s = 0.5f * ( 1.f + sc / ma );
    auto t = 0.5f * ( 1.f + tc / ma );
    int is = int( s * m_width );
    int it = int( t * m_height );
    return &( m_skyData[axis][4 * ( ( m_height - 1 - is ) * m_width + it )] );
}

Ra::Engine::Data::Texture* EnvironmentTexture::getSHImage() {
    if ( m_shtexture != nullptr ) { return m_shtexture.get(); }

    size_t ambientWidth = 1024;
    auto thepixels      = new unsigned char[4 * ambientWidth * ambientWidth];
    for ( size_t i = 0; i < ambientWidth; i++ ) {
        for ( size_t j = 0; j < ambientWidth; j++ ) {

            /* We now find the cartesian components for the point (i,j) */
            float u, v, r;

            v = ( ambientWidth / 2.0 - j ) / ( ambientWidth / 2.0 ); /* v ranges from -1 to 1 */
            u = ( ambientWidth / 2.0 - i ) / ( ambientWidth / 2.0 ); /* u ranges from -1 to 1 */
            r = sqrt( u * u + v * v );                               /* The "radius" */
            if ( r > 1.0f ) {
                thepixels[4 * ( j * ambientWidth + i ) + 0] = 0;
                thepixels[4 * ( j * ambientWidth + i ) + 1] = 0;
                thepixels[4 * ( j * ambientWidth + i ) + 2] = 0;
                thepixels[4 * ( j * ambientWidth + i ) + 3] = 255;
                continue; /* Consider only circle with r<1 */
            }

            float theta = M_PI * r;      /* theta parameter of (i,j) */
            float phi   = atan2( v, u ); /* phi parameter */

            float x = std::sin( theta ) * std::cos( phi ); /* Cartesian components */
            float y = std::sin( theta ) * std::sin( phi );
            float z = std::cos( theta );

            // color = NtMN
            Ra::Core::Utils::Color color;
            Ra::Core::Vector4 normal( x, y, z, 1.f );

            Ra::Core::Vector4 MN;
            MN         = m_shMatrices[0] * normal;
            color( 0 ) = normal.dot( MN );
            MN         = m_shMatrices[1] * normal;
            color( 1 ) = normal.dot( MN );
            MN         = m_shMatrices[2] * normal;
            color( 2 ) = normal.dot( MN );

            color        = color * 0.05f;
            color        = Ra::Core::Utils::Color::linearRGBTosRGB( color );
            auto clpfnct = []( Scalar x ) {
                if ( x < 0 ) { x = 0; }
                if ( x > 1 ) { x = 1; }
                return x;
            };

            color.unaryExpr( clpfnct );
            thepixels[4 * ( j * ambientWidth + i ) + 0] =
                static_cast<unsigned char>( color[0] * 255 );
            thepixels[4 * ( j * ambientWidth + i ) + 1] =
                static_cast<unsigned char>( color[1] * 255 );
            thepixels[4 * ( j * ambientWidth + i ) + 2] =
                static_cast<unsigned char>( color[2] * 255 );
            thepixels[4 * ( j * ambientWidth + i ) + 3] = 255;
        }
    }
    Ra::Engine::Data::TextureParameters params { "shImage",
                                                 GL_TEXTURE_2D,
                                                 ambientWidth,
                                                 ambientWidth,
                                                 1,
                                                 GL_RGBA,
                                                 GL_RGBA,
                                                 GL_UNSIGNED_BYTE,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_CLAMP_TO_EDGE,
                                                 GL_LINEAR,
                                                 GL_LINEAR,
                                                 thepixels };
    m_shtexture = std::make_unique<Ra::Engine::Data::Texture>( params );
    return m_shtexture.get();
}

void EnvironmentTexture::saveShProjection( const std::string filename ) {
    getSHImage();
    auto flnm = std::string( "../" ) + filename;
    stbi_write_png(
        flnm.c_str(), m_shtexture->width(), m_shtexture->height(), 4, m_shtexture->texels(), 0 );
}

Ra::Core::Matrix4 EnvironmentTexture::getShMatrix( int channel ) {
    return m_shMatrices[channel];
}

void EnvironmentTexture::updateGL() {
    if ( !m_glReady ) {
        // load the skybox shader
        auto shaderMngr = Ra::Engine::RadiumEngine::getInstance()->getShaderProgramManager();
        const std::string vertexShaderSource { "#include \"TransformStructs.glsl\"\n"
                                               "layout (location = 0) in vec3 in_position;\n"
                                               "out vec3 var_texcoord;\n"
                                               "uniform Transform transform;\n"
                                               "void main(void)\n"
                                               "{\n"
                                               "    mat4 mvp = transform.proj * transform.view;\n"
                                               "    gl_Position = mvp*vec4(in_position.xyz, 1.0);\n"
                                               "    var_texcoord = in_position;\n"
                                               "}\n" };
        const std::string fragmentShadersource {
            "layout (location = 0) out vec4 out_color;\n"
            "in vec3 var_texcoord;\n"
            "uniform samplerCube skytexture;\n"
            "uniform float strength;\n"
            "void main(void)\n"
            "{\n"
            "    vec3 envColor = texture(skytexture, normalize(var_texcoord)).rgb;\n"
            "    out_color =vec4(strength*envColor, 1);\n"
            "}\n" };
        Ra::Engine::Data::ShaderConfiguration config { "Built In SkyBox" };
        config.addShaderSource( Ra::Engine::Data::ShaderType::ShaderType_VERTEX,
                                vertexShaderSource );
        config.addShaderSource( Ra::Engine::Data::ShaderType::ShaderType_FRAGMENT,
                                fragmentShadersource );
        auto added = shaderMngr->addShaderProgram( config );
        if ( added ) { m_skyShader = added.value(); }
        else {
            // Unable to load the shader ... deactivate skybox
            m_isSkyBox = false;
        }
        m_displayMesh->updateGL();
        m_skyTexture->initializeGL( true );
        glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
        m_glReady = true;
        // saveShProjection( "SHImage.png" );
    }
}

void EnvironmentTexture::render( const Ra::Engine::Data::ViewingParameters& viewParams ) {
    if ( m_isSkyBox ) {
        // put this in a initializeGL method ?
        if ( !m_glReady ) { updateGL(); }
        auto skyparams      = viewParams;
        Ra::Core::Matrix3 t = Ra::Core::Transform { skyparams.viewMatrix }.linear();
        skyparams.viewMatrix.setIdentity();
        skyparams.viewMatrix.topLeftCorner<3, 3>() = t;
        auto cameraManager                         = static_cast<Ra::Engine::Scene::CameraManager*>(
            Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
        auto cam = cameraManager->getActiveCamera();
        Scalar fov =
            std::clamp( cam->getZoomFactor() * cam->getFOV(), 0.001_ra, Math::Pi - 0.1_ra );
        skyparams.projMatrix =
            Ra::Core::Asset::Camera::perspective( cam->getAspect(), fov, 0.1, 3. );
        m_skyShader->bind();
        m_skyShader->setUniform( "transform.proj", skyparams.projMatrix );
        m_skyShader->setUniform( "transform.view", skyparams.viewMatrix );
        m_skyTexture->bind( 0 );
        m_skyShader->setUniform( "skytexture", 0 );
        m_skyShader->setUniform( "strength", m_environmentStrength );
        GLboolean depthEnabled;
        glGetBooleanv( GL_DEPTH_WRITEMASK, &depthEnabled );
        glDepthMask( GL_FALSE );
        m_displayMesh->render( m_skyShader );
        glDepthMask( depthEnabled );
    }
}

void EnvironmentTexture::setStrength( float s ) {
    m_environmentStrength = s;
}

float EnvironmentTexture::getStrength() const {
    return m_environmentStrength;
}

Ra::Engine::Data::Texture* EnvironmentTexture::getEnvironmentTexture() {
    return m_skyTexture.get();
}

} // namespace Data
} // namespace Engine
} // namespace Ra
