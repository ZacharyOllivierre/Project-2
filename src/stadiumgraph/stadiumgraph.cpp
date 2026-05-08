/**
 * @file stadiumgraph.cpp
 * @brief Implements the stadium graph and Prim's algorithm for calculating the minimum spanning tree mileage.
 */

#include "stadiumgraph.h"

#include <limits>

/**
 * Add an undirected connection.
 * The edge is stored in both directions.
 */
void StadiumGraph::addEdge(const QString& from,
                           const QString& to,
                           int distance)
{
    if (from.isEmpty() || to.isEmpty() || distance <= 0)
    {
        return;
    }

    GraphEdge forwardEdge;
    forwardEdge.from = from;
    forwardEdge.to = to;
    forwardEdge.distance = distance;

    GraphEdge reverseEdge;
    reverseEdge.from = to;
    reverseEdge.to = from;
    reverseEdge.distance = distance;

    m_adjacencyList[from].append(forwardEdge);
    m_adjacencyList[to].append(reverseEdge);
}

/**
 * Prim's MST algorithm.
 * Repeatedly chooses the shortest edge from visited to unvisited.
 */
QList<GraphEdge> StadiumGraph::primMST(const QString& startStadium) const
{
    QList<GraphEdge> mstEdges;

    if (!m_adjacencyList.contains(startStadium))
    {
        return mstEdges;
    }

    QSet<QString> visited;
    visited.insert(startStadium);

            // Keep adding edges until all stadiums are visited
while (visited.size() < m_adjacencyList.size())
    {
        GraphEdge bestEdge;
        int bestDistance = std::numeric_limits<int>::max();
        bool foundEdge = false;

                    // Look at every visited stadium for the next best edge
for (const QString& stadium : visited)
        {
            const QList<GraphEdge>& edges = m_adjacencyList.value(stadium);

            for (const GraphEdge& edge : edges)
            {
                if (!visited.contains(edge.to) && edge.distance < bestDistance)
                {
                    bestEdge = edge;
                    bestDistance = edge.distance;
                    foundEdge = true;
                }
            }
        }

        if (!foundEdge)
        {
            break;
        }

        mstEdges.append(bestEdge);
        visited.insert(bestEdge.to);
    }

    return mstEdges;
}

/**
 * Add all MST edge distances.
 */
int StadiumGraph::getTotalMileage(const QList<GraphEdge>& mstEdges) const
{
    int total = 0;

    for (const GraphEdge& edge : mstEdges)
    {
        total += edge.distance;
    }

    return total;
}

/**
 * Return all stadium names currently stored.
 */
QStringList StadiumGraph::getAllStadiums() const
{
    return m_adjacencyList.keys();
}
