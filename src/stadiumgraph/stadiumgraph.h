/**
 * @file stadiumgraph.h
 * @brief Declares the graph edge structure and StadiumGraph class used for stadium connections and MST calculations.
 */

#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QSet>

/**
 * One weighted connection between two stadiums.
 */
struct GraphEdge
{
    QString from;
    QString to;
    int distance;
};

/**
 * StadiumGraph
 * Stores stadium distances and runs graph algorithms.
 */
class StadiumGraph
{
public:
    /**
     * Add an undirected edge between two stadiums.
     */
    void addEdge(const QString& from,
                 const QString& to,
                 int distance);

    /**
     * Build the minimum spanning tree using Prim's algorithm.
     */
    QList<GraphEdge> primMST(const QString& startStadium) const;

    /**
     * Return total mileage for a list of MST edges.
     */
    int getTotalMileage(const QList<GraphEdge>& mstEdges) const;

    /**
     * Return all stadium names in the graph.
     */
    QStringList getAllStadiums() const;

private:
    // stadium name -> connected edges
    QMap<QString, QList<GraphEdge>> m_adjacencyList;
};
