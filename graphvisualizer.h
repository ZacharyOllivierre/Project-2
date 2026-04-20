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
    QString teamName;

    QPointF pos;
    NodeState state = NodeState::Default;
};

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
    explicit GraphVisualizer(QWidget* parent = nullptr);

    void loadGraph(const std::vector<Node> &nodes, const std::vector<Edge>& edges);
    void setNodeState(const QString& teamName, NodeState state);

public slots:

    void setNodeOpen(const QString& id);

    void setNodeClosed(const QString& id);

    void setPath(const QStringList& path);

private:
    QGraphicsScene *scene;

    // Holds nodes (circles) accessed by team name
    QMap<QString, QGraphicsEllipseItem*> nodeItems;

    // Hodls edges of nodes
    QVector<QGraphicsLineItem*> edgeItems;

    // Maps node state to color
    QHash <NodeState, QColor> colors;

    void computeLayout(std::vector<Node>& nodes, const std::vector<Edge>& edges);
};

#endif // GRAPHVISUALIZER_H
