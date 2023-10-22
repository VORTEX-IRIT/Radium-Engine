#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/Node.hpp>

#include <Core/Utils/Color.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/Data/ViewingParameters.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Scene/Light.hpp>

namespace Ra::Engine::Data {
class ShaderProgramManager;
}

namespace Ra {
namespace Dataflow {
namespace Rendering {
namespace Nodes {

/**
 * Defining some useful aliases for data type
 *
 */

using RenderObjectType = std::shared_ptr<Ra::Engine::Rendering::RenderObject>;
using LightType        = const Ra::Engine::Scene::Light*;
using CameraType       = Ra::Engine::Data::ViewingParameters;
using ColorType        = Ra::Core::Utils::Color;
using TextureType      = Ra::Engine::Data::Texture;

/**
 * Base class for Rendering nodes.
 * Rendering nodes are nodes with some interface needed to render the scene.
 */
class RA_DATAFLOW_API RenderingNode : public Dataflow::Core::Node,
                                      public Ra::Core::Utils::IndexedObject
{
  public:
    using Dataflow::Core::Node::Node;

    /// The resize(uint32_t width, uint32_t height) function is called when the application gets
    /// resized. Its goal is to resize the potential internal textures if needed.
    /// @param width The new width of the surface.
    /// @param height The new height of the surface.
    virtual void resize( uint32_t width, uint32_t height ) = 0;

    /// Build a render technic per material.
    /// @param ro The render object to get the material from
    /// @param rt The render technic to build
    virtual void buildRenderTechnique( const Ra::Engine::Rendering::RenderObject*,
                                       Ra::Engine::Rendering::RenderTechnique& ) const {};

    /// Indicate if the nod needs to setup a rendertechnique on RenderObjects
    virtual bool hasRenderTechnique() { return false; }

    /// Sets the shader program manager
    void setShaderProgramManager( Ra::Engine::Data::ShaderProgramManager* shaderMngr ) {
        m_shaderMngr = shaderMngr;
    }

    static const std::string getTypename() { return "RenderingNode"; }

  protected:
    void toJsonInternal( nlohmann::json& ) const override {}
    void fromJsonInternal( const nlohmann::json& ) override {}

    /// The renderer's shader program manager
    Ra::Engine::Data::ShaderProgramManager* m_shaderMngr;
};

} // namespace Nodes
} // namespace Rendering
} // namespace Dataflow
} // namespace Ra
