#pragma once

#include <Engine/RaEngine.hpp>

#include <array>
#include <list>
#include <set>
#include <string>
#include <vector>

namespace Ra {
namespace Engine {
namespace Data {

/**
 * Available shader type
 */
enum ShaderType : uint {
    ShaderType_VERTEX = 0,
    ShaderType_FRAGMENT,
    ShaderType_GEOMETRY,
    ShaderType_TESS_CONTROL,
    ShaderType_TESS_EVALUATION,
    ShaderType_COMPUTE,
    ShaderType_COUNT
};

/** A class used to create shader programs later on.
 * A ShaderConfiguration should be added once to the ShaderConfigurationFactory,
 * then the factory must be used to retrieve the added shader configurations.
 * Typical use case :
 *      **************************** CREATION ****************************
 *     // Create the shader configuration once (see MainApplication::addBasicShaders for example)
 *     ShaderConfiguration config("MyConfig");
 *     // Add shader files to the config (note that the file extensions can be whatever you want)
 *     config.addShader(ShaderType_VERTEX, "path/to/shader.vert.glsl");
 *     config.addShader(ShaderType_FRAGMENT, "path/to/shader.frag.glsl");
 *     // Same for other shader types. Vertex and fragment are required, other are optional
 *     // Note that, for a compute shader, only the compute shader is needed.
 *     // Add eventually some properties to the shader configuration
 *     config.addProperties( {"USE_MICROFACET;", "MICROFACET_DISTRIBUTION Trowbridge_Reitz;"} );
 *     // Add also some includes if needed
 *     config.addInclude{"\"MicrofacetFunctions.glsl\";");
 *     // Add the configuration to the factory
 *     ShaderConfigurationFactory::addConfiguration(config);
 *
 *      **************************** USAGE ****************************
 *     // When you want to reuse created shader configurations, just set it using
 *     auto config = ShaderConfigurationFactory::getConfiguration("MyConfig");
 *     // You can then use it to extract the shader from the ShaderProgramManager and use it
 *     auto shader = ShaderProgramManager::getInstance()->getShaderProgram();
 *     shader->bind();
 *     ...
 *
 *     \todo : make configuration and program (in the OpenGL sense) be packed. A shader
 * configuration might contains a ShaderProgram after compiling. This will remove the need for
 * ShaderConfigurationFactory as ShaderManager will offer the same services. Actually, there is
 * redundancy between ShaderConfigurationFactory and ShaderProgramManager
 *
 */
class RA_ENGINE_API ShaderConfiguration final
{
    friend class ShaderProgram;

  public:
    ShaderConfiguration() = default;
    /** Initializes a shader configuration with a name
     * Warning: This does not query the corresponding configuration in the
     * ShaderConfigurationFactory. The proper way to do this is by calling
     * ShaderConfigurationFactory::getConfiguration(name);
     */
    explicit ShaderConfiguration( const std::string& name );

    /** Initializes a configuration with a name, a vertex and a fragment shader
     * This does not add the configuration to the factory
     * ShaderConfigurationFactory::addConfiguration(config) must be called.
     */
    ShaderConfiguration( const std::string& name,
                         const std::string& vertexShader,
                         const std::string& fragmentShader );

    /** Add a shader, from a file, given its type
     *
     * \param type the Type of the shader
     * \param name the file to load
     */
    void addShader( ShaderType type, const std::string& name );

    /** Add a shader, given its glsl source code, of a given type.
     * When a shader is added from a glsl source string, when reloading programs,
     * only the parts coming from files are reloaded.
     *
     * \param type
     * \param source the source code of the shader
     */
    void addShaderSource( ShaderType type, const std::string& source );

    /** Add a property in the form of a \#define
     * The same shader files with different properties leads to different shader programs
     */
    void addProperty( const std::string& prop );
    void addProperties( const std::list<std::string>& props );
    void removeProperty( const std::string& prop );

    /** Add a property in the form of an \#include
     * The same shader files with different properties leads to different shader programs
     */
    void addInclude( const std::string& incl, ShaderType type = ShaderType_FRAGMENT );
    void addIncludes( const std::list<std::string>& incls, ShaderType type = ShaderType_FRAGMENT );
    void removeInclude( const std::string& incl, ShaderType type = ShaderType_FRAGMENT );

    /// Manage named strings (see ShaderProgramManager::addNamedString)
    void addNamedString( const std::string& includepath, const std::string& realfile );

    /** Tell if a shader configuration has at least a vertex and a fragment shader, or a compute
     * shader.
     * */
    bool isComplete() const;

    bool operator<( const ShaderConfiguration& other ) const;

    std::set<std::string> getProperties() const;

    const std::vector<std::pair<std::string, ShaderType>>& getIncludes() const;

    const std::vector<std::pair<std::string, std::string>>& getNamedStrings() const;

    // get default shader configuration
    static ShaderConfiguration getDefaultShaderConfig() { return m_defaultShaderConfig; }

    void setVersion( const std::string& version ) { m_version = version; }
    void setName( const std::string& name ) { m_name = name; }

    const std::string& getName() const { return m_name; }
    const std::string& getVersion() const { return m_version; }
    friend std::ostream& operator<<( std::ostream& stream, const ShaderConfiguration& config );

  private:
    std::string m_name {};
    std::string m_version {};

    /// The second member of the pair defining a shader indicates if the shader comes from file
    /// (true) or from source string (false)
    std::array<std::pair<std::string, bool>, ShaderType_COUNT> m_shaders;

    std::set<std::string> m_properties;

    std::vector<std::pair<std::string, ShaderType>> m_includes;

    std::vector<std::pair<std::string, std::string>> m_named_strings;

    static ShaderConfiguration m_defaultShaderConfig;

    static std::string s_glslVersion;

  public:
    /**
     * \brief set the OpenGL version to use in the generated header for shaders.
     * \param glbinding::Version (e.g. {4,1})
     */
    static void setOpenGLVersion( const glbinding::Version& version );

    /**
     * \brief get the OpenGL version used in the generated header for shaders.
     * \return
     */
    static std::string getGLSLVersion();
};

} // namespace Data
} // namespace Engine
} // namespace Ra
