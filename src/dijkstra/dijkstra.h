#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <QVector>
#include <QPair>
#include <QString>
#include <limits>
#include <QDebug>

#include "../graph/graphactionbuffer.h"
#include "../database/database.h"

struct EdgeD
{
    QString destination;
    int distance;
};

struct PathInfo
{
    QString destination;
    int pathDistance;
    QVector<QString> path;
};

class Dijkstra
{
public:
    Dijkstra();

    void loadTeams(const vector<mlbInfo>& teams, const vector<stadiumDistances> dist);

    QVector<PathInfo> shortestPath(const QString& start, bool debugPrint = false);

    inline GraphActionBuffer* getActionBuffer() {return &buffer;}

    void printDebug(const QVector<int>& d, const QVector<int>& p,
                    const QVector<PathInfo> res, const QString& start);

private:
    QVector<QVector<EdgeD>> graph;
    QVector<QString> teamNames;

    GraphActionBuffer buffer;

    int getTeamIndex(const QString& stadium) const;
};

#endif // DIJKSTRA_H

