#pragma once

#include <Engine/RaEngine.hpp>
#include <map>
#include <string>

#include <Engine/Data/Texture.hpp>
#include <Engine/OpenGL.hpp>
namespace Ra {
namespace Engine {
namespace Data {

/** \brief Manage Texture loading and registration.
 *
 * TextureManager allows to store texture globally such that different object can share textures
 * based on their handle or name.
 * Name uniqueness is not guaranteed by the manager, take care in case of search texture by name.
 */
class RA_ENGINE_API TextureManager final
{

  private:
  public:
    using TextureHandle = Ra::Core::Utils::Index;

    /** \brief Add a texture given its TextureParameters
     *
     * \return a TextureHandle that uniquely identify the texture in the manager.
     */
    TextureHandle addTexture( const TextureParameters& p );

    /** \brief Get raw texture ptr from handle.
     *
     * The texture is still managed by the manager. Do not delete the return ptr.
     */
    Texture* getTexture( const TextureHandle& handle );

    /** \brief Get a texture handle from textue name.
     *
     * It search for the first texture in the set of managed texture that have this name.
     * \return An handle to the texture or InvalidIndex if no texture with this name is found
     */
    TextureHandle getTextureHandle( const std::string& name );

    /** \brief Convinience function to obtain Texture ptr from name. */
    Texture* getTexture( const std::string& name ) {
        return getTexture( getTextureHandle( name ) );
    }

    /** \brief Remove a managed texture identified by handle.
     *
     * Texture dtor will register the delete texture gpu task.
     */
    void deleteTexture( const TextureHandle& handle );

    /** \brief Load \a filename and fill ImageParameters according to \a filename content.
     *
     * \note only loads 2D image file for now.
     * \param filename fully qualified image filename. See stbi_load_image for supported file
     * format.
     * \return ImageParameters corresponding to filename content, image.parameters.texels contains
     * actual image data. In case of loading error, texels is nullptr, width = height = 0.
     */
    static ImageParameters loadTextureImage( const std::string& filename, bool linearize = false );

  public:
    TextureManager();
    ~TextureManager();
    TextureManager( TextureManager const& )            = delete;
    TextureManager( TextureManager&& )                 = delete;
    TextureManager& operator=( TextureManager const& ) = delete;
    TextureManager& operator=( TextureManager&& )      = delete;

  private:
    std::vector<std::unique_ptr<Texture>> m_textures;
};

} // namespace Data
} // namespace Engine
} // namespace Ra
