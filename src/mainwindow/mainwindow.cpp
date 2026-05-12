#include "mainwindow.h"
#include "../teaminfowidget/teaminfowidget.h"
#include "../admin/adminwidget.h"
#include "../browse/browsewidget.h"
#include "../homepage/homepage.h"
#include "../trip/tripwidget.h"
#include "../stadiumgraph/stadiumgraph.h"
#include "../dfs/dfs.h"
#include "../bfs/bfs.h"

#include <QCryptographicHash>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QWidget>
#include <QSet>

mainwindow::mainwindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("MLB Planner");
    resize(1200, 750);
    setStyleSheet("QMainWindow { background:#0d1c2e; }");
}

void mainwindow::loadTeams(const std::vector<mlbInfo> &teams, Database *db)
{
    m_db = db;
    m_purchaseWindow = new PurchaseWindow(&m_souvenirManager, this);

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

    // Page 0 — Home
    m_homePage = new homepage;
    m_homePage->setStyleSheet("background:#0d1c2e;");
    m_stack->addWidget(m_homePage);
    // Homepage box navigation
    homepage *home = qobject_cast<homepage*>(m_homePage);

    if (home)
    {
        connect(home, &homepage::toTeamInfoWidget, this, [this]()
                {
                    setActivePage(m_teamInfoPage, m_navTeamInfo);
                });

        connect(home, &homepage::toBrowseWidget, this, [this]()
                {
                    setActivePage(m_browsePage, m_navBrowse);
                });

        connect(home, &homepage::toTripPlannerWidget, this, [this]()
                {
                    if (m_tripPage)
                    {
                        m_tripPage->showPlanPage();
                    }

                    setActivePage(m_tripPage, m_navPlanTrip);
                });

        connect(home, &homepage::toPathViewerWidget, this, [this]()
                {
                setActivePage(m_pathViewerPage, m_navPathViewer);
                });

        // For the top two boxes to count team and stadium count
        int teamCount = static_cast<int>(teams.size());

        QSet<QString> uniqueStadiums;

        for (const mlbInfo &team : teams)
        {
            uniqueStadiums.insert(QString::fromStdString(team.stadiumName).trimmed());
        }

        int stadiumCount = uniqueStadiums.size();

        home->setDatabaseCounts(teamCount, stadiumCount);
    }

    setActivePage(m_homePage, m_navHome);

    // Page 1 — Team Info (now has its own team list sidebar)
    m_teamInfoPage = new TeamInfoWidget(&m_souvenirManager, m_db);
    m_teamInfoPage->loadTeamList(teams);
    connect(m_teamInfoPage, &TeamInfoWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    m_stack->addWidget(m_teamInfoPage);

    // Page 2 — Browse
    m_browsePage = new BrowseWidget(teams);
    m_browsePage->hide();
    m_stack->addWidget(m_browsePage);

    // Page 3 — Plan a Trip
    m_tripPage = new TripWidget(m_db, &m_souvenirManager);
    m_tripPage->refresh();
    connect(m_tripPage, &TripWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    connect(m_tripPage, &TripWidget::routeReady,
            this, &mainwindow::onRouteReady);
    m_stack->addWidget(m_tripPage);

    // Page 4 — Path Viewer (DFS / BFS / MST display)
    m_pathViewerPage = buildPathViewerPage();
    m_stack->addWidget(m_pathViewerPage);

    // Page 5 — Admin
    m_adminPage = new AdminWidget(m_db, this);
    m_adminPage->refresh();
    connect(m_adminPage, &AdminWidget::souvenirDataChanged,
            m_teamInfoPage, &TeamInfoWidget::reloadSouvenirs);
    m_stack->addWidget(m_adminPage);

    root->addWidget(m_stack, 1);

    setActivePage(m_homePage, m_navHome);
}

// =============================================================================
//  Path Viewer placeholder page
// =============================================================================

QWidget* mainwindow::buildPathViewerPage()
{
    auto *page = new QWidget;
    page->setStyleSheet("background:#0d1c2e;");
    auto *lay = new QVBoxLayout(page);
    lay->setContentsMargins(24, 20, 24, 20);
    lay->setSpacing(12);

    auto *header = new QLabel("Path Viewer");
    header->setStyleSheet("color:#ffffff; font-size:18px; font-weight:700;");
    lay->addWidget(header);

    auto *desc = new QLabel(
        "Visualize graph traversal algorithms on the stadium network.\n"
        "Select an algorithm and starting point to see the traversal order.");
    desc->setStyleSheet("color:#4a6d8c; font-size:12px;");
    desc->setWordWrap(true);
    lay->addWidget(desc);

    // Controls card
    auto *card = new QWidget;
    card->setStyleSheet("background:#111f33; border:1px solid #1a2d45; border-radius:4px;");
    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(16, 14, 16, 14);
    cardLay->setSpacing(10);

    auto *ctrlRow = new QHBoxLayout;

    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
        return l;
    };

    auto *algoCmb = new QComboBox;
    algoCmb->addItems({"DFS from Oracle Park", "BFS from Target Field", "MST (Prim's)"});
    algoCmb->setStyleSheet(
        "QComboBox{ background:#0a1628; border:1px solid #1a2d45; border-radius:3px;"
        "  color:#b8d4ec; padding:4px 8px; }"
        "QComboBox::drop-down{ border:none; }"
        "QComboBox QAbstractItemView{ background:#0f1e30; border:1px solid #1a2d45;"
        "  selection-background-color:#1e4a70; color:#b8d4ec; }");

    auto *runBtn = new QPushButton("Run Traversal");
    runBtn->setStyleSheet(
        "QPushButton{ background:#1e4a7a; color:#c8e0f4; border:none; border-radius:3px;"
        "  padding:6px 16px; font-size:12px; }"
        "QPushButton:hover{ background:#255a90; color:#ffffff; }");
    runBtn->setCursor(Qt::PointingHandCursor);

    ctrlRow->addWidget(mkLbl("Algorithm:"));
    ctrlRow->addWidget(algoCmb, 1);
    ctrlRow->addSpacing(16);
    ctrlRow->addWidget(runBtn);
    ctrlRow->addStretch();
    cardLay->addLayout(ctrlRow);

    auto *resultList = new QListWidget;
    resultList->setStyleSheet(
        "QListWidget{ background:#0a1628; border:1px solid #1a2d45; border-radius:3px; }"
        "QListWidget::item{ padding:6px 10px; color:#b8d4ec; border:none; }"
        "QListWidget::item:alternate{ background:#0d1f35; }");
    resultList->setAlternatingRowColors(true);
    cardLay->addWidget(resultList);

    auto *totalLbl = new QLabel("Total mileage: —");
    totalLbl->setStyleSheet("color:#4a9ade; font-size:12px; font-weight:600; border:none;");
    cardLay->addWidget(totalLbl);

    lay->addWidget(card, 1);

    // Wire run button — MST live, DFS/BFS pending
    connect(runBtn, &QPushButton::clicked, this, [this, resultList, totalLbl, algoCmb]() {
        resultList->clear();
        QString algo = algoCmb->currentText();

        if (algo == "MST (Prim\'s)") {
            // Build graph from DB distances
            StadiumGraph g;
            for (const auto &d : m_db->GetStadiumDistancesVector())
                g.addEdge(QString::fromStdString(d.originatedStadium).trimmed(),
                          QString::fromStdString(d.destinationStadium).trimmed(),
                          d.distance);

            // Use first stadium as start
            QStringList all = g.getAllStadiums();
            all.sort();
            if (all.isEmpty()) {
                resultList->addItem("No stadiums found in database.");
                return;
            }

            QList<GraphEdge> mst = g.primMST(all.first());
            int total = g.getTotalMileage(mst);

            for (int i = 0; i < mst.size(); i++) {
                resultList->addItem(
                    QString("%1. %2  →  %3   (%4 mi)")
                        .arg(i + 1).arg(mst[i].from).arg(mst[i].to).arg(mst[i].distance));
            }

            totalLbl->setText(QString("Total MST mileage: %1 mi  |  %2 edges")
                                  .arg(total).arg(mst.size()));

        } else if (algo == "DFS from Oracle Park") {
            DFSGraph g;
            g.buildFromDistances(m_db->GetStadiumDistancesVector());
            DFSResult r = g.performDFSReportFromOraclePark();

            for (int i = 0; i < (int)r.visitOrder.size(); i++) {
                const DFSEdge &e = r.visitOrder[i];
                if (i == 0)
                    resultList->addItem(QString("1.  %1  (start)").arg(QString::fromStdString(e.to)));
                else
                    resultList->addItem(QString("%1.  %2  →  %3  (%4 mi)")
                        .arg(i + 1)
                        .arg(QString::fromStdString(e.from))
                        .arg(QString::fromStdString(e.to))
                        .arg(e.distance));
            }
            totalLbl->setText(QString("Total DFS traversal mileage: %1 mi").arg(r.totalMileage));

        } else if (algo == "BFS from Target Field") {
            BFSGraph g;
            g.buildFromDistances(m_db->GetStadiumDistancesVector());
            BFSResult r = g.performBFSReportFromTargetField();

            for (int i = 0; i < (int)r.visitOrder.size(); i++) {
                const BFSEdge &e = r.visitOrder[i];
                if (i == 0)
                    resultList->addItem(QString("1.  %1  (start)").arg(QString::fromStdString(e.to)));
                else
                    resultList->addItem(QString("%1.  %2  →  %3  (%4 mi)")
                        .arg(i + 1)
                        .arg(QString::fromStdString(e.from))
                        .arg(QString::fromStdString(e.to))
                        .arg(e.distance));
            }
            totalLbl->setText(QString("Total BFS traversal mileage: %1 mi").arg(r.totalMileage));
        }
    });

    return page;
}

// =============================================================================
//  Sidebar
// =============================================================================

QWidget* mainwindow::buildSidebar()
{
    QWidget *sidebar = new QWidget;
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet("background:#111f33;");

    QVBoxLayout *lay = new QVBoxLayout(sidebar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Logo
    QWidget *logo = new QWidget;
    logo->setFixedHeight(52);
    logo->setStyleSheet("QWidget { background:#162035; border:none; }");
    QVBoxLayout *logoLay = new QVBoxLayout(logo);
    logoLay->setContentsMargins(14, 0, 14, 0);
    QLabel *logoTitle = new QLabel("⚾  MLB Planner");
    logoTitle->setStyleSheet("color:#ffffff; font-size:14px; font-weight:700; background:transparent; border:none;");
    logoLay->addWidget(logoTitle);
    lay->addWidget(logo);

    // Cart button
    m_viewPurchasesButton = new QPushButton("View Purchase Screen (Cart: 0)");
    m_viewPurchasesButton->setStyleSheet(
        "QPushButton{ background:#1a3a60; color:#d0e8ff; border:none;"
        "  border-bottom:1px solid #1a2d45; padding:9px 14px;"
        "  font-size:11px; font-weight:600; text-align:left; }"
        "QPushButton:hover{ background:#1e4470; color:#ffffff; }");
    m_viewPurchasesButton->setCursor(Qt::PointingHandCursor);
    connect(m_viewPurchasesButton, &QPushButton::clicked, this, [this]() {
        if (m_purchaseWindow) {
            m_purchaseWindow->refreshScreen();
            m_purchaseWindow->show();
            m_purchaseWindow->raise();
            m_purchaseWindow->activateWindow();
        }
    });
    lay->addWidget(m_viewPurchasesButton);

    // Shopping cart reset
    m_resetCartButton = new QPushButton("Reset Shopping Cart");
    m_resetCartButton->setStyleSheet(
        "QPushButton{ background:#5c1a1a; color:#c8e0f4; border:none;"
        "  border-bottom:1px solid #1a2d45; padding:9px 14px;"
        "  font-size:11px; font-weight:600; text-align:left; }"
        "QPushButton:hover{ background:#7a2222; color:#ffffff; }");
    m_resetCartButton->setCursor(Qt::PointingHandCursor);

    connect(m_resetCartButton, &QPushButton::clicked,
            this, &mainwindow::resetShoppingCart);

    lay->addWidget(m_resetCartButton);

    // End of shopping cart reset
    auto addSection = [&](const QString &label) {
        QLabel *sec = new QLabel(label.toUpper());
        sec->setStyleSheet(
            "color:#2e4d6a; font-size:10px; letter-spacing:1.2px; padding:12px 14px 3px;");
        lay->addWidget(sec);
    };

    m_navHome       = new QPushButton("  Home");
    m_navTeamInfo   = new QPushButton("  Team Info");
    m_navBrowse     = new QPushButton("  Browse");
    m_navPlanTrip   = new QPushButton("  Plan a Trip");
    m_navViewRoute  = new QPushButton("  View Route");
    m_navPathViewer = new QPushButton("  Path Viewer");
    m_navAdmin      = new QPushButton("  Manage Data");

    addSection("Main");
    styleNavBtn(m_navHome);
    lay->addWidget(m_navHome);

    addSection("Stadiums");
    styleNavBtn(m_navTeamInfo);
    styleNavBtn(m_navBrowse);
    lay->addWidget(m_navTeamInfo);
    lay->addWidget(m_navBrowse);

    addSection("Trip Planner");
    styleNavBtn(m_navPlanTrip);
    styleNavBtn(m_navViewRoute);
    m_navViewRoute->setEnabled(false);  // enabled after route is calculated
    lay->addWidget(m_navPlanTrip);
    lay->addWidget(m_navViewRoute);

    addSection("Graph Tools");
    styleNavBtn(m_navPathViewer);
    lay->addWidget(m_navPathViewer);

    addSection("Admin");
    styleNavBtn(m_navAdmin);
    lay->addWidget(m_navAdmin);

    lay->addStretch();

    // Connections
    connect(m_navHome,      &QPushButton::clicked, this, [this]{ setActivePage(m_homePage,       m_navHome); });
    connect(m_navTeamInfo,  &QPushButton::clicked, this, [this]{ setActivePage(m_teamInfoPage,   m_navTeamInfo); });
    connect(m_navBrowse,    &QPushButton::clicked, this, [this]{ setActivePage(m_browsePage,     m_navBrowse); });
    connect(m_navPlanTrip,  &QPushButton::clicked, this, [this]{
        if (m_tripPage) m_tripPage->showPlanPage();
        setActivePage(m_tripPage, m_navPlanTrip);
    });
    connect(m_navViewRoute, &QPushButton::clicked, this, [this]{
        if (m_tripPage) m_tripPage->showRoutePage();
        setActivePage(m_tripPage, m_navViewRoute);
    });
    connect(m_navPathViewer, &QPushButton::clicked, this, [this]{
        setActivePage(m_pathViewerPage, m_navPathViewer);
    });
    connect(m_navAdmin, &QPushButton::clicked, this, [this] {
        bool ok = false;
        QString password = QInputDialog::getText(this,
            "Admin Login", "Enter administrator password:",
            QLineEdit::Password, "", &ok);
        if (!ok) return;

        QString hash = QString(QCryptographicHash::hash(
            QString("cs1d_mlb_admin_salt_%1").arg(password).toUtf8(),
            QCryptographicHash::Sha256).toHex());

        if (hash != "757ccc78485530665f59a01a4d4bcf2818d3ef57d68290a0d58021d3f89463ce") {
            QMessageBox::warning(this, "Access Denied", "Incorrect administrator password.");
            return;
        }

        if (m_adminPage)
            if (auto *aw = qobject_cast<AdminWidget*>(m_adminPage))
                aw->refresh();

        setActivePage(m_adminPage, m_navAdmin);
    });

    return sidebar;
}

// =============================================================================
//  Helpers
// =============================================================================

void mainwindow::setActivePage(QWidget *page, QPushButton *activeBtn)
{
    if (!page || !m_stack) return;
    m_stack->setCurrentWidget(page);
    for (auto *btn : {m_navHome, m_navTeamInfo, m_navBrowse,
                      m_navPlanTrip, m_navViewRoute, m_navPathViewer, m_navAdmin})
        styleNavBtn(btn, btn == activeBtn);
}

void mainwindow::styleNavBtn(QPushButton *btn, bool active)
{
    if (active) {
        btn->setStyleSheet(
            "QPushButton{ background:#162a45; color:#ffffff; border:none;"
            "  border-left:2px solid #4a9ade; padding:8px 14px;"
            "  font-size:12px; text-align:left; }"
            "QPushButton:hover{ background:#162a45; }");
    } else {
        btn->setStyleSheet(
            "QPushButton{ background:transparent; color:#5a80a0; border:none;"
            "  border-left:2px solid transparent; padding:8px 14px;"
            "  font-size:12px; text-align:left; }"
            "QPushButton:hover{ background:#13253d; color:#a0c4e0; }");
    }
    btn->setCursor(Qt::PointingHandCursor);
}

void mainwindow::updateCartNotification()
{
    int count = m_souvenirManager.getTotalItemCount();
    if (m_viewPurchasesButton)
        m_viewPurchasesButton->setText(
            QString("View Purchase Screen (Cart: %1)").arg(count));
}

// Reset Shopping cart costs and items
void mainwindow::resetShoppingCart()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Reset Shopping Cart",
        "Are you sure you want to clear all souvenir purchases?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    m_souvenirManager.clearCart();
    updateCartNotification();

    if (m_purchaseWindow != nullptr)
    {
        m_purchaseWindow->refreshScreen();
    }

    QMessageBox::information(
        this,
        "Shopping Cart Reset",
        "The shopping cart has been cleared."
        );

    if (m_tripPage != nullptr)
    {
        m_tripPage->refreshCartTotal();
    }
}

void mainwindow::onRouteReady()
{
    if (m_navViewRoute) {
        m_navViewRoute->setEnabled(true);
        if (m_tripPage) m_tripPage->showRoutePage();
        setActivePage(m_tripPage, m_navViewRoute);
    }
}
