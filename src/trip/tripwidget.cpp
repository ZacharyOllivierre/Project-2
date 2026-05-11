#include "tripwidget.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSplitter>
#include <algorithm>
#include <stdexcept>

static const char* TRIP_STYLE = R"(
    QWidget       { background:#0d1c2e; color:#b8d4ec;
                    font-family:'Segoe UI',sans-serif; font-size:12px; }
    QLabel        { background:transparent; border:none; color:#b8d4ec; }
    QListWidget   { background:#0a1628; border:1px solid #1a2d45; border-radius:3px; outline:none; }
    QListWidget::item          { padding:6px 10px; color:#b8d4ec; border:none; }
    QListWidget::item:selected { background:#1a3a5c; color:#ffffff; }
    QListWidget::item:alternate{ background:#0d1f35; }
    QComboBox     { background:#0a1628; border:1px solid #1a2d45;
                    border-radius:3px; color:#b8d4ec; padding:4px 8px; }
    QComboBox:focus            { border-color:#4a9ade; }
    QComboBox::drop-down       { border:none; width:16px; }
    QComboBox QAbstractItemView{ background:#0f1e30; border:1px solid #1a2d45;
                                 selection-background-color:#1e4a70; color:#b8d4ec; }
    QSpinBox      { background:#0a1628; border:1px solid #1a2d45;
                    border-radius:3px; color:#b8d4ec; padding:4px 6px; }
    QSplitter::handle { background:#1a2d45; border:none; }
    QScrollBar:vertical        { background:#0a1628; width:7px; border:none; }
    QScrollBar::handle:vertical{ background:#1a2d45; border-radius:3px; min-height:20px; }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
)";

// =============================================================================
//  Constructor
// =============================================================================

TripWidget::TripWidget(Database *db, SouvenirManager *mgr, QWidget *parent)
    : QWidget(parent), m_db(db), m_mgr(mgr)
{
    qDebug() << "[TRIP] Constructor start";
    setStyleSheet(TRIP_STYLE);

    m_outerStack = new QStackedWidget(this);
    m_outerStack->addWidget(buildPlanPage());
    m_outerStack->addWidget(buildRoutePage());
    m_outerStack->setCurrentIndex(0);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(m_outerStack);
    qDebug() << "[TRIP] Constructor done";
}

void TripWidget::showPlanPage()  { m_outerStack->setCurrentIndex(0); }
void TripWidget::showRoutePage() { m_outerStack->setCurrentIndex(1); }

// =============================================================================
//  refresh — populate dropdowns from DB
// =============================================================================

void TripWidget::refresh()
{
    qDebug() << "[TRIP] refresh() start, teams:" << (int)m_db->GetMlbInfoVector().size();

    m_graph = StadiumGraph();
    for (const auto &d : m_db->GetStadiumDistancesVector())
        m_graph.addEdge(QString::fromStdString(d.originatedStadium),
                        QString::fromStdString(d.destinationStadium),
                        d.distance);

    QStringList names;
    for (const auto &info : m_db->GetMlbInfoVector())
        names << QString::fromStdString(info.stadiumName).trimmed();
    names.sort();

    m_cmbStart->clear();
    m_cmbEnd->clear();
    m_cmbStartSolo->clear();

    for (const QString &n : names) {
        m_cmbStart->addItem(n);
        m_cmbEnd->addItem(n);
        m_cmbStartSolo->addItem(n);
    }

    if (m_cmbEnd->count() > 1)
        m_cmbEnd->setCurrentIndex(1);

    qDebug() << "[TRIP] refresh() done";
}

// =============================================================================
//  Build — Plan Page
// =============================================================================

QWidget* TripWidget::buildPlanPage()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(0);

    auto *header = new QLabel("Plan a Trip");
    header->setStyleSheet("color:#ffffff; font-size:18px; font-weight:700; padding-bottom:16px;");
    root->addWidget(header);

    auto *cols = new QHBoxLayout;
    cols->setSpacing(16);
    root->addLayout(cols);

    // ── LEFT: Mode selector ──────────────────────────────────────────────────
    auto *modeBox = new QWidget;
    modeBox->setStyleSheet("background:#111f33; border:1px solid #1a2d45; border-radius:4px;");
    auto *modeLayout = new QVBoxLayout(modeBox);
    modeLayout->setContentsMargins(14, 14, 14, 14);
    modeLayout->setSpacing(4);

    auto *modeTitle = new QLabel("Trip Mode");
    modeTitle->setStyleSheet("color:#ddeeff; font-size:13px; font-weight:600; border:none; padding-bottom:8px;");
    modeLayout->addWidget(modeTitle);

    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->setExclusive(true);

    auto addSection = [&](const QString &label) {
        auto *l = new QLabel(label.toUpper());
        l->setStyleSheet("color:#2e4d6a; font-size:10px; letter-spacing:1px; padding:10px 0 3px; border:none;");
        modeLayout->addWidget(l);
    };

    auto addMode = [&](const QString &label, const QString &id, const QString &sub, bool ready) -> QPushButton* {
        auto *btn = new QPushButton;
        btn->setCheckable(true);
        btn->setObjectName(id);
        QString text = label;
        if (!sub.isEmpty())
            text += "\n" + (ready ? sub : sub + "  [coming soon]");
        btn->setText(text);
        btn->setStyleSheet(
            "QPushButton{ background:transparent; border:none; border-left:2px solid transparent;"
            "  padding:7px 12px; text-align:left; font-size:12px; color:#b8d4ec; }"
            "QPushButton:hover{ background:#13253d; border-left-color:#2a5070; color:#ddeeff; }"
            "QPushButton:checked{ background:#162a45; border-left-color:#4a9ade; color:#ffffff; }");
        btn->setCursor(Qt::PointingHandCursor);
        m_modeGroup->addButton(btn);
        modeLayout->addWidget(btn);
        return btn;
    };

    addSection("Custom");
    auto *btnAstar = addMode("Custom Trip — A*",       "astar",    "Shortest path, two stops",    true);
                    addMode("Custom Trip — Dijkstra",  "dijkstra", "Single-source shortest path", false);

    addSection("Pre-Planned");
                    addMode("All 30 from Marlins Park","marlins",  "Greedy, all stadiums",        false);
                    addMode("All 30 — Min Distance",   "mst",      "Minimum spanning tree",       true);
                    addMode("DFS from Oracle Park",    "dfs",      "Depth-first traversal",       false);
                    addMode("BFS from Target Field",   "bfs",      "Breadth-first traversal",     false);

    btnAstar->setChecked(true);
    m_currentMode = "astar";

    modeLayout->addStretch();
    cols->addWidget(modeBox, 1);

    connect(m_modeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, &TripWidget::onModeChanged);

    // ── RIGHT: Input panel (stacked) ─────────────────────────────────────────
    auto *rightBox = new QWidget;
    rightBox->setStyleSheet("background:#111f33; border:1px solid #1a2d45; border-radius:4px;");
    auto *rightLayout = new QVBoxLayout(rightBox);
    rightLayout->setContentsMargins(14, 14, 14, 14);
    rightLayout->setSpacing(10);

    auto *pickerTitle = new QLabel("Selected Stops");
    pickerTitle->setStyleSheet("color:#ddeeff; font-size:13px; font-weight:600; border:none; padding-bottom:4px;");
    rightLayout->addWidget(pickerTitle);

    m_inputStack = new QStackedWidget;
    m_inputStack->setStyleSheet("background:transparent; border:none;");

    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
        return l;
    };

    // Page 0: two-stop (A*, Dijkstra)
    auto *twoPage = new QWidget;
    twoPage->setStyleSheet("background:transparent; border:none;");
    auto *twoLay = new QVBoxLayout(twoPage);
    twoLay->setContentsMargins(0,0,0,0); twoLay->setSpacing(8);
    m_cmbStart = new QComboBox; styleCombo(m_cmbStart);
    m_cmbEnd   = new QComboBox; styleCombo(m_cmbEnd);
    twoLay->addWidget(mkLbl("Start Stadium")); twoLay->addWidget(m_cmbStart);
    twoLay->addWidget(mkLbl("End Stadium"));   twoLay->addWidget(m_cmbEnd);
    twoLay->addStretch();
    m_inputStack->addWidget(twoPage);

    // Page 1: start-only (MST, Marlins)
    auto *soloPage = new QWidget;
    soloPage->setStyleSheet("background:transparent; border:none;");
    auto *soloLay = new QVBoxLayout(soloPage);
    soloLay->setContentsMargins(0,0,0,0); soloLay->setSpacing(8);
    m_cmbStartSolo = new QComboBox; styleCombo(m_cmbStartSolo);
    soloLay->addWidget(mkLbl("Start Stadium")); soloLay->addWidget(m_cmbStartSolo);
    auto *soloNote = new QLabel("All 30 stadiums will be visited.");
    soloNote->setStyleSheet("color:#4a6d8c; font-size:11px; border:none;");
    soloNote->setWordWrap(true);
    soloLay->addWidget(soloNote);
    soloLay->addStretch();
    m_inputStack->addWidget(soloPage);

    // Page 2: fixed start (DFS/BFS)
    auto *fixedPage = new QWidget;
    fixedPage->setStyleSheet("background:transparent; border:none;");
    auto *fixedLay = new QVBoxLayout(fixedPage);
    fixedLay->setContentsMargins(0,0,0,0); fixedLay->setSpacing(8);
    m_lblFixedStart = new QLabel("Starting at: —");
    m_lblFixedStart->setStyleSheet("color:#b8d4ec; font-size:12px; font-weight:600; border:none;");
    auto *fixedNote = new QLabel("Start stadium is fixed by the algorithm.");
    fixedNote->setStyleSheet("color:#4a6d8c; font-size:11px; border:none;");
    fixedNote->setWordWrap(true);
    fixedLay->addWidget(m_lblFixedStart);
    fixedLay->addWidget(fixedNote);
    fixedLay->addStretch();
    m_inputStack->addWidget(fixedPage);

    m_inputStack->setCurrentIndex(0);
    rightLayout->addWidget(m_inputStack, 1);

    m_btnCalc = makeBtn("Calculate Route", "#1a5c34");
    connect(m_btnCalc, &QPushButton::clicked, this, &TripWidget::onCalculateRoute);
    rightLayout->addWidget(m_btnCalc);

    cols->addWidget(rightBox, 1);
    return page;
}

// =============================================================================
//  Build — Route Page
// =============================================================================

QWidget* TripWidget::buildRoutePage()
{
    auto *page = new QWidget;
    auto *root = new QVBoxLayout(page);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header bar
    auto *headerBar = new QWidget;
    headerBar->setFixedHeight(44);
    headerBar->setStyleSheet("background:#111f33; border-bottom:1px solid #1a2d45;");
    auto *hlay = new QHBoxLayout(headerBar);
    hlay->setContentsMargins(16, 0, 16, 0);

    auto *btnBack = new QPushButton("Back to Planner");
    btnBack->setStyleSheet(
        "QPushButton{ background:transparent; color:#7aa0c0; border:none; font-size:12px; padding:0; }"
        "QPushButton:hover{ color:#ffffff; }");
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, this, &TripWidget::onBackToPlanner);

    auto *routeTitle = new QLabel("View Route");
    routeTitle->setStyleSheet("color:#ffffff; font-size:14px; font-weight:600; border:none;");

    m_lblTotalMiles = new QLabel("0 miles total");
    m_lblTotalMiles->setStyleSheet("color:#4a9ade; font-size:12px; font-weight:600; border:none;");

    m_lblTotalSpent = new QLabel("$0.00 spent");
    m_lblTotalSpent->setStyleSheet("color:#40c878; font-size:12px; font-weight:600; border:none;");

    hlay->addWidget(btnBack);
    hlay->addSpacing(16);
    hlay->addWidget(routeTitle);
    hlay->addStretch();
    hlay->addWidget(m_lblTotalMiles);
    hlay->addSpacing(16);
    hlay->addWidget(m_lblTotalSpent);
    root->addWidget(headerBar);

    // Body: stop list | souvenir panel (side by side, no QSplitter to avoid paint crash)
    auto *body = new QHBoxLayout;
    body->setContentsMargins(0, 0, 0, 0);
    body->setSpacing(1);

    // Left: stop list
    auto *stopPanel = new QWidget;
    stopPanel->setStyleSheet("background:#0d1c2e;");
    auto *stopLayout = new QVBoxLayout(stopPanel);
    stopLayout->setContentsMargins(16, 12, 8, 12);
    stopLayout->setSpacing(6);

    auto *stopTitle = new QLabel("Route");
    stopTitle->setObjectName("stopTitle");
    stopTitle->setStyleSheet("color:#ddeeff; font-size:13px; font-weight:600; border:none;");
    stopLayout->addWidget(stopTitle);

    m_stopList = new QListWidget;
    m_stopList->setAlternatingRowColors(true);
    m_stopList->setStyleSheet(
        "QListWidget{ background:#0a1628; border:1px solid #1a2d45; border-radius:0px; }"
        "QListWidget::item{ padding:6px 10px; color:#b8d4ec; }"
        "QListWidget::item:selected{ background:#1a3a5c; color:#ffffff; }");
    connect(m_stopList, &QListWidget::currentRowChanged, this, &TripWidget::onStopSelected);
    stopLayout->addWidget(m_stopList, 1);

    body->addWidget(stopPanel, 1);

    // Divider
    auto *div = new QWidget;
    div->setFixedWidth(1);
    div->setStyleSheet("background:#1a2d45;");
    body->addWidget(div);

    // Right: souvenir panel
    auto *souvenirPanel = new QWidget;
    souvenirPanel->setStyleSheet("background:#0d1c2e;");
    auto *souvenirLayout = new QVBoxLayout(souvenirPanel);
    souvenirLayout->setContentsMargins(8, 12, 16, 12);
    souvenirLayout->setSpacing(6);

    m_lblStopTitle = new QLabel("Souvenirs");
    m_lblStopTitle->setStyleSheet("color:#ddeeff; font-size:13px; font-weight:600; border:none;");
    souvenirLayout->addWidget(m_lblStopTitle);

    m_souvenirList = new QListWidget;
    m_souvenirList->setAlternatingRowColors(true);
    m_souvenirList->setStyleSheet(
        "QListWidget{ background:#0a1628; border:1px solid #1a2d45; border-radius:0px; }"
        "QListWidget::item{ padding:6px 10px; color:#b8d4ec; }"
        "QListWidget::item:selected{ background:#1a3a5c; color:#ffffff; }");
    souvenirLayout->addWidget(m_souvenirList, 1);

    auto *buyRow = new QHBoxLayout;
    auto *qtyLbl = new QLabel("Qty:");
    qtyLbl->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
    m_spnQty = new QSpinBox;
    m_spnQty->setRange(1, 100);
    m_spnQty->setValue(1);
    m_spnQty->setFixedWidth(60);
    m_btnBuy = makeBtn("Buy Souvenir", "#1e4a7a");
    connect(m_btnBuy, &QPushButton::clicked, this, &TripWidget::onBuyStop);
    buyRow->addWidget(qtyLbl);
    buyRow->addWidget(m_spnQty);
    buyRow->addStretch();
    buyRow->addWidget(m_btnBuy);
    souvenirLayout->addLayout(buyRow);

    body->addWidget(souvenirPanel, 1);

    auto *bodyWidget = new QWidget;
    bodyWidget->setLayout(body);
    root->addWidget(bodyWidget, 1);

    return page;
}

// =============================================================================
//  Algorithm runners
// =============================================================================

QList<TripStop> TripWidget::runAstar(const QString &start, const QString &end)
{
    if (start.trimmed().isEmpty() || end.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please select a start and end stadium.");
        return {};
    }

    qDebug() << "[ASTAR] running from" << start << "to" << end;

    AStarRunner::Result result = AStarRunner::run(
        start.trimmed().toStdString(),
        end.trimmed().toStdString(),
        m_db->GetStadiumDistancesVector());

    qDebug() << "[ASTAR] cost:" << result.totalCost << "path size:" << (int)result.path.size();

    if (result.totalCost < 0 || result.path.empty()) {
        QMessageBox::warning(this, "No Path",
            QString("No path found from %1 to %2.").arg(start, end));
        return {};
    }

    m_totalMiles = result.totalCost;
    QList<TripStop> stops;

    // Build distance lookup (trimmed keys)
    QMap<QString, int> distMap;
    for (const auto &d : m_db->GetStadiumDistancesVector()) {
        QString from = QString::fromStdString(d.originatedStadium).trimmed();
        QString to   = QString::fromStdString(d.destinationStadium).trimmed();
        distMap[from + "|" + to] = d.distance;
        distMap[to   + "|" + from] = d.distance;
    }

    for (int i = 0; i < (int)result.path.size(); i++) {
        TripStop ts;
        ts.stadiumName = QString::fromStdString(result.path[i]).trimmed();

        for (const auto &info : m_db->GetMlbInfoVector())
            if (QString::fromStdString(info.stadiumName).trimmed() == ts.stadiumName)
                { ts.teamName = QString::fromStdString(info.teamName); break; }

        ts.distFromPrev = (i == 0) ? 0
            : distMap.value(QString::fromStdString(result.path[i-1]).trimmed()
                            + "|" + ts.stadiumName, 0);

        stops.append(ts);
    }

    return stops;
}

QList<TripStop> TripWidget::runMST(const QString &start)
{
    qDebug() << "[MST] running from" << start;

    QList<GraphEdge> mstEdges = m_graph.primMST(start);
    m_totalMiles = m_graph.getTotalMileage(mstEdges);

    qDebug() << "[MST] edges:" << mstEdges.size() << "total miles:" << m_totalMiles;

    // BFS over MST tree to get visit order
    QMap<QString, QList<QString>> tree;
    for (const GraphEdge &e : mstEdges)
        tree[e.from].append(e.to);

    QList<QString> queue = { start };
    QSet<QString>  visited;
    QList<QString> order;
    visited.insert(start);

    while (!queue.isEmpty()) {
        QString cur = queue.takeFirst();
        order.append(cur);
        for (const QString &child : tree.value(cur))
            if (!visited.contains(child))
                { visited.insert(child); queue.append(child); }
    }

    QMap<QString, int> edgeDist;
    for (const GraphEdge &e : mstEdges)
        edgeDist[e.from + "|" + e.to] = e.distance;

    QList<TripStop> stops;
    for (int i = 0; i < order.size(); i++) {
        TripStop ts;
        ts.stadiumName  = order[i];
        ts.distFromPrev = (i == 0) ? 0 : edgeDist.value(order[i-1] + "|" + order[i], 0);
        for (const auto &info : m_db->GetMlbInfoVector())
            if (QString::fromStdString(info.stadiumName).trimmed() == ts.stadiumName)
                { ts.teamName = QString::fromStdString(info.teamName); break; }
        stops.append(ts);
    }

    return stops;
}

QList<TripStop> TripWidget::runDijkstra(const QString &start, const QString &end)
{
    Q_UNUSED(start); Q_UNUSED(end);
    QMessageBox::information(this, "Coming Soon",
        "Dijkstra is under development. Replace this function body with your implementation.");
    return {};
}

QList<TripStop> TripWidget::runDFS(const QString &start)
{
    Q_UNUSED(start);
    QMessageBox::information(this, "Coming Soon",
        "DFS traversal is under development. Replace this function body with your implementation.");
    return {};
}

QList<TripStop> TripWidget::runBFS(const QString &start)
{
    Q_UNUSED(start);
    QMessageBox::information(this, "Coming Soon",
        "BFS traversal is under development. Replace this function body with your implementation.");
    return {};
}

// =============================================================================
//  Slots
// =============================================================================

void TripWidget::onModeChanged(QAbstractButton *btn)
{
    if (!btn) return;
    m_currentMode = btn->objectName();

    if (m_currentMode == "astar" || m_currentMode == "dijkstra")
        m_inputStack->setCurrentIndex(0);
    else if (m_currentMode == "mst" || m_currentMode == "marlins")
        m_inputStack->setCurrentIndex(1);
    else if (m_currentMode == "dfs") {
        m_lblFixedStart->setText("Starting at: Oracle Park");
        m_inputStack->setCurrentIndex(2);
    } else if (m_currentMode == "bfs") {
        m_lblFixedStart->setText("Starting at: Target Field");
        m_inputStack->setCurrentIndex(2);
    }
}

void TripWidget::onCalculateRoute()
{
    try {
        qDebug() << "[CALC] mode:" << m_currentMode;

        QList<TripStop> stops;
        m_totalMiles = 0;

        QString start     = m_cmbStart->currentText().trimmed();
        QString end       = m_cmbEnd->currentText().trimmed();
        QString startSolo = m_cmbStartSolo->currentText().trimmed();

        qDebug() << "[CALC] start:" << start << "end:" << end << "solo:" << startSolo;

        if (m_currentMode == "astar") {
            if (start == end) {
                QMessageBox::warning(this, "Invalid", "Start and end stadiums must be different.");
                return;
            }
            stops = runAstar(start, end);
        } else if (m_currentMode == "dijkstra") {
            if (start == end) {
                QMessageBox::warning(this, "Invalid", "Start and end stadiums must be different.");
                return;
            }
            stops = runDijkstra(start, end);
        } else if (m_currentMode == "mst") {
            stops = runMST(startSolo);
        } else if (m_currentMode == "marlins") {
            stops = runDijkstra("loanDepot park", end);
        } else if (m_currentMode == "dfs") {
            stops = runDFS("Oracle Park");
        } else if (m_currentMode == "bfs") {
            stops = runBFS("Target Field");
        }

        if (stops.isEmpty()) {
            qDebug() << "[CALC] stops empty, not switching page";
            return;
        }

        qDebug() << "[CALC] got" << stops.size() << "stops, loading route";
        m_currentRoute = stops;
        loadRoute(stops);

        qDebug() << "[CALC] switching to route page";
        m_outerStack->setCurrentIndex(1);
        emit routeReady();
        qDebug() << "[CALC] done";

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", QString("Exception: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Error", "Unknown error in Calculate Route. Check debug output.");
    }
}

void TripWidget::onBackToPlanner()
{
    m_outerStack->setCurrentIndex(0);
}

void TripWidget::onStopSelected(int row)
{
    if (row < 0 || row >= m_currentRoute.size()) return;
    const TripStop &stop = m_currentRoute[row];
    m_lblStopTitle->setText(QString("Souvenirs — %1")
        .arg(stop.teamName.isEmpty() ? stop.stadiumName : stop.teamName));
    loadSouvenirList(stop.stadiumName);
}

void TripWidget::onBuyStop()
{
    if (!m_mgr) return;
    int row = m_souvenirList->currentRow();
    if (row < 0 || row >= m_stopSouvenirs.size()) return;
    int stopRow = m_stopList->currentRow();
    if (stopRow < 0 || stopRow >= m_currentRoute.size()) return;

    SouvenirItem item = m_stopSouvenirs[row];
    m_mgr->buySouvenir(m_currentRoute[stopRow].stadiumName, item, m_spnQty->value());
    m_lblTotalSpent->setText(QString("$%1 spent").arg(m_mgr->getGrandTotal(), 0, 'f', 2));
    emit cartUpdated();
}

// =============================================================================
//  Helpers
// =============================================================================

void TripWidget::loadRoute(const QList<TripStop> &stops)
{
    m_stopList->clear();

    for (int i = 0; i < stops.size(); i++) {
        const TripStop &s = stops[i];
        QString text = QString("%1.  %2").arg(i + 1)
            .arg(s.teamName.isEmpty() ? s.stadiumName : s.teamName);
        if (s.distFromPrev > 0)
            text += QString("   +%1 mi").arg(s.distFromPrev);
        m_stopList->addItem(text);
    }

    m_lblTotalMiles->setText(QString("%1 miles total").arg(m_totalMiles));
    m_lblTotalSpent->setText(QString("$%1 spent")
        .arg(m_mgr ? m_mgr->getGrandTotal() : 0.0, 0, 'f', 2));

    auto *title = findChild<QLabel*>("stopTitle");
    if (title)
        title->setText(QString("Route  %1 Stop%2")
            .arg(stops.size()).arg(stops.size() == 1 ? "" : "s"));

    if (!stops.isEmpty()) {
        m_stopList->setCurrentRow(0);
        onStopSelected(0);
    }
}

void TripWidget::loadSouvenirList(const QString &stadiumName)
{
    m_souvenirList->clear();
    m_stopSouvenirs = getSouvenirs(stadiumName);
    for (const SouvenirItem &s : m_stopSouvenirs)
        m_souvenirList->addItem(QString("%1    $%2").arg(s.name).arg(s.price, 0, 'f', 2));
    if (!m_stopSouvenirs.isEmpty())
        m_souvenirList->setCurrentRow(0);
}

QList<SouvenirItem> TripWidget::getSouvenirs(const QString &stadiumName)
{
    QList<SouvenirItem> items;
    QSqlDatabase db = QSqlDatabase::database("MLB Info Database");
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("SELECT item_name, price FROM souvenirs "
                  "WHERE trim(team_name) IN ("
                  "  SELECT trim(team_name) FROM mlb_info WHERE trim(stadium_name)=trim(?)"
                  ") ORDER BY item_name");
        q.addBindValue(stadiumName);
        if (q.exec())
            while (q.next())
                items.append({q.value(0).toString(), q.value(1).toDouble()});
    }
    if (items.isEmpty())
        items = {{"Baseball Cap", 19.99}, {"Baseball Bat", 89.39},
                 {"Team Pennant", 17.99}, {"Autographed Baseball", 29.99},
                 {"Team Jersey", 199.99}};
    return items;
}

QPushButton* TripWidget::makeBtn(const QString &label, const QString &bg)
{
    auto *b = new QPushButton(label);
    b->setStyleSheet(QString(
        "QPushButton{ background:%1; color:#c8e0f4; border:none; border-radius:3px;"
        "  padding:6px 14px; font-size:12px; }"
        "QPushButton:hover{ color:#ffffff; }"
        "QPushButton:disabled{ color:#3a5060; }").arg(bg));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

void TripWidget::styleCombo(QComboBox *c)
{
    c->setStyleSheet(
        "QComboBox{ background:#0a1628; border:1px solid #1a2d45; border-radius:3px;"
        "  color:#b8d4ec; padding:5px 8px; }"
        "QComboBox:focus{ border-color:#4a9ade; }"
        "QComboBox::drop-down{ border:none; width:16px; }"
        "QComboBox QAbstractItemView{ background:#0f1e30; border:1px solid #1a2d45;"
        "  selection-background-color:#1e4a70; color:#b8d4ec; }");
}
