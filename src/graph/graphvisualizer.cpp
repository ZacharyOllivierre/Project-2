#include "graphvisualizer.h"

GraphVisualizer::GraphVisualizer(QPointF minSize, QWidget* parent) : QGraphicsView(parent)
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

    setMinimumWidth(minSize.x());
    setMinimumHeight(minSize.y());


}

void GraphVisualizer::updateGraphData(const std::vector<mlbInfo>& teams, const std::vector<stadiumDistances> dist)
{
    nodes.clear();
    edges.clear();

    // Build nodes
    for (const auto& team : teams)
    {
        Node newNode;
        newNode.stadiumName = QString::fromStdString(team.stadiumName);
        newNode.state = NodeState::Default;

        // Default pos updated before load
        newNode.pos = QPoint(0, 0);

        nodes.push_back(newNode);
    }

    // Build edges
    for (const auto& edge : dist)
    {
        Edge newEdge;
        newEdge.from = QString::fromStdString(edge.originatedStadium);
        newEdge.to = QString::fromStdString(edge.destinationStadium);
        newEdge.weight = edge.distance;

        edges.push_back(newEdge);
    }
}

void GraphVisualizer::loadGraph(int nodeSize, int edgeWidth)
{
    scene->clear();
    nodeItems.clear();
    edgeItems.clear();

    // Calculate node positions
    computeLayout();

    // Create the edge graphic objects
    for (const auto &edge : edges)
    {
        // Default positions
        QPointF start = nodes[0].pos;
        QPointF end = nodes[0].pos;

        // Update default positions
        for (const Node &n : nodes)
        {
            if (n.stadiumName == edge.from)
            {
                start = n.pos;
            }

            if (n.stadiumName == edge.to)
            {
                end = n.pos;
            }
        }

        // Create line
        QPen pen(Qt::black);
        pen.setWidth(edgeWidth);
        auto line = scene->addLine(QLineF(start, end), pen);

        edgeItems.push_back(line);
    }

    // Create the node graphic objects
    for (const auto &node : nodes)
    {
        // Offset needed as ellipses (nodes) are printed from top left
        float offset = nodeSize / 2.0;

        auto item = scene->addEllipse(node.pos.x() - offset, node.pos.y() - offset, nodeSize, nodeSize,
                                      QPen(Qt::black), QBrush(colors[node.state]));

        nodeItems[node.stadiumName] = item;

        // Create node label
        auto text = scene->addText(node.stadiumName);
        text->setDefaultTextColor(Qt::white);

        // Center text under node
        QRectF textRect = text->boundingRect();

        text->setPos(node.pos.x() - textRect.width() / 2.0,
                     node.pos.y() + offset + 2);

        QFont font;
        font.setPointSize(8);
        text->setFont(font);
    }

    // Set scene to fit nodes and edges
    scene->setSceneRect(scene->itemsBoundingRect());
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// Slots
void GraphVisualizer::setNodeStart(const QString& stadiumName)
{
    setNodeState(stadiumName, NodeState::Start);
}

void GraphVisualizer::setNodeEnd(const QString& stadiumName)
{
    setNodeState(stadiumName, NodeState::Goal);
}

void GraphVisualizer::setNodeOpen(const QString& stadiumName)
{
    setNodeState(stadiumName, NodeState::Open);
}

void GraphVisualizer::setNodeClosed(const QString& stadiumName)
{
    setNodeState(stadiumName, NodeState::Closed);
}

void GraphVisualizer::setPath(const QStringList& pathStadiumNames)
{
    for (const QString& stadium : pathStadiumNames)
    {
        setNodeState(stadium, NodeState::Path);
    }
}

void GraphVisualizer::setPath(const std::vector<mlbInfo> pathMlbInfo)
{
    QStringList path;
    path.clear();

    for (const mlbInfo& loc : pathMlbInfo)
    {
        path.push_back(QString::fromStdString(loc.stadiumName));
    }

    setPath(path);
}

void GraphVisualizer::setPath(const QString& stadiumName)
{
    setNodeState(stadiumName, NodeState::Path);
}

void GraphVisualizer::rerollGraph()
{
    loadGraph();
}

void GraphVisualizer::resizeEvent(QResizeEvent* event)
{
    // Base class resize
    QGraphicsView::resizeEvent(event);

    // Fit map to new screen size
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

/*The issue: Nodes need a position on the screen relative to their connections.
This function first gives all the nodes a random position,
Then iteration amount of times, appplies a repulsion force to nodes (seperates them)
and applies an attraction force to connected nodes

Forces are calculated before node pos is updated and when applied are bounded damping force*/
void GraphVisualizer::computeLayout()
{
    int iterations = 10;
    float repulsion = 1000;
    float attraction = 0.05;
    float damping = 0.85;

    float screenWidth = this->width();
    float screenHeight = this->height();

    // Lookup table for stadium name to node pointer
    QMap<QString, Node*> nodeMap;

    // Forces are stored as QPoint (xVelocity, yVelocity)
    QMap<QString, QPointF> forceBuffer;

    // Init forceBuffer and nodeMap
    for (Node& n : nodes)
    {
        nodeMap[n.stadiumName] = &n;
        forceBuffer[n.stadiumName] = QPointF(0,0);
    }

    // Random initiallization
    for (Node& n : nodes)
    {
        n.pos = QPointF(QRandomGenerator::global()->bounded(screenWidth),
                        QRandomGenerator::global()->bounded(screenHeight));
    }

    // Run process of calculating total force for each node each iteration
    for (int i = 0; i < iterations; i++)
    {
        // Repulsion force from every node and every other node
        for (int a = 0; a < nodes.size(); a++)
        {
            for (int b = a + 1; b < nodes.size(); b++)
            {
                Node* nodeA = &nodes[a];
                Node* nodeB = &nodes[b];

                QPointF delta = nodeA->pos - nodeB->pos;

                // Distance formula
                double dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
                dist = std::max(1.0, dist);

                // Avoid division by 0
                if (dist == 0.0)
                {
                    continue;
                }

                QPointF direction = delta / dist;

                // Inverse square repulsion
                QPointF force = direction * (repulsion / dist);

                // Accumulate repulsion force for this iteration
                forceBuffer[nodeA->stadiumName] += force;
                forceBuffer[nodeB->stadiumName] -= force;
            }
        }

        // Attraction for all connected nodes
        for (const Edge& e : edges)
        {
            Node* fromNode = nodeMap.value(e.from, nullptr);
            Node* toNode   = nodeMap.value(e.to, nullptr);

            // Skip if either node is not found
            if (!fromNode || !toNode)
            {
                continue;
            }

            QPointF delta = toNode->pos - fromNode->pos;

            double dist = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y());
            dist = std::max(1.0, dist);

            if (dist == 0.0)
            {
                continue;
            }

            QPointF direction = delta / dist;

            QPointF force = direction * (attraction * dist);

            forceBuffer[fromNode->stadiumName] += force;
            forceBuffer[toNode->stadiumName] -= force;
        }

        // Apply this iterations forces with dampening
        for (Node& n : nodes)
        {
            n.pos += forceBuffer[n.stadiumName] * damping;
        }

        // Reset force buffer
        for (auto i = forceBuffer.begin(); i != forceBuffer.end(); i++)
        {
            i.value() = QPointF(0, 0);
        }
    }
}

void GraphVisualizer::setNodeState(const QString& stadiumName, NodeState state)
{
    // Early out if unrecognized stadium
    if (!nodeItems.contains(stadiumName))
    {
        return;
    }

    auto item = nodeItems[stadiumName];

    item->setBrush(QBrush(colors[state]));
}
