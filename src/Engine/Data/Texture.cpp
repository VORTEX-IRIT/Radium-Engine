
#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Utils/Log.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/RadiumEngine.hpp>

#include <glbinding/gl/enum.h>
#include <globjects/Texture.h>

#include <cmath>

namespace Ra {
namespace Engine {
namespace Data {
using namespace Core::Utils; // log

Texture::Texture( const TextureParameters& texParameters ) :
    m_textureParameters { texParameters }, m_texture { nullptr }, m_isMipMapped { false } {}

class DeleteTextureTask : public Core::Task
{
  public:
    explicit DeleteTextureTask( std::unique_ptr<globjects::Texture> texture ) :
        m_texture( std::move( texture ) ) {}

    /// Return the name of the task.
    std::string getName() const override { return "DeleteTextureTask"; };

    /// Do the task job. Will be called from the task queue threads.
    virtual void process() override {
        m_texture->detach();
        m_texture.reset();
    };

  private:
    std::unique_ptr<globjects::Texture> m_texture;
};

Texture::~Texture() {
    if ( m_updateImageTaskId.isValid() ) {
        RadiumEngine::getInstance()->removeGpuTask( m_updateImageTaskId );
    }

    if ( m_updateSamplerTaskId.isValid() ) {
        RadiumEngine::getInstance()->removeGpuTask( m_updateSamplerTaskId );
    }

    // register delayed destroy gpu texture task
    destroy();
}

void Texture::initialize() {
    if ( !isSupportedTarget() ) return;
    // Transform texels if needed

    computeIsMipMappedFlag();

    // Update the sampler parameters
    registerUpdateSamplerParametersTask();

    // upload texture to the GPU
    registerUpdateImageDataTask();
}

void Texture::initializeNow() {
    if ( !isSupportedTarget() ) return;
    computeIsMipMappedFlag();

    createTexture();
    sendSamplerParametersToGpu();
    sendImageDataToGpu();
}

void Texture::destroy() {
    if ( m_texture ) {
        // if engine is still available
        if ( auto engine = RadiumEngine::getInstance() ) {
            auto task = std::make_unique<DeleteTextureTask>( std::move( m_texture ) );
            engine->addGpuTask( std::move( task ) );
        }
        // else gpu representation will not be cleaned by the application.
        m_texture.reset();
    }
}

void Texture::destroyNow() {
    m_texture->detach();
    m_texture.reset();
}

void Texture::updateData( std::shared_ptr<void> newData ) {
    // register gpu task to update opengl representation before next rendering
    std::lock_guard<std::mutex> lock( m_updateMutex );
    m_textureParameters.image.texels = newData;
    registerUpdateImageDataTask();
}

void Texture::resize( size_t w, size_t h, size_t d, std::shared_ptr<void> pix ) {
    m_textureParameters.image.width  = w;
    m_textureParameters.image.height = h;
    m_textureParameters.image.depth  = d;
    m_textureParameters.image.texels = pix;
    if ( createTexture() ) {
        computeIsMipMappedFlag();
        sendSamplerParametersToGpu();
        sendImageDataToGpu();
    }
    else { sendImageDataToGpu(); }
}

void Texture::setParameters( const TextureParameters& textureParameters ) {
    setSamplerParameters( textureParameters.sampler );
    setImageParameters( textureParameters.image );
}

void Texture::setImageParameters( const ImageParameters& imageParameters ) {
    std::lock_guard<std::mutex> lock( m_updateMutex );
    m_textureParameters.image = imageParameters;
    registerUpdateImageDataTask();
}

void Texture::setSamplerParameters( const SamplerParameters& samplerParameters ) {
    std::lock_guard<std::mutex> lock( m_updateMutex );
    m_textureParameters.sampler = samplerParameters;
    registerUpdateSamplerParametersTask();
}

void Texture::bind( int unit ) {
    if ( unit >= 0 ) { m_texture->bindActive( uint( unit ) ); }
    else { m_texture->bind(); }
}

void Texture::bindImageTexture( int unit,
                                GLint level,
                                GLboolean layered,
                                GLint layer,
                                GLenum access ) {
    m_texture->bindImageTexture(
        uint( unit ), level, layered, layer, access, m_textureParameters.image.internalFormat );
}

void Texture::linearize( ImageParameters& image ) {
    if ( !image.isLinear ) {
        // Only RGB and RGBA texture contains color information
        // (others are not really colors and must be managed explicitly by the user)
        uint numComp  = 0;
        bool hasAlpha = false;
        switch ( image.format ) {
            // RED texture store a gray scale color. Verify if we need to convert
        case GL_RED:
            numComp = 1;
            break;
        case GL_RGB:
            numComp = 3;
            break;
        case GL_RGBA:
            numComp  = 4;
            hasAlpha = true;
            break;
        default:
            LOG( logERROR ) << "Textures with format " << image.format << " can't be linearized.\n";
            return;
        }
        if ( image.type == GL_TEXTURE_CUBE_MAP ) { linearizeCubeMap( image, numComp, hasAlpha ); }
        else {
            srgbToLinearRgb( reinterpret_cast<uint8_t*>( std::get<0>( image.texels ).get() ),
                             image.width,
                             image.height,
                             image.depth,
                             numComp,
                             hasAlpha );
        }
        image.isLinear = true;
    }
}

bool Texture::isSupportedTarget() {
    if ( ( m_textureParameters.image.target != GL_TEXTURE_1D ) &&
         ( m_textureParameters.image.target != GL_TEXTURE_2D ) &&
         ( m_textureParameters.image.target != GL_TEXTURE_RECTANGLE ) &&
         ( m_textureParameters.image.target != GL_TEXTURE_3D ) &&
         ( m_textureParameters.image.target != GL_TEXTURE_CUBE_MAP ) ) {
        LOG( logERROR ) << "Texture of type " << m_textureParameters.image.target
                        << " must be generated explicitly!";
        return false;
    }
    return true;
}

void Texture::computeIsMipMappedFlag() {
    m_isMipMapped = !( m_textureParameters.sampler.minFilter == GL_NEAREST ||
                       m_textureParameters.sampler.minFilter == GL_LINEAR );
}

bool Texture::createTexture() {
    if ( m_texture == nullptr ) {
        m_texture = globjects::Texture::create( m_textureParameters.image.target );
        GL_CHECK_ERROR;
        return true;
    }
    return false;
}

void Texture::registerUpdateImageDataTask() {
    // register gpu task to update opengl representation before next rendering
    if ( m_updateImageTaskId.isInvalid() ) {
        auto taskFunc = [this]() {
            std::lock_guard<std::mutex> taskLock( m_updateMutex );
            // Generate OpenGL texture
            this->createTexture();
            this->sendImageDataToGpu();
            m_updateImageTaskId = Core::TaskQueue::TaskId::Invalid();
        };
        auto task           = std::make_unique<Core::FunctionTask>( taskFunc, getName() );
        m_updateImageTaskId = RadiumEngine::getInstance()->addGpuTask( std::move( task ) );
    }
}

void Texture::registerUpdateSamplerParametersTask() {
    if ( m_updateSamplerTaskId.isInvalid() ) {
        auto taskFunc = [this]() {
            std::lock_guard<std::mutex> taskLock( m_updateMutex );
            // Generate OpenGL texture
            this->createTexture();
            this->sendSamplerParametersToGpu();
            m_updateSamplerTaskId = Core::TaskQueue::TaskId::Invalid();
        };
        auto task             = std::make_unique<Core::FunctionTask>( taskFunc, getName() );
        m_updateSamplerTaskId = RadiumEngine::getInstance()->addGpuTask( std::move( task ) );
    }
}

void Texture::sendImageDataToGpu() {
    CORE_ASSERT( m_texture != nullptr, "Cannot update non initialized texture" );
    switch ( m_texture->target() ) {
    case GL_TEXTURE_1D: {
        m_texture->image1D( 0,
                            m_textureParameters.image.internalFormat,
                            GLsizei( m_textureParameters.image.width ),
                            0,
                            m_textureParameters.image.format,
                            m_textureParameters.image.type,
                            getTexels() );
        GL_CHECK_ERROR
    } break;
    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE: {
        m_texture->image2D( 0,
                            m_textureParameters.image.internalFormat,
                            GLsizei( m_textureParameters.image.width ),
                            GLsizei( m_textureParameters.image.height ),
                            0,
                            m_textureParameters.image.format,
                            m_textureParameters.image.type,
                            getTexels() );
        GL_CHECK_ERROR
    } break;
    case GL_TEXTURE_3D: {
        m_texture->image3D( 0,
                            m_textureParameters.image.internalFormat,
                            GLsizei( m_textureParameters.image.width ),
                            GLsizei( m_textureParameters.image.height ),
                            GLsizei( m_textureParameters.image.depth ),
                            0,
                            m_textureParameters.image.format,
                            m_textureParameters.image.type,
                            getTexels() );
        GL_CHECK_ERROR
    } break;
    case GL_TEXTURE_CUBE_MAP: {
        // Load the 6 faces of the cube-map
        auto cubeMap = m_textureParameters.image.getCubeMap();

        std::array<const gl::GLvoid*, 6> data;
        std::transform( std::begin( cubeMap ),
                        std::end( cubeMap ),
                        std::begin( data ),
                        []( const std::shared_ptr<void>& val ) { return val.get(); } );

        m_texture->cubeMapImage( 0,
                                 m_textureParameters.image.internalFormat,
                                 GLsizei( m_textureParameters.image.width ),
                                 GLsizei( m_textureParameters.image.height ),
                                 0,
                                 m_textureParameters.image.format,
                                 m_textureParameters.image.type,
                                 data );

        GL_CHECK_ERROR
    } break;
    default: {
        CORE_ASSERT( 0, "Unsupported texture type ?" );
    } break;
    }
    // Generate mip-map if needed.
    if ( m_isMipMapped ) { m_texture->generateMipmap(); }

    GL_CHECK_ERROR;
}

void Texture::readFromGpu( int level ) {
    CORE_ASSERT( m_texture != nullptr, "Cannot get non initialized texture" );
    CORE_ASSERT( m_textureParameters.image.isTexelOfType<ImageParameters::ImageType>(),
                 "Can only get image typf" );
    CORE_ASSERT( m_textureParameters.image.getTexels() != nullptr, "Can only get image type" );
    CORE_ASSERT( GL_TEXTURE_CUBE_MAP != m_texture->target(), "Cannot get cube map" );

    m_texture->getImage( level,
                         m_textureParameters.image.format,
                         m_textureParameters.image.type,
                         m_textureParameters.image.getImage().get() );
}

// let the compiler warn about case fallthrough
void Texture::sendSamplerParametersToGpu() {
    switch ( m_texture->target() ) {
    case GL_TEXTURE_CUBE_MAP:
    case GL_TEXTURE_3D:
        m_texture->setParameter( GL_TEXTURE_WRAP_R, m_textureParameters.sampler.wrapR );
        GL_CHECK_ERROR;
        [[fallthrough]];
    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE:
        m_texture->setParameter( GL_TEXTURE_WRAP_T, m_textureParameters.sampler.wrapT );
        GL_CHECK_ERROR;
        [[fallthrough]];
    case GL_TEXTURE_1D:
        m_texture->setParameter( GL_TEXTURE_WRAP_S, m_textureParameters.sampler.wrapS );
        GL_CHECK_ERROR;
        break;
    default:
        break;
    }
    m_texture->setParameter( GL_TEXTURE_MIN_FILTER, m_textureParameters.sampler.minFilter );
    GL_CHECK_ERROR;
    m_texture->setParameter( GL_TEXTURE_MAG_FILTER, m_textureParameters.sampler.magFilter );
    GL_CHECK_ERROR;
}

/// \todo template by texels type
void Texture::srgbToLinearRgb( uint8_t* texels,
                               uint width,
                               uint height,
                               uint depth,
                               uint numComponent,
                               bool hasAlphaChannel ) {
    // auto linearize = [gamma](float in)-> float {
    auto linearize = []( uint8_t in ) -> uint8_t {
        // Constants are described at https://en.wikipedia.org/wiki/SRGB
        float c = float( in ) / 255;
        if ( c < 0.04045 ) { c = c / 12.92f; }
        else { c = std::pow( ( ( c + 0.055f ) / ( 1.055f ) ), 2.4f ); }
        return uint8_t( c * 255 );
    };
    uint numValues = hasAlphaChannel ? numComponent - 1 : numComponent;
#pragma omp parallel for
    for ( int i = 0; i < int( width * height * depth ); ++i ) {
        // Convert each R or RGB value while keeping alpha unchanged
        for ( uint p = i * numComponent; p < i * numComponent + numValues; ++p ) {
            texels[p] = linearize( texels[p] );
        }
    }
}

void Texture::linearizeCubeMap( ImageParameters& image, uint numComponent, bool hasAlphaChannel ) {
    if ( image.type == gl::GLenum::GL_UNSIGNED_BYTE ) {
        /// Only unsigned byte texture could be linearized. Considering other formats where
        /// already linear
        const auto cubeMap = image.getCubeMap();
        for ( int i = 0; i < 6; ++i ) {
            srgbToLinearRgb( reinterpret_cast<uint8_t*>( cubeMap[i].get() ),
                             image.width,
                             image.height,
                             image.depth,
                             numComponent,
                             hasAlphaChannel );
        }
    }
}

} // namespace Data
} // namespace Engine
} // namespace Ra
