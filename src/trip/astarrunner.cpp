#include "astarrunner.h"
#include "../astar/astar.h"

namespace AStarRunner {

Result run(const std::string& start,
           const std::string& goal,
           const std::vector<stadiumDistances>& distances)
{
    // Trim start/goal in case of whitespace mismatch with DB entries
    std::string trimmedStart = start;
    std::string trimmedGoal  = goal;
    auto trim = [](std::string s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        return s;
    };
    trimmedStart = trim(trimmedStart);
    trimmedGoal  = trim(trimmedGoal);

    // Build graph with trimmed names so lookups always match
    AStarGraph g;
    // Rebuild distances with trimmed names
    std::vector<stadiumDistances> trimmedDist;
    for (const auto &d : distances) {
        stadiumDistances td = d;
        td.originatedStadium  = trim(d.originatedStadium);
        td.destinationStadium = trim(d.destinationStadium);
        trimmedDist.push_back(td);
    }
    g.buildFromDistances(trimmedDist);
    int cost = g.runAStar(trimmedStart, trimmedGoal);
    Result r;
    r.totalCost = cost;
    if (cost >= 0)
        r.path = g.getPath(trimmedGoal);
    return r;
}

} // namespace AStarRunner
