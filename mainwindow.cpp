#include "mainwindow.h"
#include "TeamInfoWidget.h"
#include <QHBoxLayout>
#include <QListWidget>

/**
 * Constructor — builds the main window with team list on left
 * and TeamInfoWidget on the right.
 */
mainwindow::mainwindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("MLB Planner");
    resize(1000, 700);
}

/**
 * Loads team data into the window after DB is ready.
 * Call this from main.cpp after db.OpenDB().
 * @param teams - vector of mlbInfo from the database
 */
void mainwindow::loadTeams(const std::vector<mlbInfo>& teams) {
    QWidget* central = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    // ── Left: team list ──
    QListWidget* teamList = new QListWidget();
    teamList->setMaximumWidth(220);
    for (const auto& team : teams) {
        teamList->addItem(QString::fromStdString(team.teamName));
    }

    // ── Right: team info widget ──
    TeamInfoWidget* teamInfo = new TeamInfoWidget();

    // show first team by default
    if (!teams.empty()) {
        teamInfo->setTeam(teams[0]);
        teamList->setCurrentRow(0);
    }

    // clicking a team updates the right panel
    connect(teamList, &QListWidget::currentRowChanged, [=](int row) {
        if (row >= 0 && row < (int)teams.size()) {
            teamInfo->setTeam(teams[row]);
        }
    });

    layout->addWidget(teamList);
    layout->addWidget(teamInfo);
    setCentralWidget(central);
}
