#include "dfs.h"
#include <algorithm>

int DFSGraph::getOrAddNode(const std::string& name)
{
    auto it = nameToIdx.find(name);
    if (it != nameToIdx.end()) return it->second;

    int idx = (int)idxToName.size();
    nameToIdx[name] = idx;
    idxToName.push_back(name);
    adjList.push_back({});
    return idx;
}

void DFSGraph::addUndirectedEdge(int u, int v, int w)
{
    if (u == v) return;

    bool foundUV = false;
    for (auto& e : adjList[u])
    {
        if (e.first == v) { foundUV = true; if (w < e.second) e.second = w; break; }
    }
    if (!foundUV) adjList[u].push_back({v, w});

    bool foundVU = false;
    for (auto& e : adjList[v])
    {
        if (e.first == u) { foundVU = true; if (w < e.second) e.second = w; break; }
    }
    if (!foundVU) adjList[v].push_back({u, w});
}

void DFSGraph::sortAdjacency()
{
    for (auto& neighbors : adjList)
    {
        std::sort(neighbors.begin(), neighbors.end(),
                  [](const std::pair<int,int>& a, const std::pair<int,int>& b)
                  {
                      if (a.second != b.second) return a.second < b.second;
                      return a.first < b.first;
                  });
    }
}

void DFSGraph::buildFromDistances(const std::vector<stadiumDistances>& dist)
{
    nameToIdx.clear();
    idxToName.clear();
    adjList.clear();

    for (const auto& sd : dist)
    {
        int u = getOrAddNode(sd.originatedStadium);
        int v = getOrAddNode(sd.destinationStadium);
        addUndirectedEdge(u, v, sd.distance);
    }

    sortAdjacency();
}

void DFSGraph::dfsRecursive(int u, std::vector<bool>& visited, DFSResult& result)
{
    visited[u] = true;

    for (const auto& edge : adjList[u])
    {
        int v = edge.first;
        int w = edge.second;

        if (!visited[v])
        {
            result.visitOrder.push_back(DFSEdge(idxToName[u], idxToName[v], w));
            result.totalMileage += w;
            dfsRecursive(v, visited, result);
        }
    }
}

DFSResult DFSGraph::performDFS(const std::string& start)
{
    DFSResult result;
    result.startStadium = start;

    auto it = nameToIdx.find(start);
    if (it == nameToIdx.end()) return result;

    int n = (int)idxToName.size();
    std::vector<bool> visited(n, false);

    // First "edge" records the start node with distance 0.
    result.visitOrder.push_back(DFSEdge("", start, 0));
    dfsRecursive(it->second, visited, result);

    return result;
}

DFSResult DFSGraph::performDFSReportFromOraclePark()
{
    return performDFS("Oracle Park");
}

void DFSGraph::printResult(const DFSResult& r) const
{
    cout << "===== DFS Traversal Report =====\n";
    cout << "Starting stadium: " << r.startStadium << "\n";
    cout << "Traversal order (depth-first, shortest neighbor first):\n";

    for (int i = 0; i < (int)r.visitOrder.size(); i++)
    {
        const DFSEdge& e = r.visitOrder[i];
        if (i == 0)
        {
            cout << "  " << (i + 1) << ". " << e.to << "  (start)\n";
        }
        else
        {
            cout << "  " << (i + 1) << ". " << e.from << " -> " << e.to
                 << "  (" << e.distance << " mi)\n";
        }
    }

    cout << "Total associated traversal mileage: " << r.totalMileage << "\n";
    cout << "================================\n";
}
