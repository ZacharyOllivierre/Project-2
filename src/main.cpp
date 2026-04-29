#include "Database.h"
#include "astar/astar.h"
#include <QCoreApplication>
#include <unordered_map>

int main(int argc, char *argv[])
{
    QCoreApplication qtApp(argc, argv);
    Database db;    

    db.OpenDB();

    auto byCap = [](const mlbInfo& a, const mlbInfo& b) -> bool { return a.seatingCapacity < b.seatingCapacity; };
    db.SortVector(db.GetMlbInfoVector(), function<bool(const mlbInfo&, const mlbInfo&)>(byCap));

    const vector<mlbInfo>& teams = db.GetMlbInfoVector();
    for (int i = 0; i < teams.size(); i++)
    {
        cout << teams[i].seatingCapacity << " - " << teams[i].teamName << " - " << teams[i].stadiumName << endl;
    }


    std::unordered_map<string, int> myMap;

    // ── A* TEST (blake) ──────────────────────────────────────────────────────
    // Remove or comment out this block once the module is integrated into the UI.
    {
        const string start = "Wrigley Field";
        const string goal  = "Yankee Stadium";

        StadiumGraph graph;
        graph.buildFromDistances(db.GetStadiumDistancesVector());

        cout << "\n[A* TEST] " << start << " -> " << goal << "\n";
        int cost = graph.runAStar(start, goal);
        if (cost == -1)
            cout << "[A*] No path found.\n";
        else
            graph.printPath(start, goal);
    }
    // ── END A* TEST ──────────────────────────────────────────────────────────



    return 0;
}


