#pragma once

#include <QMainWindow>
#include <QPushButton>
#include "../database/database.h"
#include "../purchase/purchasewindow.h"
#include "../souvenir/souvenirmanager.h"
#include <vector>

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);

    /**
     * Load team data and build UI.
     */
    void loadTeams(const std::vector<mlbInfo> &teams);

private:
    SouvenirManager m_souvenirManager;
    PurchaseWindow *m_purchaseWindow = nullptr;
    QPushButton *m_viewPurchasesButton = nullptr;

    /**
     * Update cart notification text.
     */
    void updateCartNotification();
};
