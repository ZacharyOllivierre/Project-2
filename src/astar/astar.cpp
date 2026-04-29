#include "astar.h"
#include <algorithm>

int StadiumGraph::getOrAddNode(const std::string& name)
{
    auto it = nameToIdx.find(name);
    if (it != nameToIdx.end()) return it->second;

    int idx = (int)idxToName.size();
    nameToIdx[name] = idx;
    idxToName.push_back(name);
    adjList.push_back({});
    hCost.push_back(0);
    return idx;
}

void StadiumGraph::buildFromDistances(const std::vector<stadiumDistances>& dist)
{
    nameToIdx.clear();
    idxToName.clear();
    adjList.clear();
    hCost.clear();

    for (const auto& sd : dist) {
        int u = getOrAddNode(sd.originatedStadium);
        int v = getOrAddNode(sd.destinationStadium);
        adjList[u].push_back({v, sd.distance});
    }
}

void StadiumGraph::setHeuristic(const std::string& name, int h)
{
    auto it = nameToIdx.find(name);
    if (it != nameToIdx.end())
        hCost[it->second] = h;
}

void StadiumGraph::relaxEdge(int goalIdx)
{
    // Pick the unvisited node with the lowest f = g + h.
    int minF = INF;
    int u    = -1;

    for (int i = 0; i < (int)gCost.size(); i++) {
        if (!closed[i] && gCost[i] != INF) {
            int f = gCost[i] + hCost[i];
            if (f < minF) {
                minF = f;
                u    = i;
            }
        }
    }

    if (u == -1) return;

    if (u == goalIdx) {
        closed[u] = true;
        return;
    }

    for (auto& edge : adjList[u]) {
        int v = edge.first;
        int w = edge.second;
        if (gCost[u] + w < gCost[v]) {
            gCost[v]  = gCost[u] + w;
            parent[v] = u;
        }
    }

    closed[u] = true;
}

int StadiumGraph::runAStar(const std::string& start, const std::string& goal)
{
    int n = (int)idxToName.size();

    gCost.assign(n, INF);
    parent.assign(n, -1);
    closed.assign(n, false);

    auto startIt = nameToIdx.find(start);
    auto goalIt  = nameToIdx.find(goal);
    if (startIt == nameToIdx.end() || goalIt == nameToIdx.end()) return -1;

    gCost[startIt->second] = 0;
    int goalIdx = goalIt->second;

    while (!closed[goalIdx]) {
        relaxEdge(goalIdx);

        // Guard against disconnected graph (no reachable open nodes remain).
        if (!closed[goalIdx]) {
            bool anyOpen = false;
            for (int i = 0; i < n; i++) {
                if (!closed[i] && gCost[i] != INF) { anyOpen = true; break; }
            }
            if (!anyOpen) break;
        }
    }

    return closed[goalIdx] ? gCost[goalIdx] : -1;
}

std::vector<std::string> StadiumGraph::getPath(const std::string& goal) const
{
    auto it = nameToIdx.find(goal);
    if (it == nameToIdx.end()) return {};

    std::vector<std::string> path;
    int current = it->second;

    while (current != -1) {
        path.push_back(idxToName[current]);
        current = parent[current];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void StadiumGraph::printPath(const std::string& start, const std::string& goal) const
{
    auto pathVec = getPath(goal);
    if (pathVec.empty()) {
        cout << "[A*] No path from " << start << " to " << goal << "\n";
        return;
    }

    for (int i = 0; i < (int)pathVec.size(); i++) {
        cout << pathVec[i];
        if (i < (int)pathVec.size() - 1) cout << " -> ";
    }

    auto it = nameToIdx.find(goal);
    cout << " | Cost: " << gCost[it->second] << "\n";
}
