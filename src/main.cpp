/**
 * @file main.cpp
 * @brief Starts the Qt application, opens the databases, loads MLB data, and shows the main window.
 */

#include "database.h"
#include "mainwindow.h"

#include <QApplication>
#include <functional>
#include <iostream>
#include <vector>

using std::function;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #121826;
            color: #ffffff;
            font-size: 12pt;
        }

        QLabel {
            color: #ffffff;
            background: transparent;
        }

        QListWidget {
            background-color: #1b2333;
            color: #ffffff;
            border: 1px solid #2f3b52;
            border-radius: 8px;
            padding: 4px;
        }

        QListWidget::item {
            padding: 8px;
            border-radius: 6px;
        }

        QListWidget::item:selected {
            background-color: #2563eb;
            color: #ffffff;
        }

        QFrame {
            background-color: #1b2333;
            border: 1px solid #2f3b52;
            border-radius: 10px;
        }
    )");

    Database db;
    db.OpenDB();

    auto byCap = [](const mlbInfo& a, const mlbInfo& b) -> bool
    {
        return a.seatingCapacity < b.seatingCapacity;
    };

    db.SortVector(
        db.GetMlbInfoVector(),
        function<bool(const mlbInfo&, const mlbInfo&)>(byCap)
        );

    mainwindow w(&db);
    w.loadStadiumDistances(db.GetStadiumDistancesVector());
    w.loadTeams(db.GetMlbInfoVector());
    w.show();

    return app.exec();
}
