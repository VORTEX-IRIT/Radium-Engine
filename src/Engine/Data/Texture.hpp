#pragma once

#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Utils/Color.hpp>
#include <Engine/OpenGL.hpp>
#include <Engine/RaEngine.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <variant>

namespace globjects {
class Texture;
}

namespace Ra {
namespace Engine {
namespace Data {

/// GPU Sampler configuration
struct SamplerParameters {
    /// OpenGL wrap mode in the s direction
    GLenum wrapS { GL_CLAMP_TO_EDGE };
    /// OpenGL wrap mode in the t direction
    GLenum wrapT { GL_CLAMP_TO_EDGE };
    /// OpenGL wrap mode in the r direction
    GLenum wrapR { GL_CLAMP_TO_EDGE };
    /// OpenGL minification filter ( GL_LINEAR or GL_NEAREST or GL_XXX_MIPMAP_YYY )
    GLenum minFilter { GL_LINEAR };
    /// OpenGL magnification filter ( GL_LINEAR or GL_NEAREST )
    GLenum magFilter { GL_LINEAR };
};

/// ImageParameters stores the infomation needed to upload a image to the GPU as a texture.
/// The actual data is stored as a std::variant, so this image parameter can store either a single
/// image or a cube map as an array of 6 images. The stored shared_ptr is void, and must correspond
/// to the format described by the other ImageParameters data member.
/// Its perfectly fine to have nullptr texels to represent GPU only texture.
struct ImageParameters {
    /// Types for texels variant
    using ImageType   = std::shared_ptr<void>;
    using CubeMapType = std::array<std::shared_ptr<void>, 6>;

    /// check which type is held by texels
    template <typename TexelType>
    bool isTexelOfType() const {
        return std::holds_alternative<TexelType>( texels );
    }

    ImageType getImage() const {
        CORE_ASSERT( isTexelOfType<ImageType>(), "texture variant is not ImageType" );
        return std::get<ImageType>( texels );
    }
    void setImage( ImageType image ) { texels = image; }

    /// get texels raw pointer
    /// throws std::bad_variant_access if !isTexelOfType<ImageType>()
    /// see std::get
    const void* getTexels() const { return getImage().get(); }

    /// get cube map array of shared ptr
    /// throws std::bad_variant_access if !isTexelOfType<CubeMapType>()
    /// see std::get
    const CubeMapType getCubeMap() const {
        CORE_ASSERT( isTexelOfType<CubeMapType>(), "texture variant is not CubeMapType" );
        return std::get<CubeMapType>( texels );
    }
    void setCubeMap( CubeMapType cubeMap ) { texels = cubeMap; }

    GLenum target { GL_TEXTURE_2D }; //< OpenGL target
    size_t width { 1 };              //< width of the texture (s dimension)
    size_t height { 1 };             //< height of the texture (t dimension)
    size_t depth { 1 };              //< depth of the texture (r dimension)
    GLenum format { GL_RGB };        //< Format of the external data
    /// OpenGL internal format (WARNING, for Integer textures, must be GL_XXX_INTEGER)
    GLenum internalFormat { GL_RGB };
    GLenum type { GL_UNSIGNED_BYTE }; //< Type of the components in external data
    /// set to true when linearize texture rgb component. If true, linearize has no effect.
    bool isLinear { false };
    std::variant<ImageType, CubeMapType> texels { nullptr }; //< texels OR cubeMap, shared ownership
};

/** \brief Describes the sampler and image of a texture.
 *
 * These parameters describe the image data of the texture :
 *    - ImageParameters: target, width, height, depth, format, internalFormat, type and texels for
 * describing image data
 *    - SampleParameters: wrapS, wrapT, wrapP, minFilter and magFilter for describing the sampler of
 * the texture.
 *
 * MipMap representation of the texture is automatically generated as soon as the minFilter
 * parameter is something else than GL_LINEAR or GL_NEAREST.
 * \todo allow to use another mipmap building function than glGenerateMipmap
 *
 * \note No coherence checking will be done on the content of this structure. User must ensure
 * coherent data and parameters.
 */
struct TextureParameters {
    std::string name {};
    SamplerParameters sampler {};
    ImageParameters image {};
};

/** \brief Represent a Texture of the engine.
 *
 *  When one wants to create a texture, the first thing to do is to create and fill a
 * TextureParameters to describe the Texture.
 *
 *  The Texture creation could be done either using the TextureManager or directly on the client
 * class/function.
 *
 *  When a texture is created, no GPU initialisation is realized. The user must first call
 * either initialize() to register delayed initialisation by RadiumEngine using
 * RadiumEngine::runGpuTasks() or intializeNow() to peform GPU initialization directly, with an
 * active bound context.
 * See TextureManager for information about how unique texture are defined.
 */
class RA_ENGINE_API Texture final
{
  public:
    /** \brief Textures are not copyable, delete copy constructor.
     */
    Texture( const Texture& ) = delete;

