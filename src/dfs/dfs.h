#pragma once
#include "../Database.h"
#include <unordered_map>

struct DFSEdge
{
    std::string from;
    std::string to;
    int         distance;

    DFSEdge() : from(""), to(""), distance(0) {}
    DFSEdge(const std::string& f, const std::string& t, int d)
        : from(f), to(t), distance(d) {}
};

struct DFSResult
{
    std::string          startStadium;
    std::vector<DFSEdge> visitOrder;
    int                  totalMileage;

    DFSResult() : startStadium(""), visitOrder(), totalMileage(0) {}
};

class DFSGraph
{
public:
    void      buildFromDistances(const std::vector<stadiumDistances>& dist);
    DFSResult performDFS(const std::string& start);
    void      printResult(const DFSResult& r) const;

    // Report-style entry point: starts at Oracle Park / San Francisco Giants.
    DFSResult performDFSReportFromOraclePark();

    int  nodeCount() const { return (int)idxToName.size(); }
    bool containsStadium(const std::string& name) const { return nameToIdx.find(name) != nameToIdx.end(); }

private:
    std::unordered_map<std::string, int>            nameToIdx;
    std::vector<std::string>                        idxToName;
    std::vector<std::vector<std::pair<int,int>>>    adjList;   // {neighbor_idx, weight}

    int  getOrAddNode(const std::string& name);
    void addUndirectedEdge(int u, int v, int w);
    void sortAdjacency();
    void dfsRecursive(int u, std::vector<bool>& visited, DFSResult& result);
};
