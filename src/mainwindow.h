/**
 * @file mainwindow.h
 * @brief Declares the main application window and its buttons, database pointer, souvenir manager, and MST graph object.
 */

#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <vector>

#include "database.h"
#include "souvenirmanager.h"
#include "purchasewindow.h"
#include "stadiumgraph.h"

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwindow(Database* db, QWidget* parent = nullptr);

    void loadTeams(const std::vector<mlbInfo>& teams);
    void loadStadiumDistances(const std::vector<stadiumDistances>& distances);

private:
    Database* m_database = nullptr;

    SouvenirManager m_souvenirManager;
    StadiumGraph m_stadiumGraph;

    PurchaseWindow* m_purchaseWindow = nullptr;

    QPushButton* m_viewPurchasesButton = nullptr;
    QPushButton* m_mstButton = nullptr;

    void updateCartNotification();
    void showMSTResult();
};
