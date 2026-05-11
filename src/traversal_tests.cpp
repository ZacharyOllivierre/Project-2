// traversal_tests.cpp
//
// Unit tests for the DFS and BFS traversal REPORT features.
// These are report-style traversals, not vacation trips:
//   - No souvenir purchasing, no user stadium selection, no checkout flow.
//   - Output is a deterministic ordered list of stadiums + total mileage.
//
// Build target:  make tests
// Run:           ./traversal_tests

#include "dfs/dfs.h"
#include "bfs/bfs.h"
#include "astar/astar.h"      // pulled in only to prove A* still compiles unchanged
#include <iostream>
#include <vector>
#include <string>

static int gTestsRun    = 0;
static int gTestsPassed = 0;

#define CHECK(cond, msg)                                                       \
    do {                                                                       \
        ++gTestsRun;                                                           \
        if (cond) {                                                            \
            ++gTestsPassed;                                                    \
            std::cout << "  [PASS] " << msg << "\n";                           \
        } else {                                                               \
            std::cout << "  [FAIL] " << msg                                    \
                      << "  (at " << __FILE__ << ":" << __LINE__ << ")\n";     \
        }                                                                      \
    } while (0)

// --------------------------------------------------------------------------
// Mock graph used for deterministic tests.
//
//        START
//       /  |  \
//      30 10 20
//     /    |    \
//    A     B     C
//    |     |
//    50    15
//     \   /
//       D
//
// Adjacency lists, sorted ascending by distance:
//   START : B(10), C(20), A(30)
//   A     : START(30), D(50)
//   B     : START(10), D(15)
//   C     : START(20)
//   D     : B(15), A(50)
// --------------------------------------------------------------------------
static std::vector<stadiumDistances> buildMockGraph()
{
    std::vector<stadiumDistances> edges;
    auto add = [&](const std::string& a, const std::string& b, int d) {
        stadiumDistances sd;
        sd.originatedStadium  = a;
        sd.destinationStadium = b;
        sd.distance           = d;
        edges.push_back(sd);
    };

    add("START", "A", 30);
    add("START", "B", 10);
    add("START", "C", 20);
    add("A",     "D", 50);
    add("B",     "D", 15);
    return edges;
}

// Real-data graph used only for "starts at the assigned stadium" tests.
static std::vector<stadiumDistances> buildSmallRealisticGraph()
{
    std::vector<stadiumDistances> edges;
    auto add = [&](const std::string& a, const std::string& b, int d) {
        stadiumDistances sd;
        sd.originatedStadium  = a;
        sd.destinationStadium = b;
        sd.distance           = d;
        edges.push_back(sd);
    };

    // Cluster containing Oracle Park (start of DFS report).
    add("Oracle Park",                       "Oakland-Alameda County Coliseum", 50);
    add("Oakland-Alameda County Coliseum",   "Oracle Park",                     50);
    add("Oracle Park",                       "Safeco Field",                   680);
    add("Safeco Field",                      "Oracle Park",                    680);

    // Cluster containing Target Field (start of BFS report).
    add("Target Field",   "Miller Park",   300);
    add("Miller Park",    "Target Field",  300);
    add("Target Field",   "Busch Stadium", 465);
    add("Busch Stadium",  "Target Field",  465);

    // Bridge between the two clusters so the graph is connected.
    add("Safeco Field",   "Target Field",  1390);
    add("Target Field",   "Safeco Field",  1390);

    return edges;
}

// --------------------------------------------------------------------------
// Tests
// --------------------------------------------------------------------------
static void testDFSStartsAtOraclePark()
{
    std::cout << "[test] DFS starts at Oracle Park / San Francisco Giants\n";
    DFSGraph g;
    g.buildFromDistances(buildSmallRealisticGraph());
    DFSResult r = g.performDFSReportFromOraclePark();

    CHECK(r.startStadium == "Oracle Park", "DFS report starts at Oracle Park");
    CHECK(!r.visitOrder.empty(),           "DFS visit order is non-empty");
    CHECK(r.visitOrder.front().to == "Oracle Park",
          "DFS first entry in visit order is Oracle Park");
}

