#pragma once
#include <QMainWindow>
#include <vector>
#include "Database.h"

class mainwindow : public QMainWindow {
    Q_OBJECT
public:
    explicit mainwindow(QWidget* parent = nullptr);

    /**
     * Loads team data and builds the UI.
     * @param teams - vector of mlbInfo from database
     */
    void loadTeams(const std::vector<mlbInfo>& teams);
};
