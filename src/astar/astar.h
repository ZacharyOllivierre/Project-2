#pragma once
#include "../database/database.h"
#include <unordered_map>

class StadiumGraph {
public:
    void buildFromDistances(const std::vector<stadiumDistances>& dist);
    void setHeuristic(const std::string& name, int h);

    // Returns total cost, or -1 if goal is unreachable.
    int runAStar(const std::string& start, const std::string& goal);

    std::vector<std::string> getPath(const std::string& goal) const;
    void printPath(const std::string& start, const std::string& goal) const;

private:
    static constexpr int INF = 9999999;

    std::unordered_map<std::string, int>         nameToIdx;
    std::vector<std::string>                     idxToName;
    std::vector<std::vector<std::pair<int,int>>> adjList;   // {neighbor_idx, weight}
    std::vector<int>                             hCost;     // h(n); 0 by default

    // Per-search state — reset at the start of each runAStar call.
    std::vector<int>  gCost;
    std::vector<int>  parent;
    std::vector<bool> closed;

    int  getOrAddNode(const std::string& name);
    void relaxEdge(int goalIdx);
};
