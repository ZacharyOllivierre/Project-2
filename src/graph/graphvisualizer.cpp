#include "graphvisualizer.h"

GraphVisualizer::GraphVisualizer(QPointF minSize, QWidget* parent) : QGraphicsView(parent)
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    // Set colors
    nodeColors[NodeState::Default] = Qt::gray;
    nodeColors[NodeState::Open]    = Qt::yellow;
    nodeColors[NodeState::Closed]  = Qt::red;
    nodeColors[NodeState::Path]    = Qt::green;
    nodeColors[NodeState::Start]   = Qt::blue;
    nodeColors[NodeState::Goal]    = Qt::magenta;

    edgeColors[EdgeState::Default] = Qt::gray;
    edgeColors[EdgeState::Open] = Qt::yellow;
    edgeColors[EdgeState::Closed] = Qt::red;
    edgeColors[EdgeState::Path] = Qt::green;

    setMinimumWidth(minSize.x());
    setMinimumHeight(minSize.y());
}

void GraphVisualizer::updateGraphData(const std::vector<mlbInfo>& teams, const std::vector<stadiumDistances> dist)
{
    clearData();

    // Build nodes
    for (const auto& team : teams)
    {
        Node newNode;
        newNode.stadiumName = QString::fromStdString(team.stadiumName);

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
    for (Edge& edge : edges)
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
        QPen pen(edgeColors[EdgeState::Default]);
        pen.setWidth(edgeWidth);
        auto line = scene->addLine(QLineF(start, end), pen);
        line->setOpacity(0.3);

        // Create edge visual wrapper
        EdgeVisual* edgeVisual = new EdgeVisual;
        edgeVisual->edge = &edge;
        edgeVisual->state = EdgeState::Default;
        edgeVisual->line = line;

        // Store in map
        QString key = getEdgeKey(edge.from, edge.to);
        edgeItems[key] = edgeVisual;
    }

    // Create the node graphic objects
    for (Node& node : nodes)
    {
        // Offset needed as ellipses (nodes) are printed from top left
        float offset = nodeSize / 2.0;

        auto item = scene->addEllipse(
            node.pos.x() - offset, node.pos.y() - offset,
            nodeSize, nodeSize,
            QPen(Qt::black),
            QBrush(nodeColors[NodeState::Default]));

        // nodeItems[node.stadiumName] = item;

        // Create node label
        auto text = scene->addText(node.stadiumName);
        text->setDefaultTextColor(Qt::white);

        // Center text under node
        QRectF textRect = text->boundingRect();
        text->setPos(node.pos.x() - textRect.width() / 2.0,
                     node.pos.y() + offset);

        QFont font;
        font.setPointSize(20);
        text->setFont(font);

        // Create node visual wrapper struct
        NodeVisual* nodeVisual = new NodeVisual;
        nodeVisual->node = &node;
        nodeVisual->state = NodeState::Default;
        nodeVisual->ellipse = item;
        nodeVisual->label = text;

        // Store in node map
        nodeItems[node.stadiumName] = nodeVisual;
    }

    // Set scene to fit nodes and edges
    scene->setSceneRect(scene->itemsBoundingRect());
    fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

// Slots
void GraphVisualizer::playActions(const GraphActionBuffer& buffer, int delay)
{
    auto actions = buffer.getActionBuffer();

    QTimer* timer = new QTimer(this);


    // Allocate counter on the heap not sure why but it doesnt work as int
    int* actionIndex = new int(0);

    // Connect timer to lambda
    QObject::connect(timer, &QTimer::timeout, this, [this, timer, actions, actionIndex]()
    {
        if (*actionIndex >= actions.size())
        {
            timer->stop();
            timer->deleteLater();
            delete actionIndex;
            return;
        }

        doAction(actions[*actionIndex]);
        *actionIndex += 1;
    });

    // Start timer
    timer->start(delay);
}

void GraphVisualizer::rerollGraph()
{
    loadGraph();
}

void GraphVisualizer::resetGraph()
{
    // Recolor nodes
    for (auto& node : nodes)
    {
        setNodeState(node.stadiumName, NodeState::Default);
    }

    // Recolor edges
    for (auto &edge : edges)
    {
        setEdgeState(edge.from, edge.to, EdgeState::Default);
    }
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
    int iterations = 20;
    float repulsion = 1500;
    float attraction = 0.04;
    float damping = 0.75;

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

QString GraphVisualizer::getEdgeKey(const QString& from, const QString& to)
{
    if (from.isEmpty() || to.isEmpty())
    {
        qDebug() << "Either from or to is empty returning nothing";
        return "";
    }

    QString key = from;
    key += "<->";
    key += to;

    return key;
}

void GraphVisualizer::clearData()
{
    nodes.clear();
    edges.clear();
    nodeItems.clear();
    edgeItems.clear();
}

void GraphVisualizer::setNodeState(const QString& stadiumName, NodeState state)
{
    if (!nodeItems.contains(stadiumName))
    {
        return;
    }

    NodeVisual* nv = nodeItems[stadiumName];

    // update stored object
    nv->state = state;

    // update rendering
    nv->ellipse->setBrush(QBrush(nodeColors[state]));
}

void GraphVisualizer::setEdgeState(const QString& from, const QString& to, EdgeState state)
{
    QString edgeKey = getEdgeKey(from, to);

    if (!edgeItems.contains(edgeKey))
    {
        return;
    }

    EdgeVisual* ev = edgeItems[edgeKey];

    ev->state = state;

    QPen pen = edgeColors[state];
    pen.setWidth(5);
    ev->line->setPen(pen);

    ev->line->setOpacity(1);
}

void GraphVisualizer::doAction(const Action& action)
{
    switch (action.type)
    {
    case ActionType::NodeStart:
        setNodeState(action.node, NodeState::Start);
        break;

    case ActionType::NodeGoal:
        setNodeState(action.node, NodeState::Goal);
        break;

    case ActionType::NodeOpen:
        setNodeState(action.node, NodeState::Open);
        break;

    case ActionType::NodeClosed:
        setNodeState(action.node, NodeState::Closed);
        break;

    case ActionType::NodePath:
        setNodeState(action.node, NodeState::Path);
        break;

    case ActionType::EdgeOpen:
        setEdgeState(action.from, action.to, EdgeState::Open);
        break;

    case ActionType::EdgeClosed:
        setEdgeState(action.from, action.to, EdgeState::Closed);
        break;

    case ActionType::EdgePath:
        setEdgeState(action.from, action.to, EdgeState::Path);
        break;

    case ActionType::ClearMap:
        resetGraph();
        break;
    }
}
