#include "mainwindow.h"
#include "../teaminfowidget/teaminfowidget.h"
<<<<<<< HEAD
=======
#include "../admin/adminwidget.h"
#include "../browse/browsewidget.h"
#include "../graph/graphvisualizer.h"
#include "../homepage/homepage.h"
>>>>>>> 9d86517 (Homepage UI)

#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

/**
 * Constructor
 * Build the main window shell.
 */
mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("MLB Planner");
    resize(1100, 700);

    m_purchaseWindow = new PurchaseWindow(&m_souvenirManager, this);
<<<<<<< HEAD
=======

    QWidget *central = new QWidget(this);
    central->setObjectName("centralWidget");
    central->setStyleSheet("background:#0d1c2e;");
    QHBoxLayout *root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    setCentralWidget(central);

    root->addWidget(buildSidebar());

    m_stack = new QStackedWidget;
    m_stack->setStyleSheet("QStackedWidget { background:#0d1c2e; }");

    // Page 0 — Home (No longer blank yippeee - Brandon)
    homepage *home = new homepage;
    m_homePage = home;
    m_homePage->setStyleSheet("background:#0d1c2e;");
    m_stack->addWidget(m_homePage);

    // Page 1 — Team Info
    m_teamInfoPage = new TeamInfoWidget(&m_souvenirManager, m_db);
    if (!teams.empty())
        m_teamInfoPage->setTeam(teams[0]);
    connect(m_teamInfoPage, &TeamInfoWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    m_stack->addWidget(m_teamInfoPage);

    // Page 2 — Browse
    m_browsePage = new BrowseWidget(teams);
    m_browsePage->hide();   // prevent it auto-showing before user navigates to it
    m_stack->addWidget(m_browsePage);

    // Page 3 — Graph
    m_graphPage = new GraphVisualizer(QPointF(800, 600));
    m_graphPage->updateGraphData(teams, m_db->GetStadiumDistancesVector());
    m_graphPage->loadGraph();
    m_stack->addWidget(m_graphPage);

    // Page 4 — Plan a Trip placeholder
    auto *tripPage = new QWidget;
    tripPage->setStyleSheet("background:#0d1c2e;");
    m_stack->addWidget(tripPage);

    // Page 4 — Admin
    m_adminPage = new AdminWidget(m_db, this);
    m_adminPage->refresh();
    connect(m_adminPage, &AdminWidget::souvenirDataChanged,
            m_teamInfoPage, &TeamInfoWidget::reloadSouvenirs);
    m_stack->addWidget(m_adminPage);

    root->addWidget(m_stack, 1);

    // Default to Home — Browse won't appear until user clicks it
    setActivePage(m_homePage, m_navHome);

    // Connect homepage clickable boxes to actual page navigation
    connect(home, &homepage::toBrowseWidget, this, [this]() {
        setActivePage(m_browsePage, m_navBrowse);
    });

    connect(home, &homepage::toTeamInfoWidget, this, [this]() {
        setActivePage(m_teamInfoPage, m_navTeamInfo);
    });

    connect(home, &homepage::toTripPlannerWidget, this, [this]() {
        setActivePage(m_stack->widget(4), m_navPlanTrip);
    });


>>>>>>> 9d86517 (Homepage UI)
}

/**
 * Update the cart count on the button.
 */
void mainwindow::updateCartNotification()
{
    int count = m_souvenirManager.getTotalItemCount();

    if (m_viewPurchasesButton != nullptr) {
        m_viewPurchasesButton->setText(QString("View Purchase Screen (Cart: %1)").arg(count));
    }
}

/**
 * Load team data into the UI.
 */
void mainwindow::loadTeams(const std::vector<mlbInfo> &teams)
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(central);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    // -------------------------
    // Left side
    // -------------------------
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    m_viewPurchasesButton = new QPushButton();
    m_viewPurchasesButton->setStyleSheet("QPushButton {"
                                         "background-color: #2563eb;"
                                         "color: white;"
                                         "font-weight: 600;"
                                         "border: 1px solid #3b82f6;"
                                         "border-radius: 8px;"
                                         "padding: 8px 14px;"
                                         "}"
                                         "QPushButton:hover {"
                                         "background-color: #3b82f6;"
                                         "}"
                                         "QPushButton:pressed {"
                                         "background-color: #1d4ed8;"
                                         "}");

    updateCartNotification();

    QListWidget *teamList = new QListWidget();
    teamList->setMaximumWidth(240);

    for (const auto &team : teams) {
        teamList->addItem(QString::fromStdString(team.teamName));
    }

    leftLayout->addWidget(m_viewPurchasesButton);
    leftLayout->addWidget(teamList);

    // -------------------------
    // Right side
    // -------------------------
    TeamInfoWidget *teamInfo = new TeamInfoWidget(&m_souvenirManager);

    if (!teams.empty()) {
        teamInfo->setTeam(teams[0]);
        teamList->setCurrentRow(0);
    }

    connect(teamList, &QListWidget::currentRowChanged, this, [teamInfo, teams](int row) {
        if (row >= 0 && row < static_cast<int>(teams.size())) {
            teamInfo->setTeam(teams[row]);
        }
    });

    connect(m_viewPurchasesButton, &QPushButton::clicked, this, [this]() {
        if (m_purchaseWindow != nullptr) {
            m_purchaseWindow->refreshScreen();
            m_purchaseWindow->show();
            m_purchaseWindow->raise();
            m_purchaseWindow->activateWindow();
        }
    });

    connect(teamInfo, &TeamInfoWidget::cartUpdated, this, [this]() { updateCartNotification(); });

    layout->addWidget(leftPanel);
    layout->addWidget(teamInfo, 1);

    setCentralWidget(central);
}