    /** \brief Textures are not copyable, delete operator =.
     */
    void operator=( const Texture& ) = delete;

    /** \brief Texture constructor. No GPU initialization is done there.
     *
     * \param texParameters Name of the texture
     */
    explicit Texture( const TextureParameters& texParameters );

    /** \brief Texture destructor. Both internal data and GPU representation are deleted.
     */
    ~Texture();

    /** \brief Generate the OpenGL representation of the texture according to the stored
     * TextureData (delayed).
     *
     * This method use the stored TextureParameters to generate and configure OpenGL
     * texture. It creates gpu tasks the engine will run during next draw call, so it can be called
     * without active opengl context.
     */
    void initialize();

    /** \brief Generate the GPU representation of the texture <b>right now</b>. Need an active
     * OpenGL context.
     *
     * see initialze() which is the same method, but delay gpu stuff to engine gpu tasks.
     */
    void initializeNow();

    /** \brief destroy the GPU texture representation.
     *
     * It creates gpu tasks the engine will run during next draw call, so it can be called
     * without active opengl context.
     */
    void destroy();

    /** \brief Generate the GPU representation of the texture <b>right now</b>. Need an active
     * OpenGL context.
     *
     * see destroy() which is the same method, but delay gpu stuff to engine gpu tasks.
     */
    void destroyNow();

    /** \return Name of the texture. */
    inline std::string getName() const { return m_textureParameters.name; }
    /** Sets the name of the texture. */
    inline void setName( const std::string& name ) { m_textureParameters.name = name; }

    /** \return the pixel format of the texture */
    GLenum getFormat() const { return m_textureParameters.image.format; }

    /** \return the width of the texture */
    size_t getWidth() const { return m_textureParameters.image.width; }

    /** \return the height of the texture */
    size_t getHeight() const { return m_textureParameters.image.height; }

    /** \return the depth of the texture */
    size_t getDepth() const { return m_textureParameters.image.depth; }

    /** \return raw pointer to texels (or nullptr if cubeMap or no cpu side representation). */
    const void* getTexels() { return m_textureParameters.image.getTexels(); }

    /** \brief Get ptr to the managed globjects::Texture.
     *
     * Please unsure do keep data consistent between Texture's TextureParameters and the
     * globjects::Texture and prefer using Textue methods to change texture representation if
     * avaiable.
     * \return ptr to globjects::Texture associated with the Texture.
     */
    globjects::Texture* getGpuTexture() const { return m_texture.get(); }

    /** \brief get read access to texture parameters */
    const TextureParameters& getParameters() const { return m_textureParameters; }
    /** \brief read/write access to texture parameters */
    TextureParameters& getParameters() { return m_textureParameters; }

    /** \brief Update the cpu representation of data contained by the texture.
     *
     * \a newData must contain the same number (of the same type) of elements than old data (not
  checked).
     * Element count can be obtained with getWidth() * getHeight()
     * Element type can be obtained with getFormat()
     * \param newData user image pointer to wrap
     */
    void updateData( std::shared_ptr<void> newData );

    /** \brief Resize the texture. Need active OpenGL context.
     *
     * This allocate GPU memory to store the new resized texture and, if texels are not nullptr,
     * upload the new content.
     * \note If texels are not nullptr, user must ensure the texels array is correctly
     * dimensioned.
     * \param w width of the texture
     * \param h height of the texture
     * \param d depth of the texture
     * \param pix the new texels array corresponding the the new texture dimension
     */
    void resize( size_t w = 1, size_t h = 1, size_t d = 1, std::shared_ptr<void> pix = nullptr );

