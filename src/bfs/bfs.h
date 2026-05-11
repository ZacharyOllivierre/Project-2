#pragma once
#include "../Database.h"
#include <unordered_map>

struct BFSEdge
{
    std::string from;
    std::string to;
    int         distance;

    BFSEdge() : from(""), to(""), distance(0) {}
    BFSEdge(const std::string& f, const std::string& t, int d)
        : from(f), to(t), distance(d) {}
};

struct BFSResult
{
    std::string          startStadium;
    std::vector<BFSEdge> visitOrder;
    int                  totalMileage;

    BFSResult() : startStadium(""), visitOrder(), totalMileage(0) {}
};

class BFSGraph
{
public:
    void      buildFromDistances(const std::vector<stadiumDistances>& dist);
    BFSResult performBFS(const std::string& start);
    void      printResult(const BFSResult& r) const;

    // Report-style entry point: starts at Target Field / Minnesota Twins.
    BFSResult performBFSReportFromTargetField();

    int  nodeCount() const { return (int)idxToName.size(); }
    bool containsStadium(const std::string& name) const { return nameToIdx.find(name) != nameToIdx.end(); }

private:
    std::unordered_map<std::string, int>            nameToIdx;
    std::vector<std::string>                        idxToName;
    std::vector<std::vector<std::pair<int,int>>>    adjList;   // {neighbor_idx, weight}

    int  getOrAddNode(const std::string& name);
    void addUndirectedEdge(int u, int v, int w);
    void sortAdjacency();
};
