#include "graphactionbuffer.h"

GraphActionBuffer::GraphActionBuffer() {}


void GraphActionBuffer::clear()
{
    actionBuffer.clear();
}

void GraphActionBuffer::addAction(Action action)
{
    actionBuffer.push_back(action);
}

// Node actions
void GraphActionBuffer::setNodeStart(const QString& node)
{
    addAction({ActionType::NodeStart, node, "", ""});
}

void GraphActionBuffer::setNodeGoal(const QString& node)
{
    addAction({ActionType::NodeGoal, node, "", ""});
}

void GraphActionBuffer::setNodeOpen(const QString& node)
{
    addAction({ActionType::NodeOpen, node, "", ""});
}

void GraphActionBuffer::setNodeClosed(const QString& node)
{
    addAction({ActionType::NodeClosed, node, "", ""});
}

void GraphActionBuffer::setNodePath(const QString& node)
{
    addAction({ActionType::NodePath, node, "", ""});
}

// Edge Actions
void GraphActionBuffer::setEdgeOpen(const QString& from, const QString& to)
{
    addAction({ActionType::EdgeOpen, "", from, to});
}

void GraphActionBuffer::setEdgeClosed(const QString& from, const QString& to)
{
    addAction({ActionType::EdgeClosed, "", from, to});
}

void GraphActionBuffer::setEdgePath(const QString& from, const QString& to)
{
    addAction({ActionType::EdgePath, "", from, to});
}

void GraphActionBuffer::clearMap()
{
    addAction({ActionType::ClearMap, "", "", ""});
}