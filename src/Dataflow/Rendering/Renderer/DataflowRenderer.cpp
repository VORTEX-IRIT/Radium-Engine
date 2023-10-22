#include <Dataflow/Rendering/Renderer/DataflowRenderer.hpp>

#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/Data/ViewingParameters.hpp>

#include <Engine/Rendering/RenderObject.hpp>

#include <Engine/Scene/DefaultCameraManager.hpp>
#include <Engine/Scene/DefaultLightManager.hpp>

#include <globjects/Framebuffer.h>

using namespace Ra::Engine;
using namespace Ra::Engine::Scene;
using namespace Ra::Engine::Data;
using namespace Ra::Engine::Rendering;
using namespace gl;

namespace Ra {
namespace Dataflow {
namespace Rendering {
namespace Renderer {
using namespace Ra::Dataflow::Core;

DataflowRenderer::RenderGraphController::RenderGraphController() :
    Ra::Core::Resources::ObservableVoid() {}

void DataflowRenderer::RenderGraphController::configure( DataflowRenderer* renderer,
                                                         int w,
                                                         int h ) {
    m_shaderMngr = renderer->m_shaderProgramManager;
    m_width      = w;
    m_height     = h;
    if ( !m_graphToLoad.empty() ) {
        loadGraph( m_graphToLoad );
        m_graphToLoad = "";
    }
}

void DataflowRenderer::RenderGraphController::resize( int w, int h ) {
    m_width  = w;
    m_height = h;
    if ( m_renderGraph ) { m_renderGraph->resize( m_width, m_height ); }
}

void DataflowRenderer::RenderGraphController::update( const Ra::Engine::Data::ViewingParameters& ) {

    if ( m_renderGraph && m_renderGraph->m_recompile ) {
        // compile the model
        m_renderGraph->init();
        // notify the view the model changes
        notify();
        // notify the model the view may have changed
        m_renderGraph->resize( m_width, m_height );
    }
}

void DataflowRenderer::RenderGraphController::loadGraph( const std::string filename ) {
    m_renderGraph.release();
    auto graphName = filename.substr( filename.find_last_of( '/' ) + 1 );
    m_renderGraph  = std::make_unique<RenderingGraph>( graphName );
    m_renderGraph->setShaderProgramManager( m_shaderMngr );
    m_renderGraph->loadFromJson( filename );
    notify();
}

void DataflowRenderer::RenderGraphController::defferedLoadGraph( const std::string filename ) {
    m_graphToLoad = filename;
}

void DataflowRenderer::RenderGraphController::saveGraph( const std::string filename ) {
    if ( m_renderGraph ) {
        auto graphName = filename.substr( filename.find_last_of( '/' ) + 1 );
        m_renderGraph->saveToJson( filename );
        m_renderGraph->setInstanceName( graphName );
    }
}

void DataflowRenderer::RenderGraphController::resetGraph() {
    m_renderGraph.release();
    m_renderGraph = std::make_unique<RenderingGraph>( "untitled" );
    m_renderGraph->setShaderProgramManager( m_shaderMngr );
}

// ------------------------------------------------------------------------------------------
//  Interface with Radium renderer ...
// ------------------------------------------------------------------------------------------

DataflowRenderer::DataflowRenderer( RenderGraphController& controller ) :
    Renderer(), m_controller { controller }, m_name { m_controller.getRendererName() } {
    m_controller.attachMember( this, &DataflowRenderer::graphChanged );
}

DataflowRenderer::~DataflowRenderer() = default;

void DataflowRenderer::graphChanged() {
    m_graphChanged = true;
}

bool DataflowRenderer::buildRenderTechnique( Ra::Engine::Rendering::RenderObject* ro ) const {
    if ( m_controller.m_renderGraph ) {
        if ( m_controller.m_renderGraph->m_recompile ) {
            m_controller.m_renderGraph->init();
            m_controller.resize( m_width, m_height );
        }
        m_controller.m_renderGraph->buildRenderTechnique( ro );
        return true;
    }
    else { return false; }
}

void DataflowRenderer::initResources() {
    // uses several resources from the Radium engine
    auto resourcesRootDir { RadiumEngine::getInstance()->getResourcesDir() + "Shaders/" };

    m_shaderProgramManager->addShaderProgram(
        { { "Hdr2Ldr" },
          resourcesRootDir + "2DShaders/Basic2D.vert.glsl",
          resourcesRootDir + "2DShaders/Hdr2Ldr.frag.glsl" } );

    m_postprocessFbo = std::make_unique<globjects::Framebuffer>();
}

void DataflowRenderer::initializeInternal() {
    auto cmngr = dynamic_cast<DefaultCameraManager*>(
        Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
    if ( cmngr == nullptr ) {
        cmngr = new DefaultCameraManager();
        Ra::Engine::RadiumEngine::getInstance()->registerSystem( "DefaultCameraManager", cmngr );
    }
    auto lmngr = dynamic_cast<DefaultLightManager*>(
        Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultLightManager" ) );
    if ( lmngr == nullptr ) {
        lmngr = new DefaultLightManager();
        Ra::Engine::RadiumEngine::getInstance()->registerSystem( "DefaultLightManager", lmngr );
    }
    m_lightmanagers.push_back( lmngr );

    // Initialize renderer resources
    initResources();
    m_controller.configure( this, m_width, m_height );

    // TODO update shared textures as the rengerGraphe modify its output : observe the renderGraph
    for ( const auto& t : m_sharedTextures ) {
        m_secondaryTextures.insert( { t.first, t.second.get() } );
    }
}

void DataflowRenderer::resizeInternal() {
    // Resize the graph resources
    m_controller.resize( m_width, m_height );

    // Resize the internal resources
    m_postprocessFbo->bind();
    m_postprocessFbo->attachTexture( GL_COLOR_ATTACHMENT0, m_fancyTexture->texture() );
#ifdef PASSES_LOG
    if ( m_postprocessFbo->checkStatus() != GL_FRAMEBUFFER_COMPLETE ) {
        LOG( Ra::Core::Utils::logERROR ) << "FBO Error (NodeBasedRenderer::m_postprocessFbo) : "
                                         << m_postprocessFbo->statusString();
    }
#endif
    // finished with fbo, unbind to bind default
    globjects::Framebuffer::unbind();
}

void DataflowRenderer::updateStepInternal( const Ra::Engine::Data::ViewingParameters& renderData ) {
    // std::cout << "DataflowRenderer::updateStepInternal() : calling update on graph controller."
    // << std::endl;
    m_controller.update( renderData );
    if ( m_controller.m_renderGraph ) {
        // Update renderTechnique if needed
        if ( m_graphChanged ) {
            buildAllRenderTechniques();
            m_graphChanged = false;
        }
        // TODO, improve light and camera management to prevent multiple alloc/copy ...
        auto lights = getLights();
        lights->clear();
        lights->reserve( getLightManager()->count() );
        for ( size_t i = 0; i < getLightManager()->count(); i++ ) {
            lights->push_back( getLightManager()->getLight( i ) );
        }
        // The graph will take ownership of the light pointer ...
        m_controller.m_renderGraph->setDataSources( allRenderObjects(), lights );
    }
}

void DataflowRenderer::renderInternal( const Ra::Engine::Data::ViewingParameters& renderData ) {
    // std::cout << "DataflowRenderer::renderInternal() : executing the graph." << std::endl;
    // TODO, replace this kind of test by a call to a controller method
    if ( m_controller.m_renderGraph && m_controller.m_renderGraph->m_ready ) {
        // Cameras
        // set input data
        m_cameras.clear();
        m_cameras.push_back( renderData );
        m_controller.m_renderGraph->setCameras( &m_cameras );
        // execute the graph
        m_controller.m_renderGraph->execute();
        // TODO : get all the resulting images (not only the "Beauty" channel
        const auto& images = m_controller.m_renderGraph->getImagesOutput();
        // The first image is the "beauty" channel, set the color texture to this
        m_colorTexture = images[0];
    }
    else { m_colorTexture = nullptr; }
}

void DataflowRenderer::postProcessInternal( const Ra::Engine::Data::ViewingParameters& ) {
    if ( m_colorTexture ) {
        m_postprocessFbo->bind();

        // GL_ASSERT( glDrawBuffers( 1, buffers ) );
        GL_ASSERT( glDrawBuffer( GL_COLOR_ATTACHMENT0 ) );
        GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
        GL_ASSERT( glDepthMask( GL_FALSE ) );

        auto shader = m_postProcessEnabled
                          ? m_shaderProgramManager->getShaderProgram( "Hdr2Ldr" )
                          : m_shaderProgramManager->getShaderProgram( "DrawScreen" );
        shader->bind();
        shader->setUniform( "screenTexture", m_colorTexture, 0 );
        m_quadMesh->render( shader );

        GL_ASSERT( glDepthMask( GL_TRUE ) );
        GL_ASSERT( glEnable( GL_DEPTH_TEST ) );

        m_postprocessFbo->unbind();
    }
}

void DataflowRenderer::debugInternal( const Ra::Engine::Data::ViewingParameters& ) {}

void DataflowRenderer::uiInternal( const Ra::Engine::Data::ViewingParameters& ) {}

} // namespace Renderer
} // namespace Rendering
} // namespace Dataflow
} // namespace Ra

#if 0
#    include <RadiumNBR/NodeBasedRenderer.hpp>

#    include <Core/Containers/MakeShared.hpp>

#    ifdef PASSES_LOG
#        include <Core/Utils/Log.hpp>
using namespace Ra::Core::Utils; // log
#    endif

#    include <Engine/Data/ShaderProgramManager.hpp>
#    include <Engine/Data/Texture.hpp>
#    include <Engine/Data/ViewingParameters.hpp>

#    include <Engine/Rendering/RenderObject.hpp>
#    include <Engine/Scene/DefaultCameraManager.hpp>
#    include <Engine/Scene/DefaultLightManager.hpp>

#    include <globjects/Framebuffer.h>

#    include <RadiumNBR/Passes/DebugPass.hpp>
#    include <RadiumNBR/Passes/UiPass.hpp>

using namespace Ra::Engine;
using namespace Ra::Engine::Scene;
using namespace Ra::Engine::Data;
using namespace Ra::Engine::Rendering;
namespace RadiumNBR {
using namespace gl;

int NodeBasedRendererMagic = 0xFF0F00F0;

static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };

static NodeBasedRenderer::RenderControlFunctor noOpController;

NodeBasedRenderer::NodeBasedRenderer() : Renderer(), m_controller{ noOpController } {}

NodeBasedRenderer::NodeBasedRenderer( NodeBasedRenderer::RenderControlFunctor& controller ) :
    Renderer(), m_controller{ controller }, m_name{ m_controller.getRendererName() } {
    setDisplayNode( m_originalRenderGraph.getDisplayNode() );
}

NodeBasedRenderer::~NodeBasedRenderer() {
    m_displaySinkNode->detach( m_displayObserverId );
    m_originalRenderGraph.destroy();
}

bool NodeBasedRenderer::buildRenderTechnique( RenderObject* ro ) const {
    auto rt = Ra::Core::make_shared<RenderTechnique>();
    for ( size_t level = 0; level < m_originalRenderGraph.getNodesByLevel()->size(); level++ )
    {
        for ( size_t node = 0; node < m_originalRenderGraph.getNodesByLevel()->at( level ).size();
              node++ )
        {
            m_originalRenderGraph.getNodesByLevel()->at( level ).at( node )->buildRenderTechnique(
                ro, *rt );
        }
    }
    rt->updateGL();
    ro->setRenderTechnique( rt );
    return true;
}

void NodeBasedRenderer::initResources() {
    // uses several resources from the Radium engine
    auto resourcesRootDir{ RadiumEngine::getInstance()->getResourcesDir() + "Shaders/" };

    m_shaderProgramManager->addShaderProgram(
        { { "Hdr2Ldr" },
          resourcesRootDir + "2DShaders/Basic2D.vert.glsl",
          resourcesRootDir + "2DShaders/Hdr2Ldr.frag.glsl" } );

    m_postprocessFbo = std::make_unique<globjects::Framebuffer>();
}

void NodeBasedRenderer::loadFromJson( const std::string& jsonFilePath ) {
    m_jsonFilePath = jsonFilePath;

    if ( m_jsonFilePath != "" )
    {
        m_originalRenderGraph.loadFromJson( jsonFilePath );
        m_originalRenderGraph.init();
    }
    else
    { std::cerr << "No Json was given to load a render graph." << std::endl; }
}

void NodeBasedRenderer::compileRenderGraph() {
    m_originalRenderGraph.init();
    m_originalRenderGraph.resize( m_width, m_height );
    buildAllRenderTechniques();
    m_displayedTexture = m_fancyTexture.get();
}

void NodeBasedRenderer::reloadRenderGraphFromJson() {
    if ( m_jsonFilePath != "" )
    {
        std::cout << "Reloading Render Graph from Json..." << std::endl;
        // Destroy the resources used by the nodes
        m_originalRenderGraph.destroy();

        // Clear the nodes
        m_originalRenderGraph.clearNodes();

        // Reload
        m_originalRenderGraph.loadFromJson( m_jsonFilePath );
        m_originalRenderGraph.init();
        m_originalRenderGraph.resize( m_width, m_height );
        buildAllRenderTechniques();

        // Reset displayed texture
        m_displayedTexture = m_fancyTexture.get();

        std::cout << "Render Graph Reloaded!" << std::endl;
    }
    else
    { std::cerr << "No Json was given to reload a render graph." << std::endl; }
}

void NodeBasedRenderer::initializeInternal() {

    // TODO : this must be done only once, see register system ...
    auto cmngr = dynamic_cast<DefaultCameraManager*>(
        Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
    if ( cmngr == nullptr )
    {
        cmngr = new DefaultCameraManager();
        Ra::Engine::RadiumEngine::getInstance()->registerSystem( "DefaultCameraManager", cmngr );
    }
    auto lmngr = dynamic_cast<DefaultLightManager*>(
        Ra::Engine::RadiumEngine::getInstance()->getSystem( "DefaultLightManager" ) );
    if ( lmngr == nullptr )
    {
        lmngr = new DefaultLightManager();
        Ra::Engine::RadiumEngine::getInstance()->registerSystem( "DefaultLightManager", lmngr );
    }
    m_lightmanagers.push_back( lmngr );

    // Initialize renderer resources
    initResources();
    m_controller.configure( this, m_width, m_height );

    for ( const auto& t : m_sharedTextures )
    { m_secondaryTextures.insert( { t.first, t.second.get() } ); }

    // Todo cache this in an attribute ?
    auto resourcesCheck = Ra::Core::Resources::getResourcesPath(
        reinterpret_cast<void*>( &RadiumNBR::NodeBasedRendererMagic ), { "Resources/RadiumNBR" } );
    if ( !resourcesCheck )
    {
        LOG( Ra::Core::Utils::logERROR ) << "Unable to find resources for NodeBasedRenderer!";
        return;
    }
    auto resourcesPath{ *resourcesCheck };
}

void NodeBasedRenderer::resizeInternal() {
    // Resize each internal resources
    m_controller.resize( m_width, m_height );
    m_originalRenderGraph.resize( m_width, m_height );

    m_postprocessFbo->bind();
    m_postprocessFbo->attachTexture( GL_COLOR_ATTACHMENT0, m_fancyTexture->texture() );
#    ifdef PASSES_LOG
    if ( m_postprocessFbo->checkStatus() != GL_FRAMEBUFFER_COMPLETE )
    {
        LOG( Ra::Core::Utils::logERROR ) << "FBO Error (NodeBasedRenderer::m_postprocessFbo) : "
                                         << m_postprocessFbo->checkStatus();
    }
#    endif
    // finished with fbo, unbind to bind default
    globjects::Framebuffer::unbind();
}

void NodeBasedRenderer::renderInternal( const ViewingParameters& renderData ) {
    // Run the render graph
    m_originalRenderGraph.execute();
}

// Draw debug stuff, do not overwrite depth map but do depth testing
void NodeBasedRenderer::debugInternal( const ViewingParameters& renderData ) {
#    if 0
    if ( m_drawDebug )
    {
        const ShaderProgram* shader;

        m_postprocessFbo->bind();
        GL_ASSERT( glDisable( GL_BLEND ) );
        GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
        GL_ASSERT( glDepthMask( GL_FALSE ) );
        GL_ASSERT( glDepthFunc( GL_LESS ) );

        glDrawBuffers( 1, buffers );

        for ( const auto& ro : m_debugRenderObjects )
        {
            ro->render( RenderParameters{}, renderData );
        }

        DebugRender::getInstance()->render( renderData.viewMatrix, renderData.projMatrix );

        m_postprocessFbo->unbind();

        m_uiXrayFbo->bind();
        // Draw X rayed objects always on top of normal objects
        GL_ASSERT( glDepthMask( GL_TRUE ) );
        GL_ASSERT( glClear( GL_DEPTH_BUFFER_BIT ) );
        for ( const auto& ro : m_xrayRenderObjects )
        {
            if ( ro->isVisible() )
            {
                shader = ro->getRenderTechnique()->getShader();

                // bind data
                shader->bind();
                // lighting for Xray : fixed
                shader->setUniform( "light.color", Ra::Core::Utils::Color::Grey( 5.0 ) );
                shader->setUniform( "light.type", Light::LightType::DIRECTIONAL );
                shader->setUniform( "light.directional.direction", Ra::Core::Vector3( 0, -1, 0 ) );

                Ra::Core::Matrix4 M = ro->getTransformAsMatrix();
                shader->setUniform( "transform.proj", renderData.projMatrix );
                shader->setUniform( "transform.view", renderData.viewMatrix );
                shader->setUniform( "transform.model", M );

                ro->getRenderTechnique()->getMaterial()->bind( shader );

                // render
                ro->getMesh()->render();
            }
        }
        m_uiXrayFbo->unbind();
    }
#    endif
}

// Draw UI stuff, always drawn on top of everything else + clear ZMask
// TODO: NODEGRAPH! Unused ?
void NodeBasedRenderer::uiInternal( const ViewingParameters& renderData ) {
#    if 0
    const ShaderProgram* shader;

    m_uiXrayFbo->bind();
    glDrawBuffers( 1, buffers );
    // Enable z-test
    GL_ASSERT( glDepthMask( GL_TRUE ) );
    GL_ASSERT( glEnable( GL_DEPTH_TEST ) );
    GL_ASSERT( glDepthFunc( GL_LESS ) );
    GL_ASSERT( glClear( GL_DEPTH_BUFFER_BIT ) );
    for ( const auto& ro : m_uiRenderObjects )
    {
        if ( ro->isVisible() )
        {
            shader = ro->getRenderTechnique()->getShader();

            // bind data
            shader->bind();

            Ra::Core::Matrix4 M  = ro->getTransformAsMatrix();
            Ra::Core::Matrix4 MV = renderData.viewMatrix * M;
            Ra::Core::Vector3 V  = MV.block<3, 1>( 0, 3 );
            Scalar d         = V.norm();

            Ra::Core::Matrix4 S    = Ra::Core::Matrix4::Identity();
            S.coeffRef( 0, 0 ) = S.coeffRef( 1, 1 ) = S.coeffRef( 2, 2 ) = d;

            M = M * S;

            shader->setUniform( "transform.proj", renderData.projMatrix );
            shader->setUniform( "transform.view", renderData.viewMatrix );
            shader->setUniform( "transform.model", M );

            ro->getRenderTechnique()->getMaterial()->bind( shader );

            // render
            ro->getMesh()->render();
        }
    }
    m_uiXrayFbo->unbind();
#    endif
}

void NodeBasedRenderer::postProcessInternal( const ViewingParameters& /* renderData */ ) {
    if ( m_colorTexture )
    {
        m_postprocessFbo->bind();

        // GL_ASSERT( glDrawBuffers( 1, buffers ) );
        GL_ASSERT( glDrawBuffer( GL_COLOR_ATTACHMENT0 ) );
        GL_ASSERT( glDisable( GL_DEPTH_TEST ) );
        GL_ASSERT( glDepthMask( GL_FALSE ) );

        auto shader = m_postProcessEnabled
                          ? m_shaderProgramManager->getShaderProgram( "Hdr2Ldr" )
                          : m_shaderProgramManager->getShaderProgram( "DrawScreen" );
        shader->bind();
        shader->setUniform( "screenTexture", m_colorTexture, 0 );
        m_quadMesh->render( shader );

        GL_ASSERT( glDepthMask( GL_TRUE ) );
        GL_ASSERT( glEnable( GL_DEPTH_TEST ) );

        m_postprocessFbo->unbind();
    }
}

void NodeBasedRenderer::updateStepInternal( const ViewingParameters& renderData ) {
    if ( m_reloadJson )
    {
        reloadRenderGraphFromJson();
        if ( m_resetPath )
        {
            m_jsonFilePath = m_jsonFilePath.substr( 0, m_jsonFilePath.rfind( '/' ) + 1 );
            m_resetPath    = false;
        }
        m_reloadJson = false;
    }

    if ( m_originalRenderGraph.m_recompile )
    {
        std::cerr << "NodeBasedRenderer::updateStepInternal :Recompiling Graph\n";
        compileRenderGraph();
        m_originalRenderGraph.m_recompile = false;
    }

    // Render objects
    m_originalRenderGraph.getDataNode<NodeTypeRenderObject>()->setElements( *allRenderObjects() );
    // Lights
    std::vector<const Ra::Engine::Scene::Light*> lights;
    for ( size_t i = 0; i < getLightManager()->count(); i++ )
    { lights.push_back( getLightManager()->getLight( i ) ); }
    m_originalRenderGraph.getDataNode<NodeTypeLight>()->setElements( lights );
    // Cameras
    std::vector<NodeTypeCamera> cameras;
    cameras.push_back( renderData );
    m_originalRenderGraph.getDataNode<NodeTypeCamera>()->setElements( cameras );
    // Update the render graph

    m_originalRenderGraph.update();
}

void NodeBasedRenderer::setDisplayNode( DisplaySinkNode* displayNode ) {
    m_displaySinkNode = displayNode;
    m_displayObserverId =
        m_displaySinkNode->attachMember( this, &NodeBasedRenderer::observeDisplaySink );
}

void NodeBasedRenderer::observeDisplaySink( const std::vector<NodeTypeTexture*>& graphOutput ) {
    // TODO : find a way to make the renderer observable to manage ouput texture in the applicaiton
    // gui if needed
    std::cout << "NodeBasedRenderer::observeDisplaySink - connected textures ("
              << graphOutput.size() << ") : \n";
    /*
    for ( const auto t : graphOutput )
    {
        if ( t ) { std::cout << "\t" << t->getName() << "\n"; }
        else
        { std::cout << "\tName not available yet. Must run the graph a first time\n"; }
    }
    */
    // Add display nodes' linked textures to secondary textures
    m_secondaryTextures.clear();
    for ( const auto& t : m_sharedTextures )
    { m_secondaryTextures.insert( { t.first, t.second.get() } ); }

    bool colorTextureSet = false;
    if ( m_displaySinkNode )
    {
        auto textures = m_displaySinkNode->getTextures();
        for ( const auto t : textures )
        {
#    ifdef GRAPH_CALL_TRACE
            std::cout << t->getName() << std::endl;
#    endif
            if ( t )
            {
                if ( !colorTextureSet )
                {
                    m_colorTexture  = t;
                    colorTextureSet = true;
                }
                else
                { m_secondaryTextures.insert( { t->getName(), t } ); }
            }
        }
    }

    if ( !colorTextureSet )
    {
        m_colorTexture  = m_fancyTexture.get();
        colorTextureSet = true;
    }
}

} // namespace RadiumNBR
#endif
