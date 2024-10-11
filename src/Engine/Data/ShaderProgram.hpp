#pragma once

#include <Engine/RaEngine.hpp>

#include <Core/Types.hpp>

#include <Core/CoreMacros.hpp>
#include <Engine/Data/ShaderConfiguration.hpp>

#include <globjects/Program.h>

#include <array>
#include <memory>
#include <string>
#include <type_traits>

namespace globjects {
class Shader;
class NamedString;
class StaticStringSource;
} // namespace globjects

namespace Ra {
namespace Engine {
namespace Data {

class Texture;

/**
 * Abstraction of OpenGL Shader Program
 * \see globjects::Program and https://www.khronos.org/opengl/wiki/Shader
 *
 */
class RA_ENGINE_API ShaderProgram final
{
  public:
    ShaderProgram();
    explicit ShaderProgram( const Data::ShaderConfiguration& shaderConfig );
    ~ShaderProgram();

    void load( const Data::ShaderConfiguration& shaderConfig );
    void reload();

    Data::ShaderConfiguration getBasicConfiguration() const;

    void bind() const;
    void validate() const;
    void unbind() const;

    /// Uniform setters
    template <typename T>
    void setUniform( const char* name, const T& value ) const;
    void setUniform( const char* name, Texture* tex, int texUnit ) const;

    template <typename T>
    void setUniform( gl::GLint location, const T& value ) const {
        m_program->setUniform( location, value );
    }

    //! use automatic texture unit computation
    //! if you want to send a particular texture unit, use setUniform.
    //! It binds tex on an "arbitrary" tex unit.
    //! \warning, call a std::map::find (in O(log(active tex unit in the shader)))
    void setUniformTexture( const char* name, Texture* tex ) const;

    globjects::Program* getProgramObject() const;

    ///\todo go private, and update ShaderConfiguration to add from source !
    void addShaderFromSource( Data::ShaderType type,
                              std::unique_ptr<globjects::StaticStringSource>&& source,
                              const std::string& name = "",
                              bool fromFile           = true );

    void link();

  private:
    struct TextureBinding {
        int m_texUnit { -1 };
        int m_location { -1 };
        TextureBinding( int unit, int location ) : m_texUnit { unit }, m_location { location } {}
        TextureBinding() = default;
    };
    using TextureUnits = std::map<std::string, TextureBinding>;
    TextureUnits textureUnits;

    void loadShader( Data::ShaderType type,
                     const std::string& name,
                     const std::set<std::string>& props,
                     const std::vector<std::pair<std::string, Data::ShaderType>>& includes,
                     bool fromFile              = true,
                     const std::string& version = "#version " +
                                                  ShaderConfiguration::getGLSLVersion() );

    std::string preprocessIncludes( const std::string& name,
                                    const std::string& shader,
                                    int level,
                                    int line = 0 );

  private:
    Data::ShaderConfiguration m_configuration;

    std::array<std::pair<bool, std::unique_ptr<globjects::Shader>>, Data::ShaderType_COUNT>
        m_shaderObjects;
    std::array<std::unique_ptr<globjects::StaticStringSource>, Data::ShaderType_COUNT>
        m_shaderSources;

    std::unique_ptr<globjects::Program> m_program;
};

// declare specialization, definied in .cpp
template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Vector2d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Vector3d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Vector4d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Matrix2d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Matrix3d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Core::Matrix4d& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name, const Scalar& value ) const;

template <>
RA_ENGINE_API void ShaderProgram::setUniform( const char* name,
                                              const std::vector<Scalar>& value ) const;

// Uniform setters
template <typename T>
inline void ShaderProgram::setUniform( const char* name, const T& value ) const {
    m_program->setUniform<T>( name, value );
}

} // namespace Data
} // namespace Engine
} // namespace Ra
