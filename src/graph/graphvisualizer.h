#ifndef GRAPHVISUALIZER_H
#define GRAPHVISUALIZER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QMap>
#include <QColor>
#include <QString>
#include <QPointF>
#include <vector>
#include <QRandomGenerator>
#include <QTimer>

#include "../database/database.h"
#include "graphactionbuffer.h"

enum class NodeState
{
    Default,
    Open,
    Closed,
    Path,
    Start,
    Goal
};

enum class EdgeState
{
    Default,
    Open,
    Closed,
    Path
};

struct Node
{
    QString stadiumName;
    QPointF pos;
};

struct Edge
{
    QString from;
    QString to;
    double weight;
};

struct NodeVisual
{
    Node* node;

    NodeState state = NodeState::Default;

    QGraphicsEllipseItem* ellipse = nullptr;
    QGraphicsTextItem* label = nullptr;
};

struct EdgeVisual
{
    Edge* edge;

    EdgeState state = EdgeState::Default;
    QGraphicsLineItem* line = nullptr;
};


class GraphVisualizer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphVisualizer(QPointF minSize, QWidget* parent = nullptr);

    void updateGraphData(const std::vector<mlbInfo>& teams,
                        const std::vector<stadiumDistances> dist);

    void loadGraph(int nodeSize = 30, int edgeWidth = 5);

public slots:
    // Slot that accepts action buffer and plays it with staggering delay
    void playActions(const GraphActionBuffer& buffer, int delay = 50);

    // Rerolls generation of graph visual, resets vals to default size
    void rerollGraph();

    // Recolors and resets states
    void resetGraph();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene *scene;

    std::vector<Node> nodes;
    std::vector<Edge> edges;

    // Holds visual objects, accessed by stadium name
    QMap<QString, NodeVisual*> nodeItems;
    // Holds edge visual objects, accessed by to<->from key
    QMap<QString, EdgeVisual*> edgeItems;

    // Maps node state to color
    QHash <NodeState, QColor> nodeColors;
    QHash <EdgeState, QColor> edgeColors;

    // Updates default positions of nodes
    void computeLayout();

    // Scales map to fit within bounds
    void fitGraphToView(float margin);

    // Gives key for edge items
    QString getEdgeKey(const QString& from, const QString& to);

    void clearData();

    void setNodeState(const QString& stadiumName, NodeState state);
    void setEdgeState(const QString& from, const QString& to, EdgeState state);
    void doAction(const Action& action);
};

#endif // GRAPHVISUALIZER_H