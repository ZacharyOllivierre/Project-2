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

#include "../database/database.h"

/*The map can be updated live by connecting an emitting signal to these slots
 For example:

Pathing class emits nodeOpened signal
    emit nodeOpened(currentNodeName);

Outside of both classes connect signals
    connect(astar, &AStar::nodeOpened, graphVisualizer, &GraphVisualizer::setNodeOpen);
*/

enum class NodeState
{
    Default,
    Open,
    Closed,
    Path,
    Start,
    Goal
};

struct Node
{
    // Unique identifier
    QString stadiumName;

    QPointF pos;
    NodeState state = NodeState::Default;
};

// Change from and to to Node pointers
struct Edge
{
    QString from;
    QString to;
    double weight;
};

class GraphVisualizer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphVisualizer(QPointF minSize, QWidget* parent = nullptr);

    void updateGraphData(const std::vector<mlbInfo>& teams,
                        const std::vector<stadiumDistances> dist);

    void loadGraph(int nodeSize = 20, int edgeWidth = 2);

public slots:
    void setNodeStart(const QString& stadiumName);
    void setNodeEnd(const QString& stadiumName);

    void setNodeOpen(const QString& stadiumName);

    void setNodeClosed(const QString& stadiumName);

    // Can set path by QStringList of stadium names or mlbInfo vec
    void setPath(const QStringList& pathStadiumNames);
    void setPath(const std::vector<mlbInfo> pathMlbInfo);

    // Sets one node to path state
    void setPath(const QString& stadiumName);

    // Rerolls generation of graph visual, resets vals to default size
    void rerollGraph();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsScene *scene;

    std::vector<Node> nodes;
    std::vector<Edge> edges;

    // Holds nodes (circles) accessed by stadiumName
    QMap<QString, QGraphicsEllipseItem*> nodeItems;

    // Hodls edges of nodes
    QVector<QGraphicsLineItem*> edgeItems;

    // Maps node state to color
    QHash <NodeState, QColor> colors;

    // Updates default positions of nodes
    void computeLayout();

    // Scales map to fit within bounds
    void fitGraphToView(float margin);

    void setNodeState(const QString& stadiumName, NodeState state);
};

#endif // GRAPHVISUALIZER_H
