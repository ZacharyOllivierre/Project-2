#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <vector>
#include "database.h"
#include "souvenirmanager.h"
#include "purchasewindow.h"

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget* parent = nullptr);

    /**
     * Load team data and build UI.
     */
    void loadTeams(const std::vector<mlbInfo>& teams);

private:
    SouvenirManager m_souvenirManager;
    PurchaseWindow* m_purchaseWindow = nullptr;
    QPushButton* m_viewPurchasesButton = nullptr;

    /**
     * Update cart notification text.
     */
    void updateCartNotification();
};
