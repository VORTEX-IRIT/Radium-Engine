#pragma once
#include <Engine/RaEngine.hpp>

#include <Core/Types.hpp>
#include <Engine/Data/Mesh.hpp>
#include <Engine/Data/Texture.hpp>

#include <string>

namespace Ra {
namespace Engine {
namespace Data {
class ShaderProgram;
struct ViewingParameters;

/// \brief Defines a spherical, infinite light sources and, optionally, sky boxes.
/// An operator to convert an EnvironmentTexture to SH matrices for irradiance mapping is defined.
/// An operator to draw a textured sky box is defined.
class RA_ENGINE_API EnvironmentTexture
{
  public:
    /**
     * \brief Environment map type inferred from file names and format.
     * Supported environment type.
     *   - ENVMAP_PFM : cross envmap in PortableFloatMap format
     *       (http://www.pauldebevec.com/Research/HDR/PFM/)
     *   - ENVMAP_CUBE : cube map with 6 textures (1 per face)
     *   - ENVMAP_LATLON : equirectangular Projection of the envmap
     *       (https://en.wikipedia.org/wiki/Equirectangular_projection)
     */
    enum class EnvMapType { ENVMAP_PFM, ENVMAP_CUBE, ENVMAP_LATLON };
    /**
     * Constructors and destructor follow the 'rule of five'
     *  https://en.cppreference.com/w/cpp/language/rule_of_three
     */
    /** \{ */

    /**
     * \brief Construct an envmap from a file.
     * Supported image component of the envmap are the following.
     *
     *   - a pfm file for cross cubemap
     *   - a list of image files for individual cube map faces (see below for naming convention).
     *   - a single png, exr, hdr, jpg file for Spherical equirectangular envmap
     * \see EnvMapType
     *
     * If the cubemap is defined by a list of files, they must be named according to the
     * corresponding face of the cube :
     * *posx* or *-X-plux*  : +X face of the cube <br/>
     * *negx* or *-X-minux* : -X face of the cube <br/>
     * *posy* or *-Y-plux* : +Y face of the cube <br/>
     * *negy* or *-Y-minux* : -Y face of the cube <br/>
     * *posz* or *-Z-plux* : +Z face of the cube <br/>
     * *negz* or *-Z-minux* : -Z face of the cube <br/>
     * \param mapName The list of filenames
     * \param type The file type. Supported file types are PFM for single files or whatever is
     * supported by stb for per-face files.
     * \param isSkybox indicates if the envmap must be associated with a skybox and rendered like
     * this.
     *
     * \note The envmap is transformed at loading/build time to the Radium global frame.
     * \pre The file(s) given as mapName must exist.
     * \todo : if the file given to the constructor does not exist, generates a white envmap ?
     */
    explicit EnvironmentTexture( const std::string& mapName, bool isSkybox = false );
    EnvironmentTexture( const EnvironmentTexture& )            = delete;
    EnvironmentTexture& operator=( const EnvironmentTexture& ) = delete;
    EnvironmentTexture( EnvironmentTexture&& )                 = default;
    EnvironmentTexture& operator=( EnvironmentTexture&& )      = default;
    /// destructor
    ~EnvironmentTexture() = default;
    /**\}*/

    /**
     * \brief Get the name used to construct the environment texture
     * \return The file name, possibly ';' separated list of names, of the environment texture.
     */
    inline const std::string& getImageName() const { return m_name; }
    /**
     * \brief Get the type of the environment texture
     * \see EnvMapType
     * \return The type of the environment texture
     */
    inline EnvMapType getImageType() const { return m_type; }

    /**
     * \brief Saves the spherical image representing the SH-encoded envmap
     * \param filename the file to write, in any format supported by stb library
     */
    void saveShProjection( const std::string& filename );

    /**
     * \brief Return the SH Matrix corresponding to the given color channel.
     * The SH matrix is computed according to the paper
     *    An Efficient Representation for Irradiance Environment Maps,
     *    Ramamoorthi, Ravi and Hanrahan, Pat
     *    SIGGRAPH 2001
     *    https://doi.org/10.1145/383259.383317
     *
     * \param channel The number of the color channel (red = 0, green = 1, blue = 2)
     * \return the SH irradiance matrix for the channel
     */
    const Ra::Core::Matrix4& getShMatrix( int channel );

    /**
     * \brief Render the envmap as a textured cube. This method does nothing if the envmap is not a
     * skybox \param viewParams The viewing parameter used to draw the scene
     */
    void render( const Ra::Engine::Data::ViewingParameters& viewParams );

    /**
     * \brief Set the state of the skybox
     * \param state true to enable skybox rendering, false to just use the SH coefficients
     */
    inline void setSkybox( bool state ) { m_isSkyBox = state; }

    /**
     * \brief Set the multiplicative factor which defined the power of the light source
     * \param s the envmap power
     */
    void setStrength( float s );

    /**
     * \brief Get the multiplicative factor which defined the power of the light source
     * \return the envmap power
     */
    float getStrength() const;

    /**
     * \return true if the envmap is a skybox and might be rendered.
     */
    inline bool isSkybox() const { return m_isSkyBox; }

    /**
     * \return the prefiltered cubemap texture defining the environment
     * \note Prefiltering is made using standard mip-mapping.
     * \todo Prefilter according to bsdf roughness values ?
     *       This will help for envmap lighting but might be wrong for skybox rendering
     */
    Ra::Engine::Data::Texture* getEnvironmentTexture();

    /**
     * \brief Update the OpenGL state of the envmap : texture, skybox and shaders if needed.
     */
    void updateGL();

  private:
    /// \brief Initialize the texture representing the skybox
    void initializeTexture();

    /// \brief Loads and transform portableFloatMap cross image
    void setupTexturesFromPfm();
    /// \brief Loads and transform cubemap from list of image files
    void setupTexturesFromCube();
    /// \brief Loads and transform an equirectangular environment map
    void setupTexturesFromSphericalEquiRectangular();

    /// \brief Compute the irradiance SH matrices
    /// \see getShMatrix
    void computeSHMatrices();

    /**
     * \brief
     * Update the coefficients (i.e. compute the next term in the integral)
     * \param hdr Value of the envmap for the direction (x, y, z)
     * \param domega solid angle of the pixel in the direction (x, y, z)
     */
    void updateCoeffs( float* hdr, float x, float y, float z, float domega );
    float* getPixel( float x, float y, float z );
    void tomatrix();
    Ra::Engine::Data::Texture* getSHImage();

    std::string m_name;
    EnvMapType m_type;

    // The raw pixel values of the envmap
    std::array<std::shared_ptr<float[]>, 6> m_skyData;
    size_t m_width { 0 };
    size_t m_height { 0 };
    float m_shcoefs[9][3];
    Ra::Core::Matrix4 m_shMatrices[3];
    std::unique_ptr<Ra::Engine::Data::Texture> m_shtexture { nullptr };
    /// Is the envmap a Skybox ?
    bool m_isSkyBox { false };
    /// The mesh of the skybox
    std::unique_ptr<Ra::Engine::Data::Mesh> m_displayMesh { nullptr };
    /// The texture of the skybox
    std::unique_ptr<Ra::Engine::Data::Texture> m_skyTexture { nullptr };
    /// The Lod to use for the skybox
    float m_environmentStrength { 1 };
    /// The shader for the skybox
    const Ra::Engine::Data::ShaderProgram* m_skyShader { nullptr };
    bool m_glReady { false };
};

} // namespace Data
} // namespace Engine
} // namespace Ra
