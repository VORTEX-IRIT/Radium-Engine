#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/Node.hpp>

#include <nodes/NodeDataModel>

#include <string>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {
using namespace Ra::Dataflow::Core;

/**
 * \brief Allow to manage connections in the RadiumNodeEditor external
 */
class RA_DATAFLOW_API ConnectionStatusData : public QtNodes::NodeData
{
  public:
    ConnectionStatusData( std::shared_ptr<Node> node, const std::string& outputName ) :
        m_node { node }, m_outputName { outputName } {}

    QtNodes::NodeDataType type() const override {
        return QtNodes::NodeDataType { "connection", "connectionStatus" };
    }

    std::shared_ptr<Node> getNode() { return m_node; }
    std::string getOutputName() { return m_outputName; }

  private:
    std::shared_ptr<Node> m_node { nullptr };
    std::string m_outputName;
};

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
