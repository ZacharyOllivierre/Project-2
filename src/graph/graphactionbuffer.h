#ifndef GRAPHACTIONBUFFER_H
#define GRAPHACTIONBUFFER_H

#include <QString>
#include <QVector>

/* Traversals and pathing algorithms use action buffer to build a vec of
 * desired map actions. The buffer is then given to visualizer to show
 * To refer to a node (Stadium Name)
 * An edge (to and from Stadium Names)*/

enum class ActionType
{
    NodeStart,
    NodeGoal,

    NodeOpen,
    NodeClosed,
    NodePath,

    EdgeOpen,
    EdgeClosed,
    EdgePath,

    ClearMap
};

struct Action
{
    ActionType type;

    QString node;

    QString from;
    QString to;
};

class GraphActionBuffer
{
private:
    QVector<Action> actionBuffer;

public:
    GraphActionBuffer();

    inline const QVector<Action>& getActionBuffer() const
    {
        return actionBuffer;
    }

    // Generic clear and add action to buffer
    void clear();
    void addAction(Action action);

    // Node actions
    void setNodeStart(const QString& node);
    void setNodeGoal(const QString& node);
    void setNodeOpen(const QString& node);
    void setNodeClosed(const QString& node);
    void setNodePath(const QString& node);

    // Edge actions
    void setEdgeOpen(const QString& from, const QString& to);
    void setEdgeClosed(const QString& from, const QString& to);
    void setEdgePath(const QString& from, const QString& to);

    void clearMap();
};

#endif // GRAPHACTIONBUFFER_H
