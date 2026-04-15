#include "Database.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Database db;
    db.OpenDB();

    auto byCap = [](const mlbInfo& a, const mlbInfo& b) -> bool {
        return a.seatingCapacity < b.seatingCapacity;
    };

    db.SortVector(
        db.GetMlbInfoVector(),
        function<bool(const mlbInfo&, const mlbInfo&)>(byCap)
        );

    const vector<mlbInfo>& teams = db.GetMlbInfoVector();
    for (int i = 0; i < (int)teams.size(); i++) {
        cout << teams[i].seatingCapacity
             << " - " << teams[i].teamName
             << " - " << teams[i].stadiumName << endl;
    }

    mainwindow w;
    w.loadTeams(db.GetMlbInfoVector());
    w.show();

    return app.exec();
}