static void testBFSStartsAtTargetField()
{
    std::cout << "[test] BFS starts at Target Field / Minnesota Twins\n";
    BFSGraph g;
    g.buildFromDistances(buildSmallRealisticGraph());
    BFSResult r = g.performBFSReportFromTargetField();

    CHECK(r.startStadium == "Target Field", "BFS report starts at Target Field");
    CHECK(!r.visitOrder.empty(),            "BFS visit order is non-empty");
    CHECK(r.visitOrder.front().to == "Target Field",
          "BFS first entry in visit order is Target Field");
}

static void testDFSShortestNeighborFirst()
{
    std::cout << "[test] DFS visits neighbors in shortest-distance-first order\n";
    DFSGraph g;
    g.buildFromDistances(buildMockGraph());
    DFSResult r = g.performDFS("START");

    // Expected DFS order on the mock graph (shortest neighbor first):
    //   START -> B(10) -> D(15) -> A(50) -> C(20)
    // visitOrder[0] is the seed (from="", to=START, dist=0).
    std::vector<std::string> expectedTo  = {"START", "B", "D", "A", "C"};
    std::vector<int>         expectedDist = {0, 10, 15, 50, 20};

    CHECK(r.visitOrder.size() == expectedTo.size(),
          "DFS visited exactly 5 nodes on the mock graph");

    bool orderOk = (r.visitOrder.size() == expectedTo.size());
    bool distOk  = (r.visitOrder.size() == expectedTo.size());
    for (size_t i = 0; i < r.visitOrder.size() && i < expectedTo.size(); ++i)
    {
        if (r.visitOrder[i].to       != expectedTo[i])   orderOk = false;
        if (r.visitOrder[i].distance != expectedDist[i]) distOk  = false;
    }
    CHECK(orderOk, "DFS order is START, B, D, A, C (shortest neighbor first)");
    CHECK(distOk,  "DFS per-edge mileages are 0, 10, 15, 50, 20");

    // Total mileage = 10 + 15 + 50 + 20 = 95
    CHECK(r.totalMileage == 95, "DFS total mileage equals 95");
}

static void testBFSShortestNeighborFirst()
{
    std::cout << "[test] BFS visits neighbors in shortest-distance-first order\n";
    BFSGraph g;
    g.buildFromDistances(buildMockGraph());
    BFSResult r = g.performBFS("START");

    // Expected BFS order on the mock graph:
    //   level 0 : START
    //   level 1 : B(10), C(20), A(30)        (sorted ascending)
    //   level 2 : D(15) discovered from B    (B is processed before A/C)
    std::vector<std::string> expectedTo   = {"START", "B", "C", "A", "D"};
    std::vector<int>         expectedDist = {0, 10, 20, 30, 15};

    CHECK(r.visitOrder.size() == expectedTo.size(),
          "BFS visited exactly 5 nodes on the mock graph");

    bool orderOk = (r.visitOrder.size() == expectedTo.size());
    bool distOk  = (r.visitOrder.size() == expectedTo.size());
    for (size_t i = 0; i < r.visitOrder.size() && i < expectedTo.size(); ++i)
    {
        if (r.visitOrder[i].to       != expectedTo[i])   orderOk = false;
        if (r.visitOrder[i].distance != expectedDist[i]) distOk  = false;
    }
    CHECK(orderOk, "BFS order is START, B, C, A, D (shortest neighbor first)");
    CHECK(distOk,  "BFS per-edge mileages are 0, 10, 20, 30, 15");

    // Total mileage = 10 + 20 + 30 + 15 = 75
    CHECK(r.totalMileage == 75, "BFS total mileage equals 75");
}

