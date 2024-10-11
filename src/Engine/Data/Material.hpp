#pragma once

#include <Engine/RaEngine.hpp>

#include <map>
#include <string>

#include <Engine/Data/MaterialTextureSet.hpp>
#include <Engine/Data/RenderParameters.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/RadiumEngine.hpp>

namespace Ra {
namespace Engine {
namespace Data {

/** \brief Base class for materials.
 *
 * Do not assume a given Material representation but only make the difference between opaque and
 * transparent materials.
 */
class RA_ENGINE_API Material : public Data::ShaderParameterProvider
{
  public:
    /** \brief Identifies the type of the material.
     *
     *  MAT_OPAQUE and MAT_TRANSPARENT implements the GLSL "surfacic" BSDF interface
     *  MAT_DENSITY implements the GLSL "volumetric" interface
     */
    enum class MaterialAspect {
        MAT_OPAQUE,      /// <- The material is either OPAQUE or TRANSPARENT
        MAT_TRANSPARENT, /// <- The material is TRANSPARENT
        MAT_DENSITY      /// <- The material implements the VOLUME interface
    };

    /**
     * \note The material does not have ownership on its texture nor its shaders.
     * This destructor do not delete the associated textures and the corresponding shaders.
     */
    virtual ~Material() = default;

    /** \return the name of the material instance
     */
    inline const std::string& getInstanceName() const { return m_instanceName; }

    /** \return the name of the material, can be used a UUID
     * \note the material name is expected to be used to define the ShaderConfiguration name
     */
    inline const std::string& getMaterialName() const { return m_materialName; }

    /** set the aspect (MAT_OPAQUE or MAT_TRANSPARENT) of the material.
     * \param aspect
     */
    inline void setMaterialAspect( const MaterialAspect& aspect ) { m_aspect = aspect; }

    /** \brief Get the aspect (MAT_OPAQUE or MAT_TRANSPARENT) of the material.
     *
     * \return the current aspect of the Material
     */
    inline const MaterialAspect& getMaterialAspect() const { return m_aspect; }

    /** \return true if the material is transparent.
     */
    virtual bool isTransparent() const { return m_aspect == MaterialAspect::MAT_TRANSPARENT; }

    /** * \brief Get the list of properties the material migh use in a shader.
     *
     * Each property will be added to the shader used for rendering this material under the form
     * "#define theProperty". Shaders that support the given property could then fully render the
     * material. Others migh render the meterial eroneously.
     *
     * The default implementation returns an empty list.
     *
     * \todo : Validate this proposal
     * \todo : make the property list modifiable as well
     */
    std::list<std::string> getPropertyList() const override {
        return ShaderParameterProvider::getPropertyList();
    }

    /** \brief Makes the Material take its base color from the VERTEX_COLOR attribute of the
     * rendered geometry.
     *
     * \param state activate (true) or deactivate (false) VERTEX_COLOR attribute usage
     *
     * Any material that support per-vertex color parameterization should implement this method
     * accordingly
     */
    virtual void setColoredByVertexAttrib( [[maybe_unused]] bool state ) {};

    /** \brief Indicates if the material takes the VERTEX_COLOR attribute into account.
     */
    virtual bool isColoredByVertexAttrib() const { return false; }

    /** \brief Mark the Material as needing update before the next OpenGL call.
     */
    inline void needUpdate() { setDirty(); }

  protected:
    /** \brief Creates a named material with the given aspect.
     *
     * \param instanceName
     * \param materialName
     * \param aspect
     */
    explicit Material( const std::string& instanceName,
                       const std::string& materialName,
                       MaterialAspect aspect = MaterialAspect::MAT_OPAQUE );

    /** \brief Change the material name.
     *
     * \note This method should be used carefully as the name is a key for render technique factory
     */
    inline void setMaterialName( std::string newName ) { m_materialName = std::move( newName ); }

    /// Return dirty state
    bool isDirty() { return m_isDirty; }

    /// Set dirty state to true
    void setDirty() { m_isDirty = true; }

    /// Set dirty state to false
    void setClean() { m_isDirty = false; }

  private:
    /// Material instance name
    std::string m_instanceName {};
    /// Material aspect
    MaterialAspect m_aspect { MaterialAspect::MAT_OPAQUE };
    /// Dirty mark : true if the openGL state of the material need to be updated before next draw
    /// call
    bool m_isDirty { true };
    /// Unique material name that can be used to identify the material class
    std::string m_materialName;
};

} // namespace Data
} // namespace Engine
} // namespace Ra
