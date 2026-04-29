#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>
#include "../database/database.h"
#include "../purchase/purchasewindow.h"
#include "../souvenir/souvenirmanager.h"
#include <vector>

class TeamInfoWidget;
class AdminWidget;
class BrowseWidget;
class GraphVisualizer;

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);
    void loadTeams(const std::vector<mlbInfo> &teams, Database *db);

private slots:
    void updateCartNotification();

private:
    SouvenirManager  m_souvenirManager;
    Database        *m_db              = nullptr;
    PurchaseWindow  *m_purchaseWindow  = nullptr;

    QPushButton     *m_viewPurchasesButton = nullptr;
    QStackedWidget  *m_stack               = nullptr;

    QWidget         *m_homePage        = nullptr;
    TeamInfoWidget  *m_teamInfoPage    = nullptr;
    BrowseWidget    *m_browsePage      = nullptr;
    GraphVisualizer *m_graphPage       = nullptr;
    AdminWidget     *m_adminPage       = nullptr;

    QPushButton     *m_navHome         = nullptr;
    QPushButton     *m_navTeamInfo     = nullptr;
    QPushButton     *m_navBrowse       = nullptr;
    QPushButton     *m_navGraph        = nullptr;
    QPushButton     *m_navPlanTrip     = nullptr;
    QPushButton     *m_navAdmin        = nullptr;

    QWidget* buildSidebar();
    void     setActivePage(QWidget *page, QPushButton *activeBtn);
    void     styleNavBtn(QPushButton *btn, bool active = false);
};
