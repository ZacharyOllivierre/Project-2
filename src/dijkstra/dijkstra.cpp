#include "dijkstra.h"

Dijkstra::Dijkstra() {}

// Note it is NOT team name, still need to switch variable names to stadium

void Dijkstra::loadTeams(const vector<mlbInfo>& teams,
                         const vector<stadiumDistances> dist)
{
    graph.clear();
    teamNames.clear();

    // Init
    for (auto& team : teams)
    {
        QString sn = QString::fromStdString(team.stadiumName).trimmed();
        if (sn == "Minute Maid Park") sn = "Daikin Park";
        teamNames.push_back(sn);
        graph.push_back(QVector<EdgeD>());
    }

    // Add edges
    for (auto & d : dist)
    {
        QString origStadium = QString::fromStdString(d.originatedStadium).trimmed();
        QString destStadium = QString::fromStdString(d.destinationStadium).trimmed();
        if (origStadium == "Minute Maid Park") origStadium = "Daikin Park";
        if (destStadium == "Minute Maid Park") destStadium = "Daikin Park";
        int fromIndx = getTeamIndex(origStadium);
        int toIndx   = getTeamIndex(destStadium);

        // Safety net if to or from is not found
        if (fromIndx == -1 || toIndx == -1)
        {
            continue;
        }

        // Add edges to both sides
        EdgeD edge;
        edge.destination = destStadium;
        edge.distance = d.distance;

        graph[fromIndx].push_back(edge);

        EdgeD oppositeEdge;
        oppositeEdge.destination = origStadium;
        oppositeEdge.distance = d.distance;

        graph[toIndx].push_back(oppositeEdge);
    }
}

QVector<PathInfo> Dijkstra::shortestPath(const QString& start, bool debugPrint)
{
    // Representation of infinity
    const int INF = std::numeric_limits<int>::max();
    QVector<PathInfo> results;

    int startIndex = getTeamIndex(start);

    // Early out if start not found
    if (startIndex == -1)
    {
        qDebug() << "early out start not found\n";
        return results;
    }

    int n = teamNames.size();

    // Tracker vars
    QVector<int> distances(n, INF);
    QVector<bool> visited(n, false);
    QVector<int> previous(n, -1);

    distances[startIndex] = 0;

    // Buffer action start node
    buffer.setNodeStart(start);

    // Main loop
    for (int i = 0; i < n; i++)
    {
        int current = -1;
        int bestDistance = INF;

        // Find closest unvisited node
        for (int j = 0; j < n; j++)
        {
            if (!visited[j] && distances[j] < bestDistance)
            {
                bestDistance = distances[j];
                current = j;
            }
        }

        // None found
        if (current == -1)
        {
            break;
        }

        visited[current] = true;

        // Buffer action location closed
        buffer.setNodeClosed(teamNames[current]);

        // Relax edges
        for (EdgeD& edge : graph[current])
        {
            int neighbor = getTeamIndex(edge.destination);

            if (neighbor == -1)
            {
                continue;
            }

            int newDist = distances[current] + edge.distance;

            // Buffer action
            buffer.setEdgeOpen(teamNames[current], edge.destination);
            // buffer.setEdgeClosed(teamNames[current], edge.destination);
            buffer.setNodeOpen(edge.destination);

            if (newDist < distances[neighbor])
            {
                distances[neighbor] = newDist;
                previous[neighbor] = current;
            }

        }
    }

    // Put together results
    for (int i = 0; i < n; i++)
    {
        PathInfo info = {teamNames[i], distances[i]};

        QVector<QString> reversePath;
        int current = i;

        while (current != -1)
        {
            reversePath.push_back(teamNames[current]);
            current = previous[current];
        }

        // Reverse reverse path
        for (int j = reversePath.size() - 1; j >= 0; j--)
        {
            info.path.push_back(reversePath[j]);
        }

        buffer.setNodeStart(start);

        results.push_back(info);
    }

    // Call buffer actions to show each final path
    buffer.clearMap();
    for (auto& result : results)
    {
        buffer.setNodeGoal(start);
        buffer.setNodeStart(result.destination);

        const auto& path = result.path;
        for (int i = 1; i < path.size(); i++)
        {
            buffer.setEdgePath(path[i - 1], path[i]);
            buffer.setEdgePath(path[i], path[i - 1]);

            buffer.setNodePath(path[i]);
        }
        buffer.clearMap();
    }

    if (debugPrint)
    {
        printDebug(distances, previous, results, start);
    }

    buffer.clearMap();

    return results;
}

void Dijkstra::printDebug(const QVector<int>& d, const QVector<int>& p,
                          const QVector<PathInfo> res, const QString& start)
{
    qDebug() << "=== Dijkstra Debug ===";
    qDebug() << "Start node:" << start;
    qDebug() << "\nNode distances and previous nodes:";

    for (int i = 0; i < teamNames.size(); i++)
    {
        QString prevNode = (p[i] == -1) ? "None" : teamNames[p[i]];
        QString distStr = (d[i] == std::numeric_limits<int>::max()) ? "INF" : QString::number(d[i]);
        qDebug() << teamNames[i] << "-> Distance:" << distStr << ", Previous:" << prevNode;
    }

    qDebug() << "\nReconstructed paths from start:";

    for (const PathInfo& info : res)
    {
        QString pathStr = info.path.join(" -> ");
        QString distStr = (info.pathDistance == std::numeric_limits<int>::max()) ? "INF" : QString::number(info.pathDistance);
        qDebug() << "To" << info.destination << ": Distance =" << distStr << ", Path =" << pathStr;
    }

    qDebug() << "=====================\n";
}

int Dijkstra::getTeamIndex(const QString& stadium) const
{
    for (int i = 0; i < teamNames.size(); i++)
    {
        if (teamNames[i] == stadium.trimmed())
        {
            return i;
        }
    }

    qDebug() << "Stadium " << stadium << " index not found returning -1";
    return -1;
}