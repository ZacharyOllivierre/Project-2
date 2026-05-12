#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QWidget>
#include "../database/database.h"
#include "../purchase/purchasewindow.h"
#include "../souvenir/souvenirmanager.h"
#include "../homepage/homepage.h"
#include <vector>

class TeamInfoWidget;
class AdminWidget;
class BrowseWidget;
class TripWidget;

class mainwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainwindow(QWidget *parent = nullptr);
    void loadTeams(const std::vector<mlbInfo> &teams, Database *db);

private slots:
    void updateCartNotification();
    void resetShoppingCart();
    void onRouteReady();

private:
    SouvenirManager  m_souvenirManager;
    Database        *m_db             = nullptr;
    PurchaseWindow  *m_purchaseWindow = nullptr;

    QPushButton    *m_viewPurchasesButton = nullptr;
    QStackedWidget *m_stack               = nullptr;

    QWidget        *m_homePage        = nullptr;
    TeamInfoWidget *m_teamInfoPage    = nullptr;
    BrowseWidget   *m_browsePage      = nullptr;
    TripWidget     *m_tripPage        = nullptr;
    QWidget        *m_pathViewerPage  = nullptr;
    AdminWidget    *m_adminPage       = nullptr;

    QPushButton    *m_navHome         = nullptr;
    QPushButton    *m_navTeamInfo     = nullptr;
    QPushButton    *m_navBrowse       = nullptr;
    QPushButton    *m_navPlanTrip     = nullptr;
    QPushButton    *m_navViewRoute    = nullptr;
    QPushButton    *m_navPathViewer   = nullptr;
    QPushButton    *m_resetCartButton = nullptr;
    QPushButton    *m_navAdmin        = nullptr;


    QWidget* buildSidebar();
    QWidget* buildPathViewerPage();
    void     setActivePage(QWidget *page, QPushButton *activeBtn);
    void     styleNavBtn(QPushButton *btn, bool active = false);
};
