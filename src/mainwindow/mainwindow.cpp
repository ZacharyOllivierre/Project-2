#include "mainwindow.h"
#include "../teaminfowidget/teaminfowidget.h"
#include "../admin/adminwidget.h"
#include "../browse/browsewidget.h"
#include "../homepage/homepage.h"
#include "../trip/tripwidget.h"
#include "../stadiumgraph/stadiumgraph.h"
#include "../graph/graphvisualizer.h"
#include "../graph/graphactionbuffer.h"
#include "../bfs/bfs.h"
#include "../dfs/dfs.h"
#include "../dijkstra/dijkstra.h"

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
#include <QListWidget>

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

    // --- Page 0: Homepage ---
    m_homePage = new homepage;
    m_homePage->setStyleSheet("background:#0d1c2e;");

    auto *hp = qobject_cast<homepage*>(m_homePage);
    if (hp) {
        connect(hp, &homepage::toBrowseWidget,      this, [this]{ setActivePage(m_browsePage,     m_navBrowse); });
        connect(hp, &homepage::toTeamInfoWidget,    this, [this]{ setActivePage(m_teamInfoPage,   m_navTeamInfo); });
        connect(hp, &homepage::toTripPlannerWidget, this, [this]{
            if (m_tripPage) m_tripPage->showPlanPage();
            setActivePage(m_tripPage, m_navPlanTrip);
        });
        connect(hp, &homepage::toPathViewerWidget,  this, [this]{ setActivePage(m_pathViewerPage, m_navPathViewer); });
    }
    m_stack->addWidget(m_homePage);

    // --- Page 1: Team Info ---
    m_teamInfoPage = new TeamInfoWidget(&m_souvenirManager, m_db);
    m_teamInfoPage->loadTeamList(teams);
    connect(m_teamInfoPage, &TeamInfoWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    m_stack->addWidget(m_teamInfoPage);

    // --- Page 2: Browse ---
    m_browsePage = new BrowseWidget(teams);
    m_browsePage->hide();
    m_stack->addWidget(m_browsePage);

    // --- Page 3: Plan a Trip ---
    m_tripPage = new TripWidget(m_db, &m_souvenirManager);
    m_tripPage->refresh();
    connect(m_tripPage, &TripWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    connect(m_tripPage, &TripWidget::routeReady,
            this, &mainwindow::onRouteReady);

    connect(m_tripPage, &TripWidget::animateRoute,
            this, [this](const GraphActionBuffer &buf) {
                if (m_pathVisualizer) {
                    m_pathVisualizer->resetGraph();
                    m_pathVisualizer->playActions(buf, 80);
                    setActivePage(m_pathViewerPage, m_navPathViewer);
                }
            });
    m_stack->addWidget(m_tripPage);

    // --- Page 4: Path Viewer ---
    m_pathViewerPage = buildPathViewerPage();
    m_stack->addWidget(m_pathViewerPage);

    // --- Page 5: Admin ---
    m_adminPage = new AdminWidget(m_db, this);
    m_adminPage->refresh();

    // IMPORTANT: These connections trigger the real-time refresh
    connect(m_adminPage, &AdminWidget::souvenirDataChanged,
            m_teamInfoPage, &TeamInfoWidget::reloadSouvenirs);
    connect(m_adminPage, &AdminWidget::dataReloaded,
            this, &mainwindow::onDataReloaded);

    m_stack->addWidget(m_adminPage);

    root->addWidget(m_stack, 1);

    // Initial Homepage counts
    onDataReloaded();

    setActivePage(m_homePage, m_navHome);
}

// Master refresh function called whenever database data changes
void mainwindow::onDataReloaded()
{
    // Fetch fresh data from vectors
    auto teams = m_db->GetMlbInfoVector();

    // 1. Update Team Info Page
    if (m_teamInfoPage)
        m_teamInfoPage->loadTeamList(teams);

    // 2. Update Trip Planner Dropdowns and Graph
    if (m_tripPage)
        m_tripPage->refresh();

    // 3. Update Browse Page
    if (m_browsePage)
        m_browsePage->updateTeams(teams);

    // 4. Update Homepage Counters
    int teamCount = (int)teams.size();
    int openCount = 0;
    for (const auto &t : teams) {
        QString roof = QString::fromStdString(t.roofType).trimmed();
        if (roof.compare("Open", Qt::CaseInsensitive) == 0) {
            openCount++;
        }
    }
    if (auto *hp = qobject_cast<homepage*>(m_homePage)) {
        hp->setDatabaseCounts(teamCount, openCount);
    }

    // 5. Update Path Viewer Graph
    if (m_pathVisualizer) {
        m_pathVisualizer->updateGraphData(teams, m_db->GetStadiumDistancesVector());
        m_pathVisualizer->loadGraph();
    }
}

// =============================================================================
//  Path Viewer placeholder page
// =============================================================================

QWidget* mainwindow::buildPathViewerPage()
{
    auto *page = new QWidget;
    page->setStyleSheet("background:#0d1c2e;");
    auto *outerLay = new QHBoxLayout(page);
    outerLay->setContentsMargins(0, 0, 0, 0);
    outerLay->setSpacing(0);

    m_pathVisualizer = new GraphVisualizer(QPointF(600, 500));
    m_pathVisualizer->updateGraphData(
        m_db->GetMlbInfoVector(),
        m_db->GetStadiumDistancesVector());
    m_pathVisualizer->loadGraph();
    outerLay->addWidget(m_pathVisualizer, 2);

    auto *rightPanel = new QWidget;
    rightPanel->setStyleSheet("background:#0d1c2e;");
    auto *lay = new QVBoxLayout(rightPanel);
    lay->setContentsMargins(12, 16, 16, 16);
    lay->setSpacing(10);
    outerLay->addWidget(rightPanel, 1);

    auto *header = new QLabel("Path Viewer");
    header->setStyleSheet("color:#ffffff; font-size:15px; font-weight:700; border:none;");
    lay->addWidget(header);

    auto *card = new QWidget;
    card->setStyleSheet("background:#111f33; border:1px solid #1a2d45; border-radius:4px;");
    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(12, 10, 12, 10);
    cardLay->setSpacing(8);

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
    lay->addStretch(0);

    connect(runBtn, &QPushButton::clicked, this, [this, resultList, totalLbl, algoCmb]() {
        resultList->clear();
        if (m_pathVisualizer) m_pathVisualizer->resetGraph();
        QString algo = algoCmb->currentText();

        if (algo == "MST (Prim\'s)") {
            StadiumGraph g;
            for (const auto &d : m_db->GetStadiumDistancesVector())
                g.addEdge(QString::fromStdString(d.originatedStadium).trimmed(),
                          QString::fromStdString(d.destinationStadium).trimmed(),
                          d.distance);

            QStringList all = g.getAllStadiums();
            all.sort();
            if (all.isEmpty()) return;

            QList<GraphEdge> mst = g.primMST(all.first());
            int total = g.getTotalMileage(mst);

            for (int i = 0; i < mst.size(); i++) {
                resultList->addItem(QString("%1. %2  →  %3   (%4 mi)").arg(i+1).arg(mst[i].from).arg(mst[i].to).arg(mst[i].distance));
            }

            totalLbl->setText(QString("Total MST mileage: %1 mi").arg(total));
            if (m_pathVisualizer) {
                GraphActionBuffer buf;
                buf.setNodeStart(all.first());
                for (const GraphEdge &e : mst) {
                    buf.setEdgePath(e.from, e.to);
                    buf.setEdgePath(e.to, e.from);
                    buf.setNodePath(e.to);
                }
                m_pathVisualizer->playActions(buf, 60);
            }
        }
        // ... Other traversals (DFS/BFS) continue here ...
    });

    return page;
}

QWidget* mainwindow::buildSidebar()
{
    QWidget *sidebar = new QWidget;
    sidebar->setFixedWidth(200);
    sidebar->setStyleSheet("background:#111f33;");

    QVBoxLayout *lay = new QVBoxLayout(sidebar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    QWidget *logo = new QWidget;
    logo->setFixedHeight(52);
    logo->setStyleSheet("QWidget { background:#162035; border:none; }");
    QVBoxLayout *logoLay = new QVBoxLayout(logo);
    logoLay->setContentsMargins(14, 0, 14, 0);
    QLabel *logoTitle = new QLabel("⚾  MLB Planner");
    logoTitle->setStyleSheet("color:#ffffff; font-size:14px; font-weight:700; background:transparent; border:none;");
    logoLay->addWidget(logoTitle);
    lay->addWidget(logo);

    m_viewPurchasesButton = new QPushButton("View Purchase Screen (Cart: 0)");
    m_viewPurchasesButton->setStyleSheet(
        "QPushButton{ background:#1a3a60; color:#d0e8ff; border:none; border-bottom:1px solid #1a2d45; padding:9px 14px; font-size:11px; font-weight:600; text-align:left; }");
    connect(m_viewPurchasesButton, &QPushButton::clicked, this, [this]() {
        if (m_purchaseWindow) { m_purchaseWindow->refreshScreen(); m_purchaseWindow->show(); }
    });
    lay->addWidget(m_viewPurchasesButton);

    m_resetCartButton = new QPushButton("  Reset Cart");
    m_resetCartButton->setStyleSheet("QPushButton{ background:#3a1a1a; color:#f0a0a0; border:none; border-bottom:1px solid #1a2d45; padding:9px 14px; font-size:11px; font-weight:600; text-align:left; }");
    connect(m_resetCartButton, &QPushButton::clicked, this, &mainwindow::resetShoppingCart);
    lay->addWidget(m_resetCartButton);

    auto addSection = [&](const QString &label) {
        QLabel *sec = new QLabel(label.toUpper());
        sec->setStyleSheet("color:#2e4d6a; font-size:10px; letter-spacing:1.2px; padding:12px 14px 3px;");
        lay->addWidget(sec);
    };

    m_navHome       = new QPushButton("  Home");
    m_navTeamInfo   = new QPushButton("  Team Info");
    m_navBrowse     = new QPushButton("  Browse");
    m_navPlanTrip   = new QPushButton("  Plan a Trip");
    m_navViewRoute  = new QPushButton("  View Route");
    m_navPathViewer = new QPushButton("  Path Viewer");
    m_navAdmin      = new QPushButton("  Manage Data");

    addSection("Main"); lay->addWidget(m_navHome);
    addSection("Stadiums"); lay->addWidget(m_navTeamInfo); lay->addWidget(m_navBrowse);
    addSection("Trip Planner"); lay->addWidget(m_navPlanTrip); lay->addWidget(m_navViewRoute);
    addSection("Graph Tools"); lay->addWidget(m_navPathViewer);
    addSection("Admin"); lay->addWidget(m_navAdmin);

    lay->addStretch();

    connect(m_navHome,      &QPushButton::clicked, this, [this]{ setActivePage(m_homePage,       m_navHome); });
    connect(m_navTeamInfo,  &QPushButton::clicked, this, [this]{ setActivePage(m_teamInfoPage,   m_navTeamInfo); });
    connect(m_navBrowse,    &QPushButton::clicked, this, [this]{ setActivePage(m_browsePage,     m_navBrowse); });
    connect(m_navPlanTrip,  &QPushButton::clicked, this, [this]{ if (m_tripPage) m_tripPage->showPlanPage(); setActivePage(m_tripPage, m_navPlanTrip); });
    connect(m_navViewRoute, &QPushButton::clicked, this, [this]{ if (m_tripPage) m_tripPage->showRoutePage(); setActivePage(m_tripPage, m_navViewRoute); });
    connect(m_navPathViewer, &QPushButton::clicked, this, [this]{ setActivePage(m_pathViewerPage, m_navPathViewer); });
    connect(m_navAdmin, &QPushButton::clicked, this, [this] {
        bool ok = false;
        QString password = QInputDialog::getText(this, "Admin Login", "Enter password:", QLineEdit::Password, "", &ok);
        if (ok && (QString(QCryptographicHash::hash(QString("cs1d_mlb_admin_salt_%1").arg(password).toUtf8(), QCryptographicHash::Sha256).toHex()) == "757ccc78485530665f59a01a4d4bcf2818d3ef57d68290a0d58021d3f89463ce")) {
            if (m_adminPage) qobject_cast<AdminWidget*>(m_adminPage)->refresh();
            setActivePage(m_adminPage, m_navAdmin);
        }
    });

    return sidebar;
}

void mainwindow::setActivePage(QWidget *page, QPushButton *activeBtn)
{
    if (!page || !m_stack) return;
    m_stack->setCurrentWidget(page);
    for (auto *btn : {m_navHome, m_navTeamInfo, m_navBrowse, m_navPlanTrip, m_navViewRoute, m_navPathViewer, m_navAdmin})
        styleNavBtn(btn, btn == activeBtn);
}

void mainwindow::styleNavBtn(QPushButton *btn, bool active)
{
    if (active) btn->setStyleSheet("QPushButton{ background:#162a45; color:#ffffff; border:none; border-left:2px solid #4a9ade; padding:8px 14px; font-size:12px; text-align:left; }");
    else btn->setStyleSheet("QPushButton{ background:transparent; color:#5a80a0; border:none; border-left:2px solid transparent; padding:8px 14px; font-size:12px; text-align:left; }");
    btn->setCursor(Qt::PointingHandCursor);
}

void mainwindow::updateCartNotification()
{
    if (m_viewPurchasesButton)
        m_viewPurchasesButton->setText(QString("View Purchase Screen (Cart: %1)").arg(m_souvenirManager.getTotalItemCount()));
}

void mainwindow::onRouteReady()
{
    if (m_navViewRoute) {
        m_navViewRoute->setEnabled(true);
        if (m_tripPage) m_tripPage->showRoutePage();
        setActivePage(m_tripPage, m_navViewRoute);
    }
}

void mainwindow::resetShoppingCart()
{
    if (QMessageBox::question(this, "Reset", "Clear cart?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        m_souvenirManager.clearCart();
        updateCartNotification();
        if (m_purchaseWindow) m_purchaseWindow->refreshScreen();
    }
}
