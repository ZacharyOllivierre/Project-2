#pragma once
#include "../database/database.h"
#include <vector>
#include <string>

namespace AStarRunner {
    // Returns {path, totalCost}. path is empty if no route found.
    struct Result {
        std::vector<std::string> path;
        int totalCost = -1;
    };
    Result run(const std::string& start,
               const std::string& goal,
               const std::vector<stadiumDistances>& distances);
}
