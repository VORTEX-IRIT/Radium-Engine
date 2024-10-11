#pragma once

#include <Engine/RaEngine.hpp>

#include <vector>

#include <nlohmann/json.hpp>

#include <Core/Types.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/EnumConverter.hpp>
#include <Core/Utils/Log.hpp>
#include <Core/Utils/StdOptional.hpp>

#include <Core/Containers/VariableSet.hpp>

#include <Engine/Data/ShaderProgram.hpp>

namespace Ra {
namespace Engine {
namespace Data {

class Texture;

/**
 * \brief Management of shader parameters with automatic binding of a named parameter to the
 * corresponding glsl uniform.
 *
 * \note Automatic binding is only available for supported type described in BindableTypes.
 * \note Enums are stored according to their underlying_type. Enum management is automatic except
 * when requesting for the associated uniformBindableSet. To access bindable set containing a given
 * enum with type Enum, use `getAllVariable<typename std::underlying_type<typename Enum>::type>`
 */
class RA_ENGINE_API RenderParameters final : public Core::VariableSet
{
  public:
    /**
     * \brief Special type for Texture parameter
     */
    using TextureInfo = std::pair<Data::Texture*, int>;

    /// List of bindable types, to be used with static visitors
    using BindableTypes = Core::Utils::TypeList<bool,
                                                Core::Utils::Color,
                                                int,
                                                uint,
                                                Scalar,
                                                TextureInfo,
                                                std::vector<int>,
                                                std::vector<uint>,
                                                std::vector<Scalar>,
                                                Core::Vector2,
                                                Core::Vector3,
                                                Core::Vector4,
                                                Core::Matrix2,
                                                Core::Matrix3,
                                                Core::Matrix4,
                                                std::reference_wrapper<RenderParameters>,
                                                std::reference_wrapper<const RenderParameters>>;

    /** \brief Adding a texture parameter.
     *
     * \tparam T The type of parameter to add. Must be derived from Texture for this overload.
     * \param name Name of the parameter
     * \param tex Texture to add in the parameterSet
     * \param texUnit Texture unit associated with the texture object.
     * The default (-1) for the texUnit parameter implies automatic uniform binding for the
     * texture unit associated with the named sampler.
     * If texUnit is given, then uniform binding will be made at this explicit location.
     */
    template <typename T,
              typename std::enable_if<std::is_base_of<Data::Texture, T>::value, bool>::type = true>
    void setTexture( const std::string& name, T* tex, int texUnit = -1 ) {
        setVariable( name, TextureInfo { tex, texUnit } );
    }

    using VariableSet::setVariable;

    /// overload create ref wrapper for RenderParameters variable
    void setVariable( const std::string& name, RenderParameters& rp ) {
        VariableSet::setVariable( name, std::ref( rp ) );
    }
    void setVariable( const std::string& name, const RenderParameters& rp ) {
        VariableSet::setVariable( name, std::cref( rp ) );
    }

    /** \brief Bind the parameter uniform on the shader program.
     *
     * \note, this will only bind the supported parameter types.
     * \param shader The shader to bind to.
     */
    void bind( const Data::ShaderProgram* shader ) const {
        visit( StaticParameterBinder {}, shader );
    }

  private:
    /** \brief Static visitor to bind the stored parameters.
     * \note Binds only statically supported types. To bind unsupported types, use a custom
     * dynamic visitor and the RenderParameter::visit method.
     */
    class StaticParameterBinder
    {
      public:
        /// Type supported by the binder, as expected by VariableSet
        using types = BindableTypes;

        /** \brief Binds a color parameter as this requires special access to the parameter value.
         */
        void operator()( const std::string& name,
                         const Ra::Core::Utils::Color& p,
                         const Data::ShaderProgram* shader ) {
            shader->setUniform( name.c_str(), Ra::Core::Utils::Color::VectorType( p ) );
        }

        /** \brief Binds a Texture parameter as this requires special access to the parameter value.
         */
        void operator()( const std::string& name,
                         const RenderParameters::TextureInfo& p,
                         const Data::ShaderProgram* shader ) {
            auto [tex, texUnit] = p;
            if ( texUnit == -1 ) { shader->setUniformTexture( name.c_str(), tex ); }
            else { shader->setUniform( name.c_str(), tex, texUnit ); }
        }

        /** \brief Binds a embedded const RenderParameter.
         *
         * This allows to build hierarchies of RenderParameters.
         */
        template <typename T>
        void operator()( const std::string& /*name*/,
                         const std::reference_wrapper<T>& p,
                         const Data::ShaderProgram* shader ) {
            p.get().bind( shader );
        }

        /** \brief Bind any type of parameter that do not requires special access
         */
        template <typename T>
        void operator()( const std::string& name, const T& p, const Data::ShaderProgram* shader ) {
            shader->setUniform( name.c_str(), p );
        }
    };
};

/** \brief Interface to define metadata (constraints, description, ...) for the editing of parameter
 * set
 */
class RA_ENGINE_API ParameterSetEditingInterface
{
  public:
    virtual ~ParameterSetEditingInterface() = default;

    /** \brief Get a json containing metadata about the parameters.
     *
     * \return the metadata in json format
     */
    virtual nlohmann::json getParametersMetadata() const = 0;

    /// Load the ParameterSet  description
    static void loadMetaData( const std::string& basename, nlohmann::json& destination );
};

/** \brief Shader program parameter provider.
 *
 * A ShaderParameterProvider is an object that is associated to a render technique to provide the
 * uniform parameter set for the program. When an RenderObject is drawn using a given
 * rendertechnique, the ShaderParameterProvider associated to the renderTechnique is responsible to
 * set all the uniforms needed by the rendertechnique.
 */
class RA_ENGINE_API ShaderParameterProvider
{
  public:
    virtual ~ShaderParameterProvider() = default;
    virtual RenderParameters& getParameters() { return m_renderParameters; }
    virtual const RenderParameters& getParameters() const { return m_renderParameters; }

    /** \brief Update the OpenGL states used by the ShaderParameterProvider.
     *
     * These state could be the ones from an associated material (textures, precomputed tables or
     * whatever data associated to the material)  or some parameters that are
     * specific to the provider semantic.
     */
    virtual void updateGL() = 0;

    /** \brief Update the attributes of the ShaderParameterProvider to their actual values stored in
     * the renderParameters.
     */
    virtual void updateFromParameters() {};

    /** \brief Get the list of properties the provider might use in a shader.
     *
     * Each property will be added to the shader used for rendering under the form
     * "#define theProperty" when the provider is associated with the render technique.
     *
     * The default implementation returns an empty list.
     *
     * \todo : Validate this proposal
     */
    virtual std::list<std::string> getPropertyList() const { return {}; };

  private:
    /// The parameters to set for a shader
    RenderParameters m_renderParameters;
};

} // namespace Data
} // namespace Engine
} // namespace Ra
