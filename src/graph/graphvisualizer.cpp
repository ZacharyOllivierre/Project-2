#include "graphvisualizer.h"

GraphVisualizer::GraphVisualizer(QWidget* parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    // Set colors
    colors[NodeState::Default] = Qt::gray;
    colors[NodeState::Open]    = Qt::yellow;
    colors[NodeState::Closed]  = Qt::red;
    colors[NodeState::Path]    = Qt::green;
    colors[NodeState::Start]   = Qt::blue;
    colors[NodeState::Goal]    = Qt::magenta;
}

void GraphVisualizer::loadGraph(const std::vector<Node> &nodes, const std::vector<Edge>& edges)
{
    scene->clear();
    nodeItems.clear();
    edgeItems.clear();

    // Create the nodes
    int size = 20;
    for (const auto &node : nodes)
    {
        auto item = scene->addEllipse(node.pos.x(), node.pos.y(), size, size,
                                      QPen(Qt::black), QBrush(colors[node.state]));

        nodeItems[node.teamName] = item;
    }

    // Create the edges
    for (const auto &edge : edges)
    {
        // Default positions
        QPointF start = nodes[0].pos;
        QPointF end = nodes[0].pos;

        // Update default positions
        for (const Node &n : nodes)
        {
            if (n.teamName == edge.from)
            {
                start = n.pos;
            }

            if (n.teamName == edge.to)
            {
                end = n.pos;
            }
        }

        // Create line
        auto line = scene->addLine(QLineF(start, end), QPen(Qt::black));
        edgeItems.push_back(line);
    }
}

void GraphVisualizer::setNodeState(const QString& teamName, NodeState state)
{
    // Early out if unrecognized teamName
    if (!nodeItems.contains(teamName))
    {
        return;
    }

    auto item = nodeItems[teamName];

    item->setBrush(QBrush(colors[state]));
}

void GraphVisualizer::setNodeOpen(const QString& id)
{
    setNodeState(id, NodeState::Open);
}

void GraphVisualizer::setNodeClosed(const QString& id)
{
    setNodeState(id, NodeState::Closed);
}

void GraphVisualizer::setPath(const QStringList& path)
{
    for (const QString& id : path)
    {
        setNodeState(id, NodeState::Path);
    }
}

/*The issue: Nodes need a position on the screen relative to their connections.
This function first gives all the nodes a random position,
Then over iteration amount of times, appplies a repulsion force to nodes (seperates them)
and applies an attraction force to connected nodes*/
void computeLayout(std::vector<Node>& nodes, const std::vector<Edge>& edges)
{
    // Random initiallization
    for (Node& n : nodes)
    {
        n.pos = QPointF(QRandomGenerator::global()->bounded(600), QRandomGenerator::global()->bounded(400));
    }

    int iterations = 100;
    float repulsion = 8000;
    float attraction = 0.05;

    for (int i = 0; i < iterations; i++)
    {
        // Repulsion force from every node and every other node (n^2)
        for (Node& a : nodes)
        {
            for (Node& b : nodes)
            {
                // Dont apply forces to a node and itself
                if (a.teamName == b.teamName)
                {
                    continue;
                }

                //Here
            }
        }
    }
}