    /** \brief set TextureParameters.
     *
     * Call setImageParameters() and setSamplerParameters() to
     * register update GPU sample task. No check is peformed to see if data need to be updated,
     * gpu update is triggered inconditionnally.
     */
    void setParameters( const TextureParameters& textureParameters );
    /** \brief set TextureParameters.image */
    void setImageParameters( const ImageParameters& imageParameters );
    /** \brief set TerctureParameters.samples */
    void setSamplerParameters( const SamplerParameters& samplerParameters );

    /** \brief Bind the texture to GPU texture \a unit to enable its use in a shader. Need
     * active OpenGL context.
     *
     * \param unit Index of the texture to be bound. If -1 (default) only calls glBindTexture.
     */
    void bind( int unit = -1 );

    /** \brief Bind the texture to an image unit for the purpose of reading and writing it from
     * shaders. Need active OpenGL context.
     *
     * \note, only available since openGL 4.2, not available on MacOs
     * uses m_parameters.internalFormat as format.
     * see
     * https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glBindImageTexture.xhtml
     * for documentation
     */
    void bindImageTexture( int unit, GLint level, GLboolean layered, GLint layer, GLenum access );

    /** \brief Convert a color texture from sRGB to Linear RGB spaces.
     *
     * This will transform the internal representation of the texture to GL_SCALAR (GL_FLOAT).
     * Only GL_RGB[8, 16, 16F, 32F] and GL_RGBA[8, 16, 16F, 32F] are managed.
     * Full transformation as described at https://en.wikipedia.org/wiki/SRGB
     */
    static void linearize( ImageParameters& image );

    /** \brief Regiter gpu task to RadiumEngine. Will call sendImageDataToGpu during next
     * RadiumEngine::runGpuTasks() call.
     */
    void registerUpdateImageDataTask();

    /** \brief Regiter gpu task to RadiumEngine. Will call sendSamplerParametersToGpu during
     * next RadiumEngine::runGpuTasks() call.
     */
    void registerUpdateSamplerParametersTask();

    /** \brief Send image data to the GPU and generate mipmap if needed */
    void sendImageDataToGpu();
    void readFromGpu( int level = 0 );

    /** \brief Send sampler parameters to the GPU */
    void sendSamplerParametersToGpu();

  private:
    /** \brief Check if the texture target is supported.
     *
     * Current implementation supports GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE,
     * GL_TEXTURE_3D and GL_TEXTURE_CUBE_MAP.
     * \return true if the target is supported by Texture implementation.
     */
    bool isSupportedTarget();

    /** \brief set m_isMipMapped according to sampler.minFilter */
    void computeIsMipMappedFlag();

    /** \brief Allocate gup texture representation (m_texture) if not already allocated
     * (nullptr).
     *
     * \return true if allocation is performed.
     */
    bool createTexture();

    /** \brief Convert a color texture from sRGB to Linear RGB spaces.
     *
     * The content of the array of texels.
     * designated by the texel pointer is modified by side effect.
     * Full transformation as described at https://en.wikipedia.org/wiki/SRGB
     * \param texels the array of texels to linearize
     * \param numComponent number of color channels.
     * \param bool hasAlphaChannel indicate if the last channel is an alpha channel.
     * \param gamma the gama value to use (sRGB is 2.4)
     * \note only 8 bit (GL_UNSIGNED_BYTE data format) textures are managed by this operator.
     */
    static void srgbToLinearRgb( uint8_t* texels,
                                 uint width,
                                 uint height,
                                 uint depth,
                                 uint numComponent,
                                 bool hasAlphaChannel );

    /** \brief Linearize a cube map by calling sRGBToLinearRGB fore each face */
    static void linearizeCubeMap( ImageParameters& image, uint numComponent, bool hasAlphaChannel );

    TextureParameters m_textureParameters;         /**< Texture parameters */
    std::unique_ptr<globjects::Texture> m_texture; /**< Link to glObject texture */
    bool m_isMipMapped { false };                  /**< Is the texture mip-mapped ? */

    /** \brief This task is valid when a gpu image update task is registered (e.g. after a call
     * to initialize, setParameters, setImageParameters or setData).
     */
    Core::TaskQueue::TaskId m_updateImageTaskId;

    /** \brief This task is valid when a gpu sampler task is registered.
     * e.g. after a call to initialize, setParamaters or setSamplerParamters).
     */
    Core::TaskQueue::TaskId m_updateSamplerTaskId;

    /** mutex to protect non gpu setters, in a thread safe way. */
    std::mutex m_updateMutex;
};
} // namespace Data
} // namespace Engine
} // namespace Ra