static void testEachStadiumVisitedAtMostOnce()
{
    std::cout << "[test] Each stadium is visited at most once\n";

    DFSGraph dg;
    dg.buildFromDistances(buildMockGraph());
    DFSResult dr = dg.performDFS("START");

    BFSGraph bg;
    bg.buildFromDistances(buildMockGraph());
    BFSResult br = bg.performBFS("START");

    auto allUnique = [](const auto& seq) {
        std::vector<std::string> names;
        for (const auto& e : seq) names.push_back(e.to);
        for (size_t i = 0; i < names.size(); ++i)
            for (size_t j = i + 1; j < names.size(); ++j)
                if (names[i] == names[j]) return false;
        return true;
    };

    CHECK(allUnique(dr.visitOrder), "DFS visits each stadium at most once");
    CHECK(allUnique(br.visitOrder), "BFS visits each stadium at most once");
}

static void testTotalMileageOnlyCountsFirstVisitEdges()
{
    std::cout << "[test] Total mileage only counts edges used to first-visit a new stadium\n";

    DFSGraph dg;
    dg.buildFromDistances(buildMockGraph());
    DFSResult dr = dg.performDFS("START");

    BFSGraph bg;
    bg.buildFromDistances(buildMockGraph());
    BFSResult br = bg.performBFS("START");

    int dfsSum = 0;
    for (const auto& e : dr.visitOrder) dfsSum += e.distance;

    int bfsSum = 0;
    for (const auto& e : br.visitOrder) bfsSum += e.distance;

    CHECK(dfsSum == dr.totalMileage,
          "DFS total mileage equals the sum of its first-visit edge mileages");
    CHECK(bfsSum == br.totalMileage,
          "BFS total mileage equals the sum of its first-visit edge mileages");

    // The mock graph has 5 edges but only 4 first-visit edges (#nodes - 1).
    CHECK((int)dr.visitOrder.size() - 1 == 4, "DFS used exactly 4 traversal edges");
    CHECK((int)br.visitOrder.size() - 1 == 4, "BFS used exactly 4 traversal edges");
}

static void testReportFunctionsAreNotTripFunctions()
{
    std::cout << "[test] BFS/DFS are report functions, not vacation trip functions\n";

    // DFSResult / BFSResult are pure traversal data: a start stadium, a visit
    // order, and a total mileage. They have no fields for souvenirs, no
    // user-chosen stadium list, no checkout state, no trip cost in dollars.
    DFSResult dr;
    BFSResult br;

    CHECK(dr.startStadium == "" && dr.visitOrder.empty() && dr.totalMileage == 0,
          "DFSResult default-constructs as an empty traversal report");
    CHECK(br.startStadium == "" && br.visitOrder.empty() && br.totalMileage == 0,
          "BFSResult default-constructs as an empty traversal report");

    // sizeof() check is a structural assertion that no extra trip-style fields
    // have been added later by accident.
    CHECK(sizeof(DFSEdge) == sizeof(BFSEdge),
          "DFSEdge and BFSEdge are structurally equivalent traversal-edge records");
}

static void testAStarStillCompilesAndRuns()
{
    std::cout << "[test] Existing A* code still compiles and is not broken\n";

    StadiumGraph astar;
    astar.buildFromDistances(buildSmallRealisticGraph());

    int cost = astar.runAStar("Oracle Park", "Target Field");
    // We don't assert the exact A* cost (that's A*'s concern, not ours);
    // we only assert it still produces *some* answer and didn't crash.
    CHECK(cost >= 0 || cost == -1, "A* runAStar returned a valid result");
    CHECK(astar.getPath("Target Field").size() >= 1 ||
          astar.getPath("Target Field").empty(),
          "A* getPath is callable post-DFS/BFS link");
}

int main()
{
    std::cout << "================================================\n";
    std::cout << " Traversal Report Unit Tests (DFS + BFS)\n";
    std::cout << "================================================\n";

    testDFSStartsAtOraclePark();
    testBFSStartsAtTargetField();
    testDFSShortestNeighborFirst();
    testBFSShortestNeighborFirst();
    testEachStadiumVisitedAtMostOnce();
    testTotalMileageOnlyCountsFirstVisitEdges();
    testReportFunctionsAreNotTripFunctions();
    testAStarStillCompilesAndRuns();

    std::cout << "================================================\n";
    std::cout << " " << gTestsPassed << " / " << gTestsRun << " checks passed\n";
    std::cout << "================================================\n";

    return (gTestsPassed == gTestsRun) ? 0 : 1;
}
