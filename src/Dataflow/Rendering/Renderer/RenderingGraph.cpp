#include <Dataflow/Rendering/Renderer/RenderingGraph.hpp>

#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderTechnique.hpp>

using namespace Ra::Engine::Rendering;

namespace Ra {
namespace Dataflow {
namespace Rendering {
namespace Renderer {
using namespace Ra::Dataflow::Rendering::Nodes;
using namespace Ra::Dataflow::Core;

void RenderingGraph::init() {
    DataflowGraph::init();
}

bool RenderingGraph::addNode( Node* newNode ) {
    auto added = DataflowGraph::addNode( newNode );
    if ( added ) {
        // Todo : is there something to do ?
    }
    return added;
}

bool RenderingGraph::removeNode( Node* node ) {
    auto removed = DataflowGraph::removeNode( node );
    if ( removed ) {
        // Todo : is there something to do ?
    }
    return removed;
}

/*
bool RenderingGraph::postCompilationOperation() {
#if 0
    m_renderingNodes.clear();
    m_rtIndexedNodes.clear();
    m_dataProviders.clear();
    if ( m_displaySinkNode && m_displayObserverId != -1 ) {
        m_displaySinkNode->detach( m_displayObserverId );
        m_displayObserverId = -1;
        m_displaySinkNode   = nullptr;
    }
    const auto& compiledNodes = getNodesByLevel();
    int idx                   = 1; // The renderTechnique id = 0 is reserved for ui/debug objects
    for ( const auto& lvl : *compiledNodes ) {
        for ( auto n : lvl ) {
            auto renderNode = dynamic_cast<RenderingNode*>( n );
            if ( renderNode != nullptr ) {
                m_renderingNodes.push_back( renderNode );
                if ( renderNode->hasRenderTechnique() ) {
                    renderNode->setIndex( idx++ );
                    m_rtIndexedNodes.push_back( renderNode );
                }
                renderNode->setShaderProgramManager( m_shaderMngr );
            }
            else {
                auto sceneNode = dynamic_cast<SceneNode*>( n );
                if ( sceneNode ) { m_dataProviders.push_back( sceneNode ); }
                else {
                    // Manage all sinks ...
                    auto displaySink = dynamic_cast<DisplaySinkNode*>( n );
                    if ( displaySink ) {
                        m_displaySinkNode = displaySink;
                        // observe the displaySink
                        m_displayObserverId =
                            displaySink->attachMember( this, &RenderingGraph::observeSinks );
                    }
                }
            }
        }
    }
#if 0
    std::cout << "RenderingGraph::postCompilationOperation : got " <<
                 m_renderingNodes.size() << " compiled rendering nodes with " <<
                 m_rtIndexedNodes.size() << " render-passes, "  <<
                 m_dataProviders.size() << " scene nodes. \n";
#endif
#endif
return true;
}
*/
#if 0
void RenderingGraph::observeSinks( const std::vector<TextureType*>& graphOutput ) {
    m_outputTextures = graphOutput;
    /*
    std::cout << "Available output textures are :" << std::endl;
    int i = 0;
    for (auto t : m_outputTextures ) {
        std::cout << "\t tex " << i++ << " : ";
        if (t) {
            std::cout << t->getName() << std::endl;
        } else {
            std::cout << "nullptr" << std::endl;
        }
    }
    */
}
#endif

const std::vector<TextureType*>& RenderingGraph::getImagesOutput() const {
    return m_outputTextures;
}

void RenderingGraph::clearNodes() {
    DataflowGraph::clearNodes();
    m_renderingNodes.clear();
    m_rtIndexedNodes.clear();
}

void RenderingGraph::buildRenderTechnique( Ra::Engine::Rendering::RenderObject* ro ) const {
    auto rt = std::make_shared<RenderTechnique>();
    for ( const auto& rn : m_rtIndexedNodes ) {
        rn->buildRenderTechnique( ro, *rt );
    }
    rt->updateGL();
    ro->setRenderTechnique( rt );
}
} // namespace Renderer
} // namespace Rendering
} // namespace Dataflow
} // namespace Ra
