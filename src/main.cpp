#include "Database.h"
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






    return 0;
}


