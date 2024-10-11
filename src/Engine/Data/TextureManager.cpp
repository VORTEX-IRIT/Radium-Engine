#include <Engine/Data/Texture.hpp>
#include <Engine/Data/TextureManager.hpp>

#include <Core/Utils/Log.hpp>

#include <globjects/AbstractUniform.h>
#include <stb/stb_image.h>

#include <memory>

namespace Ra {
namespace Engine {
namespace Data {

using namespace Core::Utils; // log

TextureManager::TextureManager() = default;

TextureManager::~TextureManager() = default;

TextureManager::TextureHandle TextureManager::addTexture( const TextureParameters& parameters ) {

    auto texture = std::make_unique<Texture>( parameters );
    TextureHandle handle;
    // find first free slot in m_textures, e.g. where the stored unique_ptr is nullptr
    auto it = std::find_if(
        m_textures.begin(), m_textures.end(), []( const auto& texture_ ) { return !texture_; } );
    if ( it != m_textures.end() ) {
        it->swap( texture );
        handle.setValue( std::distance( m_textures.begin(), it ) );
    }
    else {
        // if no free slot, push back a new texture
        m_textures.push_back( std::move( texture ) );
        handle.setValue( m_textures.size() - 1 );
    }
    m_textures[handle.getValue()]->initialize();
    return handle;
}

ImageParameters TextureManager::loadTextureImage( const std::string& filename, bool linearize ) {
    stbi_set_flip_vertically_on_load( true );
    int n;
    ImageParameters image;
    int width;
    int height;
    unsigned char* data = stbi_load( filename.c_str(), &width, &height, &n, 0 );
    image.width         = width;
    image.height        = height;
    if ( !data ) {
        LOG( logERROR ) << "Something went wrong when loading image \"" << filename << "\".";
        image.width = image.height = 0;
        return image;
    }

    switch ( n ) {
    case 1: {
        image.format         = GL_RED;
        image.internalFormat = GL_R8;
    } break;

    case 2: {
        // suppose it is GL_LUMINANCE_ALPHA
        image.format         = GL_RG;
        image.internalFormat = GL_RG8;
    } break;

    case 3: {
        image.format         = GL_RGB;
        image.internalFormat = GL_RGB8;
    } break;

    case 4: {
        image.format         = GL_RGBA;
        image.internalFormat = GL_RGBA8;
    } break;
    default: {
        image.format         = GL_RGBA;
        image.internalFormat = GL_RGBA8;
    } break;
    }

    CORE_ASSERT( data, "Data is null" );
    // make a shared ptr, with deleter from stb
    image.texels = std::shared_ptr<void>( data, stbi_image_free );
    image.type   = GL_UNSIGNED_BYTE;
    if ( linearize ) Texture::linearize( image );
    return image;
}

Texture* TextureManager::getTexture( const TextureHandle& handle ) {
    return handle.isValid() ? m_textures[handle.getValue()].get() : nullptr;
}

TextureManager::TextureHandle TextureManager::getTextureHandle( const std::string& name ) {
    auto it = std::find_if( m_textures.begin(), m_textures.end(), [name]( const auto& texture ) {
        return texture->getName() == name;
    } );

    return it != m_textures.end() ? TextureHandle { std::distance( m_textures.begin(), it ) }
                                  : TextureHandle::Invalid();
}

void TextureManager::deleteTexture( const TextureHandle& handle ) {
    if ( handle.isValid() ) m_textures[handle.getValue()].reset( nullptr );
}

} // namespace Data
} // namespace Engine
} // namespace Ra
