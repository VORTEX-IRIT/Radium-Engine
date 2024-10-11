#pragma once

#include <Engine/Data/TextureManager.hpp>
#include <Engine/RadiumEngine.hpp>

namespace Ra {
namespace Engine {
namespace Data {

/** \brief Base class to manage a set of textures indexed by semantic (enum).
 */
template <typename TextureSemantic>
class MaterialTextureSet
{
  public:
    virtual ~MaterialTextureSet() = default;

    void addTexture( const TextureSemantic& semantic,
                     const TextureManager::TextureHandle& texture ) {
        m_textures[semantic] = texture;
    }

    /** \brief Add texture to TextureManager fisrt, then to the texture set.
     */
    void addTexture( const TextureSemantic& semantic, const TextureParameters& texture ) {
        auto texManager = RadiumEngine::getInstance()->getTextureManager();
        addTexture( semantic, texManager->addTexture( texture ) );
    }

    /** \brief Texture getter from semantic.
     *
     * \return raw (non owning) ptr to the texture, `nullptr` if the texture is not found.
     */
    Texture* getTexture( const TextureSemantic& semantic ) const {
        Texture* tex    = nullptr;
        auto texManager = RadiumEngine::getInstance()->getTextureManager();
        auto it         = m_textures.find( semantic );
        if ( it != m_textures.end() ) { tex = texManager->getTexture( it->second ); }
        return tex;
    }

  private:
    std::map<TextureSemantic, TextureManager::TextureHandle> m_textures;
};
} // namespace Data
} // namespace Engine
} // namespace Ra
