#include "adminwidget.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QSqlError>
#include <QSqlQuery>

// ─── Depth-aware palette ────────────────────────────────────────────────────
// Rule: deeper background = darker shade. Foreground elements step lighter.
// Page bg:      #0d1c2e
// Widget bg:    #111f33  (one step lighter — sits "on" the page)
// Input bg:     #0a1628  (slightly darker — recessed/inset feel)
// Border:       #1a2d45  (just visible enough to define edges, no harsh contrast)
// Text dimmed:  #4a6d8c  (section labels, placeholders — recedes)
// Text mid:     #7aa0c0  (secondary info)
// Text default: #b8d4ec  (normal readable text)
// Text primary: #ddeeff  (labels, headings)
// Text white:   #ffffff  (active item, titles)
// Accent:       #4a9ade  (interactive highlight)
// ────────────────────────────────────────────────────────────────────────────

static const char* GLOBAL_STYLE = R"(
    QWidget {
        background: #0d1c2e;
        color: #b8d4ec;
        font-family: 'Segoe UI', sans-serif;
        font-size: 12px;
    }
    QTabWidget::pane {
        border: 1px solid #1a2d45;
        border-top: none;
        background: #0d1c2e;
    }
    QTabBar::tab {
        background: #0f1e30;
        color: #4a6d8c;
        padding: 7px 18px;
        border: 1px solid #1a2d45;
        border-bottom: none;
        margin-right: 2px;
    }
    QTabBar::tab:selected {
        background: #0d1c2e;
        color: #ddeeff;
        border-bottom: 1px solid #0d1c2e;
    }
    QTabBar::tab:hover { color: #7aa0c0; }
    QGroupBox {
        border: 1px solid #1a2d45;
        border-radius: 4px;
        margin-top: 10px;
        padding-top: 10px;
        color: #4a6d8c;
        font-size: 11px;
    }
    QGroupBox::title {
        subcontrol-origin: margin;
        left: 8px;
        top: -1px;
        color: #4a6d8c;
        background: #0d1c2e;
        padding: 0 4px;
    }
    QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
        background: #0a1628;
        border: 1px solid #1a2d45;
        border-radius: 3px;
        color: #b8d4ec;
        padding: 4px 7px;
        selection-background-color: #1e4a70;
    }
    QLineEdit:focus, QSpinBox:focus,
    QDoubleSpinBox:focus, QComboBox:focus {
        border-color: #4a9ade;
        color: #ddeeff;
    }
    QLineEdit::placeholder { color: #2e4d6a; }
    QComboBox::drop-down { border: none; width: 16px; }
    QComboBox::down-arrow { width: 8px; height: 8px; }
    QComboBox QAbstractItemView {
        background: #0f1e30;
        border: 1px solid #1a2d45;
        selection-background-color: #1e4a70;
        color: #b8d4ec;
        outline: none;
    }
    QTableWidget {
        background: #0a1628;
        border: 1px solid #1a2d45;
        gridline-color: #111f33;
        alternate-background-color: #0d1f35;
        outline: none;
    }
    QTableWidget::item {
        padding: 4px 8px;
        color: #b8d4ec;
        border: none;
    }
    QTableWidget::item:selected {
        background: #1a3a5c;
        color: #ddeeff;
    }
    QHeaderView::section {
        background: #0f1e30;
        color: #4a6d8c;
        border: none;
        border-bottom: 1px solid #1a2d45;
        border-right: 1px solid #1a2d45;
        padding: 5px 8px;
        font-size: 10px;
        letter-spacing: 0.8px;
    }
    QScrollBar:vertical {
        background: #0a1628;
        width: 7px;
        border: none;
    }
    QScrollBar::handle:vertical {
        background: #1a2d45;
        border-radius: 3px;
        min-height: 20px;
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
    QSplitter::handle { background: #1a2d45; }
    QLabel { background: transparent; border: none; color: #b8d4ec; }
)";

AdminWidget::AdminWidget(Database *db, QWidget *parent)
    : QWidget(parent), m_db(db)
{
    setupUi();
}

void AdminWidget::refresh()
{
    ensureSouvenirTable();
    loadStadiumTable();

    m_cmbSouvenirTeam->clear();
    m_cmbPricingTeam->clear();

    for (const auto &info : m_db->GetMlbInfoVector()) {
        QString name = QString::fromStdString(info.teamName).trimmed();
        m_cmbSouvenirTeam->addItem(name);
        m_cmbPricingTeam->addItem(name);
    }

    if (m_cmbSouvenirTeam->count() > 0) {
        loadSouvenirTable(m_cmbSouvenirTeam->currentText());
        loadPricingTable(m_cmbPricingTeam->currentText());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI Setup
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::setupUi()
{
    setStyleSheet(GLOBAL_STYLE);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header bar — slightly lighter than page, white title
    auto *header = new QWidget;
    header->setFixedHeight(40);
    header->setStyleSheet("background:#111f33; border-bottom:1px solid #1a2d45;");
    auto *hlay = new QHBoxLayout(header);
    hlay->setContentsMargins(16, 0, 16, 0);
    auto *headerLabel = new QLabel("Admin — Manage Data");
    headerLabel->setStyleSheet("color:#ffffff; font-size:13px; font-weight:600; border:none;");
    hlay->addWidget(headerLabel);
    root->addWidget(header);

    auto *tabs = new QTabWidget;
    tabs->setDocumentMode(true);
    tabs->addTab(buildStadiumTab(),  "  Stadiums  ");
    tabs->addTab(buildSouvenirTab(), "  Souvenirs  ");
    tabs->addTab(buildPricingTab(),  "  Pricing  ");
    tabs->addTab(buildDatabaseTab(), "  Database  ");
    root->addWidget(tabs);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Stadium Tab
// ─────────────────────────────────────────────────────────────────────────────

QWidget* AdminWidget::buildStadiumTab()
{
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    // Toolbar
    auto *toolbar = new QHBoxLayout;
    m_stadiumSearch = new QLineEdit;
    m_stadiumSearch->setPlaceholderText("Search by team or stadium…");
    m_stadiumSearch->setFixedHeight(28);
    connect(m_stadiumSearch, &QLineEdit::textChanged, this, &AdminWidget::onStadiumSearchChanged);

    m_btnNewStadium = makeBtn("+ Add New Stadium", "#1e4a7a");
    connect(m_btnNewStadium, &QPushButton::clicked, this, &AdminWidget::onAddNewStadium);
    toolbar->addWidget(m_stadiumSearch);
    toolbar->addWidget(m_btnNewStadium);
    lay->addLayout(toolbar);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setHandleWidth(2);

    // Table
    m_stadiumTable = new QTableWidget(0, 8);
    m_stadiumTable->setHorizontalHeaderLabels(
        {"Team", "Stadium", "League", "Capacity", "Roof", "Surface", "Typology", "Opened"});
    styleTable(m_stadiumTable);
    connect(m_stadiumTable, &QTableWidget::itemSelectionChanged,
            this, &AdminWidget::onStadiumSelectionChanged);
    splitter->addWidget(m_stadiumTable);

    // Form
    auto *formBox = new QGroupBox("Stadium Details");
    auto *flay    = new QVBoxLayout(formBox);
    flay->setSpacing(6);

    m_edtTeamName    = new QLineEdit; m_edtTeamName->setPlaceholderText("Team Name");
    m_edtStadiumName = new QLineEdit; m_edtStadiumName->setPlaceholderText("Stadium Name");
    m_edtLocation    = new QLineEdit; m_edtLocation->setPlaceholderText("City, State");
    m_cmbLeague      = new QComboBox; m_cmbLeague->addItems({"American", "National"});
    m_cmbRoof        = new QComboBox; m_cmbRoof->addItems({"Open", "Fixed", "Retractable"});
    m_cmbSurface     = new QComboBox; m_cmbSurface->addItems(
        {"Grass", "AstroTurf GameDay Grass", "FieldTurf", "Artificial Turf"});
    m_cmbTypology    = new QComboBox; m_cmbTypology->addItems(
        {"Jewel Box", "Retro Classic", "Retro Modern", "Multipurpose", "Contemporary"});
    m_spnCapacity    = new QSpinBox;  m_spnCapacity->setRange(1000, 100000); m_spnCapacity->setSuffix(" seats");
    m_spnYearOpened  = new QSpinBox;  m_spnYearOpened->setRange(1860, 2100);
    m_edtCenterField = new QLineEdit; m_edtCenterField->setPlaceholderText("e.g. 404 feet (123 m)");

    // Helper for consistent label+field row
    auto addRow = [&](const QString &l1, QWidget *w1,
                      const QString &l2, QWidget *w2,
                      const QString &l3 = "", QWidget *w3 = nullptr) {
        auto *row = new QHBoxLayout;
        row->setSpacing(6);
        auto lbl = [](const QString &t) {
            auto *l = new QLabel(t);
            // Field labels: slightly lighter than dim, clearly readable
            l->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
            l->setFixedWidth(60);
            return l;
        };
        row->addWidget(lbl(l1)); row->addWidget(w1, 1);
        row->addWidget(lbl(l2)); row->addWidget(w2, 1);
        if (!l3.isEmpty() && w3) { row->addWidget(lbl(l3)); row->addWidget(w3, 1); }
        flay->addLayout(row);
    };

    addRow("Team:",     m_edtTeamName,    "Stadium:", m_edtStadiumName, "Location:", m_edtLocation);
    addRow("League:",   m_cmbLeague,      "Roof:",    m_cmbRoof,        "Surface:",  m_cmbSurface);
    addRow("Typology:", m_cmbTypology,    "Capacity:", m_spnCapacity,   "Opened:",   m_spnYearOpened);

    auto *lastRow = new QHBoxLayout;
    auto *cfLbl = new QLabel("Center Field:");
    cfLbl->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
    lastRow->addWidget(cfLbl); lastRow->addWidget(m_edtCenterField, 1); lastRow->addStretch();
    flay->addLayout(lastRow);

    // Buttons
    auto *btnRow = new QHBoxLayout;
    m_btnSaveStadium   = makeBtn("Save Stadium",    "#1a5c34");
    m_btnDeleteStadium = makeBtn("Remove Stadium",  "#5c1a1a");
    auto *btnClear     = makeBtn("Clear",           "#1a2d45");
    connect(m_btnSaveStadium,   &QPushButton::clicked, this, &AdminWidget::onSaveStadium);
    connect(m_btnDeleteStadium, &QPushButton::clicked, this, &AdminWidget::onDeleteStadium);
    connect(btnClear,           &QPushButton::clicked, this, &AdminWidget::onClearStadiumForm);
    m_btnDeleteStadium->setEnabled(false);
    btnRow->addWidget(m_btnSaveStadium);
    btnRow->addWidget(m_btnDeleteStadium);
    btnRow->addWidget(btnClear);
    btnRow->addStretch();
    flay->addLayout(btnRow);

    splitter->addWidget(formBox);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    lay->addWidget(splitter);
    return page;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Souvenir Tab
// ─────────────────────────────────────────────────────────────────────────────

QWidget* AdminWidget::buildSouvenirTab()
{
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    auto *topRow = new QHBoxLayout;
    auto *teamLbl = new QLabel("Team:");
    teamLbl->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
    m_cmbSouvenirTeam = new QComboBox;
    m_cmbSouvenirTeam->setMinimumWidth(220);
    connect(m_cmbSouvenirTeam, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWidget::onSouvenirTeamChanged);
    topRow->addWidget(teamLbl);
    topRow->addWidget(m_cmbSouvenirTeam);
    topRow->addStretch();
    lay->addLayout(topRow);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setHandleWidth(2);

    m_souvenirTable = new QTableWidget(0, 4);
    m_souvenirTable->setHorizontalHeaderLabels({"Item", "Category", "Price", "ID"});
    styleTable(m_souvenirTable);
    m_souvenirTable->setColumnHidden(3, true);
    connect(m_souvenirTable, &QTableWidget::itemSelectionChanged,
            this, &AdminWidget::onSouvenirSelectionChanged);
    splitter->addWidget(m_souvenirTable);

    auto *formBox = new QGroupBox("Add / Edit Souvenir");
    auto *flay    = new QVBoxLayout(formBox);
    flay->setSpacing(6);

    m_edtSouvenirItem     = new QLineEdit; m_edtSouvenirItem->setPlaceholderText("Item name");
    m_cmbSouvenirCategory = new QComboBox;
    m_cmbSouvenirCategory->addItems({"Apparel","Equipment","Memorabilia","Collectible","Food & Drink","Other"});
    m_spnSouvenirPrice = new QDoubleSpinBox;
    m_spnSouvenirPrice->setRange(0.01, 9999.99);
    m_spnSouvenirPrice->setDecimals(2);
    m_spnSouvenirPrice->setPrefix("$ ");
    m_spnSouvenirPrice->setValue(9.99);

    auto *formRow = new QHBoxLayout;
    formRow->setSpacing(6);
    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
        return l;
    };
    formRow->addWidget(mkLbl("Item:"));     formRow->addWidget(m_edtSouvenirItem, 2);
    formRow->addWidget(mkLbl("Category:")); formRow->addWidget(m_cmbSouvenirCategory);
    formRow->addWidget(mkLbl("Price:"));    formRow->addWidget(m_spnSouvenirPrice);
    flay->addLayout(formRow);

    auto *btnRow = new QHBoxLayout;
    m_btnSaveSouvenir   = makeBtn("Save Item",   "#1a5c34");
    m_btnDeleteSouvenir = makeBtn("Remove Item", "#5c1a1a");
    auto *btnClear      = makeBtn("Clear",       "#1a2d45");
    connect(m_btnSaveSouvenir,   &QPushButton::clicked, this, &AdminWidget::onSaveSouvenir);
    connect(m_btnDeleteSouvenir, &QPushButton::clicked, this, &AdminWidget::onDeleteSouvenir);
    connect(btnClear,            &QPushButton::clicked, this, &AdminWidget::onClearSouvenirForm);
    m_btnDeleteSouvenir->setEnabled(false);
    btnRow->addWidget(m_btnSaveSouvenir);
    btnRow->addWidget(m_btnDeleteSouvenir);
    btnRow->addWidget(btnClear);
    btnRow->addStretch();
    flay->addLayout(btnRow);

    splitter->addWidget(formBox);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    lay->addWidget(splitter);
    return page;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Pricing Tab
// ─────────────────────────────────────────────────────────────────────────────

QWidget* AdminWidget::buildPricingTab()
{
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    auto *topRow = new QHBoxLayout;
    topRow->setSpacing(8);
    auto mkLbl = [](const QString &t) {
        auto *l = new QLabel(t);
        l->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
        return l;
    };
    m_cmbPricingTeam = new QComboBox;
    m_cmbPricingTeam->setMinimumWidth(220);
    connect(m_cmbPricingTeam, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdminWidget::onPricingTeamChanged);

    m_spnPctIncrease = new QDoubleSpinBox;
    m_spnPctIncrease->setRange(-99.0, 500.0);
    m_spnPctIncrease->setDecimals(1);
    m_spnPctIncrease->setSuffix(" %");
    m_spnPctIncrease->setValue(5.0);
    m_spnPctIncrease->setFixedWidth(90);

    auto *btnApply = makeBtn("Apply", "#5c3d0a");
    connect(btnApply, &QPushButton::clicked, this, &AdminWidget::onApplyPctIncrease);

    topRow->addWidget(mkLbl("Team:"));
    topRow->addWidget(m_cmbPricingTeam);
    topRow->addSpacing(16);
    topRow->addWidget(mkLbl("Bulk % change:"));
    topRow->addWidget(m_spnPctIncrease);
    topRow->addWidget(btnApply);
    topRow->addStretch();
    lay->addLayout(topRow);

    m_pricingTable = new QTableWidget(0, 4);
    m_pricingTable->setHorizontalHeaderLabels({"Item", "Category", "Price", "ID"});
    styleTable(m_pricingTable);
    m_pricingTable->setColumnHidden(3, true);
    lay->addWidget(m_pricingTable);

    auto *btnRow = new QHBoxLayout;
    auto *btnSave = makeBtn("Save All Price Changes", "#1a5c34");
    connect(btnSave, &QPushButton::clicked, this, &AdminWidget::onSaveAllPrices);
    btnRow->addWidget(btnSave);
    btnRow->addStretch();
    lay->addLayout(btnRow);
    return page;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Style helpers
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::styleTable(QTableWidget *t)
{
    t->horizontalHeader()->setStretchLastSection(true);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    t->verticalHeader()->setVisible(false);
    t->setAlternatingRowColors(true);
    t->setShowGrid(false);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->verticalHeader()->setDefaultSectionSize(26);
}

QPushButton* AdminWidget::makeBtn(const QString &label, const QString &bgColor)
{
    auto *b = new QPushButton(label);
    // Buttons: bg color passed in, text is always near-white, no visible border
    b->setStyleSheet(QString(
        "QPushButton{"
        "  background:%1;"
        "  color:#c8e0f4;"
        "  border:none;"
        "  border-radius:3px;"
        "  padding:5px 14px;"
        "  font-size:12px;"
        "}"
        "QPushButton:hover{ color:#ffffff; filter:brightness(1.15); }"
        "QPushButton:disabled{ color:#3a5060; }").arg(bgColor));
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

// ─────────────────────────────────────────────────────────────────────────────
// Database maintenance helpers
// ─────────────────────────────────────────────────────────────────────────────

static QString findDatabaseFile(const QString &fileName)
{
    QStringList paths;

    paths << "databases/" + fileName
          << "../databases/" + fileName
          << "../../databases/" + fileName
          << "../../../databases/" + fileName;

    for (const QString &path : paths)
    {
        if (QFileInfo::exists(path))
        {
            return QFileInfo(path).absoluteFilePath();
        }
    }

    return "";
}

static QString activeMlbDatabasePath()
{
    QString originalPath;

    originalPath = findDatabaseFile("mlb_info.db");

    if (originalPath.isEmpty())
    {
        return "";
    }

    return QDir(QFileInfo(originalPath).absolutePath()).filePath("mlb_info_active.db");
}

static bool resetActiveMlbDatabase(QString &errorMessage)
{
    QString originalPath;
    QString activePath;

    originalPath = findDatabaseFile("mlb_info.db");
    activePath = activeMlbDatabasePath();

    if (originalPath.isEmpty())
    {
        errorMessage = "Could not find databases/mlb_info.db.";
        return false;
    }

    if (activePath.isEmpty())
    {
        errorMessage = "Could not determine the active database path.";
        return false;
    }

    if (QFileInfo::exists(activePath))
    {
        if (!QFile::remove(activePath))
        {
            errorMessage = "Could not remove mlb_info_active.db. Make sure the database is closed.";
            return false;
        }
    }

    if (!QFile::copy(originalPath, activePath))
    {
        errorMessage = "Could not copy mlb_info.db to mlb_info_active.db.";
        return false;
    }

    return true;
}

static bool souvenirExists(QSqlDatabase db,
                           const QString &teamName,
                           const QString &itemName)
{
    QSqlQuery query(db);

    query.prepare("SELECT COUNT(*) FROM souvenirs "
                  "WHERE trim(team_name)=trim(?) AND trim(item_name)=trim(?)");

    query.addBindValue(teamName);
    query.addBindValue(itemName);

    if (!query.exec())
    {
        return false;
    }

    if (!query.next())
    {
        return false;
    }

    return query.value(0).toInt() > 0;
}

static bool addDefaultSouvenirsForTeam(QSqlDatabase db,
                                       const QString &teamName,
                                       QString &errorMessage)
{
    struct DefaultSouvenir
    {
        QString itemName;
        double price;
        QString category;
    };

    QList<DefaultSouvenir> defaults;

    defaults.append({"Baseball cap", 19.99, "Apparel"});
    defaults.append({"Baseball bat", 89.39, "Equipment"});
    defaults.append({"Team pennant", 17.99, "Memorabilia"});
    defaults.append({"Autographed baseball", 29.99, "Collectible"});
    defaults.append({"Team jersey", 199.99, "Apparel"});

    for (const DefaultSouvenir &item : defaults)
    {
        if (souvenirExists(db, teamName, item.itemName))
        {
            continue;
        }

        QSqlQuery insertQuery(db);

        insertQuery.prepare("INSERT INTO souvenirs "
                            "(team_name, item_name, price, category) "
                            "VALUES (?, ?, ?, ?)");

        insertQuery.addBindValue(teamName);
        insertQuery.addBindValue(item.itemName);
        insertQuery.addBindValue(item.price);
        insertQuery.addBindValue(item.category);

        if (!insertQuery.exec())
        {
            errorMessage = insertQuery.lastError().text();
            return false;
        }
    }

    return true;
}

static bool addTraditionalSouvenirToAllTeams(QSqlDatabase db,
                                             const vector<mlbInfo> &teams,
                                             const QString &itemName,
                                             double price,
                                             const QString &category,
                                             QString &errorMessage)
{
    for (const mlbInfo &team : teams)
    {
        QString teamName;

        teamName = QString::fromStdString(team.teamName).trimmed();

        if (souvenirExists(db, teamName, itemName))
        {
            QSqlQuery updateQuery(db);

            updateQuery.prepare("UPDATE souvenirs "
                                "SET price=?, category=? "
                                "WHERE trim(team_name)=trim(?) "
                                "AND trim(item_name)=trim(?)");

            updateQuery.addBindValue(price);
            updateQuery.addBindValue(category);
            updateQuery.addBindValue(teamName);
            updateQuery.addBindValue(itemName);

            if (!updateQuery.exec())
            {
                errorMessage = updateQuery.lastError().text();
                return false;
            }
        }
        else
        {
            QSqlQuery insertQuery(db);

            insertQuery.prepare("INSERT INTO souvenirs "
                                "(team_name, item_name, price, category) "
                                "VALUES (?, ?, ?, ?)");

            insertQuery.addBindValue(teamName);
            insertQuery.addBindValue(itemName);
            insertQuery.addBindValue(price);
            insertQuery.addBindValue(category);

            if (!insertQuery.exec())
            {
                errorMessage = insertQuery.lastError().text();
                return false;
            }
        }
    }

    return true;
}

static bool deleteTraditionalSouvenirFromAllTeams(QSqlDatabase db,
                                                  const QString &itemName,
                                                  QString &errorMessage)
{
    QSqlQuery query(db);

    query.prepare("DELETE FROM souvenirs WHERE trim(item_name)=trim(?)");
    query.addBindValue(itemName);

    if (!query.exec())
    {
        errorMessage = query.lastError().text();
        return false;
    }

    return true;
}

static bool importNewStadiumsFromDatabase(QSqlDatabase destinationDb,
                                          const QString &filePath,
                                          QStringList &importedTeams,
                                          QString &errorMessage)
{
    QString connectionName;

    connectionName = "AdminImportConnection";

    if (QSqlDatabase::contains(connectionName))
    {
        QSqlDatabase::removeDatabase(connectionName);
    }

    QSqlDatabase sourceDb;

    sourceDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    sourceDb.setDatabaseName(filePath);

    if (!sourceDb.open())
    {
        errorMessage = sourceDb.lastError().text();
        return false;
    }

    QSqlQuery readQuery(sourceDb);

    if (!readQuery.exec("SELECT team_name, stadium_name, seating_capacity, "
                        "location, playing_surface, league, date_opened, "
                        "distance_to_center_field, ballpark_typology, roof_type "
                        "FROM mlb_info"))
    {
        errorMessage = readQuery.lastError().text();
        sourceDb.close();
        return false;
    }

    while (readQuery.next())
    {
        QString teamName;

        teamName = readQuery.value(0).toString().trimmed();

        QSqlQuery countQuery(destinationDb);

        countQuery.prepare("SELECT COUNT(*) FROM mlb_info WHERE trim(team_name)=trim(?)");
        countQuery.addBindValue(teamName);

        if (!countQuery.exec() || !countQuery.next())
        {
            errorMessage = countQuery.lastError().text();
            sourceDb.close();
            return false;
        }

        if (countQuery.value(0).toInt() > 0)
        {
            QSqlQuery updateQuery(destinationDb);

            updateQuery.prepare("UPDATE mlb_info SET "
                                "stadium_name=?, seating_capacity=?, location=?, "
                                "playing_surface=?, league=?, date_opened=?, "
                                "distance_to_center_field=?, ballpark_typology=?, "
                                "roof_type=? "
                                "WHERE trim(team_name)=trim(?)");

            updateQuery.addBindValue(readQuery.value(1));
            updateQuery.addBindValue(readQuery.value(2));
            updateQuery.addBindValue(readQuery.value(3));
            updateQuery.addBindValue(readQuery.value(4));
            updateQuery.addBindValue(readQuery.value(5));
            updateQuery.addBindValue(readQuery.value(6));
            updateQuery.addBindValue(readQuery.value(7));
            updateQuery.addBindValue(readQuery.value(8));
            updateQuery.addBindValue(readQuery.value(9));
            updateQuery.addBindValue(teamName);

            if (!updateQuery.exec())
            {
                errorMessage = updateQuery.lastError().text();
                sourceDb.close();
                return false;
            }
        }
        else
        {
            QSqlQuery insertQuery(destinationDb);

            insertQuery.prepare("INSERT INTO mlb_info "
                                "(team_name, stadium_name, seating_capacity, location, "
                                "playing_surface, league, date_opened, "
                                "distance_to_center_field, ballpark_typology, roof_type) "
                                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            for (int index = 0; index < 10; index++)
            {
                insertQuery.addBindValue(readQuery.value(index));
            }

            if (!insertQuery.exec())
            {
                errorMessage = insertQuery.lastError().text();
                sourceDb.close();
                return false;
            }
        }

        if (!addDefaultSouvenirsForTeam(destinationDb, teamName, errorMessage))
        {
            sourceDb.close();
            return false;
        }

        importedTeams.append(teamName);
    }

    sourceDb.close();
    importedTeams.removeDuplicates();

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  DB helpers
// ─────────────────────────────────────────────────────────────────────────────

QSqlDatabase AdminWidget::souvenirDB()
{
    return QSqlDatabase::database("MLB Info Database");
}

bool AdminWidget::ensureSouvenirTable()
{
    QSqlQuery q(souvenirDB());
    bool ok = q.exec(R"(
        CREATE TABLE IF NOT EXISTS souvenirs (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            team_name TEXT NOT NULL,
            item_name TEXT NOT NULL,
            price     REAL NOT NULL DEFAULT 0.0,
            category  TEXT NOT NULL DEFAULT 'Other'
        )
    )");
    if (!ok) qWarning() << "[Admin] ensureSouvenirTable:" << q.lastError().text();
    return ok;
}

QList<Souvenir> AdminWidget::souvenirListForTeam(const QString &teamName)
{
    QList<Souvenir> list;
    QSqlQuery q(souvenirDB());
    q.prepare("SELECT id, team_name, item_name, price, category FROM souvenirs "
              "WHERE trim(team_name)=trim(?) ORDER BY item_name");
    q.addBindValue(teamName);
    if (!q.exec()) { qWarning() << q.lastError().text(); return list; }
    while (q.next())
        list.append(Souvenir(q.value(0).toInt(), q.value(1).toString(),
                             q.value(2).toString(), q.value(3).toDouble(),
                             q.value(4).toString()));
    return list;
}

bool AdminWidget::insertSouvenir(const Souvenir &s)
{
    QSqlQuery q(souvenirDB());
    q.prepare("INSERT INTO souvenirs (team_name, item_name, price, category) VALUES (?,?,?,?)");
    q.addBindValue(s.teamName); q.addBindValue(s.itemName);
    q.addBindValue(s.price);    q.addBindValue(s.category);
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] insertSouvenir:" << q.lastError().text();
    return ok;
}

bool AdminWidget::updateSouvenir(const Souvenir &s)
{
    QSqlQuery q(souvenirDB());
    q.prepare("UPDATE souvenirs SET item_name=?, price=?, category=? WHERE id=?");
    q.addBindValue(s.itemName); q.addBindValue(s.price);
    q.addBindValue(s.category); q.addBindValue(s.id);
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] updateSouvenir:" << q.lastError().text();
    return ok;
}

bool AdminWidget::deleteSouvenir(int id)
{
    QSqlQuery q(souvenirDB());
    q.prepare("DELETE FROM souvenirs WHERE id=?");
    q.addBindValue(id);
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] deleteSouvenir:" << q.lastError().text();
    return ok;
}

bool AdminWidget::updateStadiumInDB(const mlbInfo &info, const QString &originalTeamName)
{
    QSqlQuery q(souvenirDB());
    q.prepare(R"(UPDATE mlb_info SET team_name=?,stadium_name=?,seating_capacity=?,
        location=?,playing_surface=?,league=?,date_opened=?,
        distance_to_center_field=?,ballpark_typology=?,roof_type=?
        WHERE trim(team_name)=trim(?))");
    q.addBindValue(QString::fromStdString(info.teamName));
    q.addBindValue(QString::fromStdString(info.stadiumName));
    q.addBindValue(info.seatingCapacity);
    q.addBindValue(QString::fromStdString(info.location));
    q.addBindValue(QString::fromStdString(info.playingSurface));
    q.addBindValue(QString::fromStdString(info.league));
    q.addBindValue(info.dateOpened);
    q.addBindValue(QString::fromStdString(info.distanceToCenterField));
    q.addBindValue(QString::fromStdString(info.ballparkTypology));
    q.addBindValue(QString::fromStdString(info.roofType));
    q.addBindValue(originalTeamName);
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] updateStadiumInDB:" << q.lastError().text();
    return ok;
}

bool AdminWidget::deleteStadiumFromDB(const QString &teamName)
{
    QSqlQuery q(souvenirDB());
    q.prepare("DELETE FROM mlb_info WHERE trim(team_name)=trim(?)");
    q.addBindValue(teamName);
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] deleteStadiumFromDB:" << q.lastError().text();
    QSqlQuery q2(souvenirDB());
    q2.prepare("DELETE FROM souvenirs WHERE trim(team_name)=trim(?)");
    q2.addBindValue(teamName); q2.exec();
    return ok;
}

bool AdminWidget::insertStadiumInDB(const mlbInfo &info)
{
    QSqlQuery q(souvenirDB());
    q.prepare(R"(INSERT INTO mlb_info (team_name,stadium_name,seating_capacity,location,
        playing_surface,league,date_opened,distance_to_center_field,
        ballpark_typology,roof_type) VALUES (?,?,?,?,?,?,?,?,?,?))");
    q.addBindValue(QString::fromStdString(info.teamName));
    q.addBindValue(QString::fromStdString(info.stadiumName));
    q.addBindValue(info.seatingCapacity);
    q.addBindValue(QString::fromStdString(info.location));
    q.addBindValue(QString::fromStdString(info.playingSurface));
    q.addBindValue(QString::fromStdString(info.league));
    q.addBindValue(info.dateOpened);
    q.addBindValue(QString::fromStdString(info.distanceToCenterField));
    q.addBindValue(QString::fromStdString(info.ballparkTypology));
    q.addBindValue(QString::fromStdString(info.roofType));
    bool ok = q.exec();
    if (!ok) qWarning() << "[Admin] insertStadiumInDB:" << q.lastError().text();
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Load helpers
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::loadStadiumTable(const QString &filter)
{
    m_stadiumTable->setRowCount(0);
    for (const auto &info : m_db->GetMlbInfoVector()) {
        QString team    = QString::fromStdString(info.teamName).trimmed();
        QString stadium = QString::fromStdString(info.stadiumName);
        if (!filter.isEmpty() &&
            !team.contains(filter, Qt::CaseInsensitive) &&
            !stadium.contains(filter, Qt::CaseInsensitive))
            continue;
        int row = m_stadiumTable->rowCount();
        m_stadiumTable->insertRow(row);
        m_stadiumTable->setItem(row, 0, new QTableWidgetItem(team));
        m_stadiumTable->setItem(row, 1, new QTableWidgetItem(stadium));
        m_stadiumTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(info.league)));
        m_stadiumTable->setItem(row, 3, new QTableWidgetItem(QString::number(info.seatingCapacity)));
        m_stadiumTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(info.roofType)));
        m_stadiumTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(info.playingSurface)));
        m_stadiumTable->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(info.ballparkTypology)));
        m_stadiumTable->setItem(row, 7, new QTableWidgetItem(QString::number(info.dateOpened)));
    }
    m_stadiumTable->resizeColumnsToContents();
}

void AdminWidget::populateStadiumForm(int row)
{
    m_stadiumIsNew    = false;
    m_editingTeamName = m_stadiumTable->item(row, 0)->text().trimmed();
    m_edtTeamName->setText(m_editingTeamName);
    m_edtStadiumName->setText(m_stadiumTable->item(row, 1)->text());
    m_cmbLeague->setCurrentText(m_stadiumTable->item(row, 2)->text());
    m_spnCapacity->setValue(m_stadiumTable->item(row, 3)->text().toInt());
    m_cmbRoof->setCurrentText(m_stadiumTable->item(row, 4)->text());
    m_cmbSurface->setCurrentText(m_stadiumTable->item(row, 5)->text());
    m_cmbTypology->setCurrentText(m_stadiumTable->item(row, 6)->text());
    m_spnYearOpened->setValue(m_stadiumTable->item(row, 7)->text().toInt());
    for (const auto &info : m_db->GetMlbInfoVector()) {
        if (QString::fromStdString(info.teamName).trimmed() == m_editingTeamName) {
            m_edtLocation->setText(QString::fromStdString(info.location));
            m_edtCenterField->setText(QString::fromStdString(info.distanceToCenterField));
            break;
        }
    }
    m_btnDeleteStadium->setEnabled(true);
}

void AdminWidget::clearStadiumForm()
{
    m_edtTeamName->clear(); m_edtStadiumName->clear();
    m_edtLocation->clear(); m_edtCenterField->clear();
    m_spnCapacity->setValue(40000); m_spnYearOpened->setValue(2000);
    m_cmbLeague->setCurrentIndex(0); m_cmbRoof->setCurrentIndex(0);
    m_cmbSurface->setCurrentIndex(0); m_cmbTypology->setCurrentIndex(0);
    m_btnDeleteStadium->setEnabled(false);
    m_stadiumIsNew = false; m_editingTeamName.clear();
    m_stadiumTable->clearSelection();
}

void AdminWidget::loadSouvenirTable(const QString &teamName)
{
    m_souvenirTable->setRowCount(0);
    for (const Souvenir &s : souvenirListForTeam(teamName)) {
        int row = m_souvenirTable->rowCount();
        m_souvenirTable->insertRow(row);
        m_souvenirTable->setItem(row, 0, new QTableWidgetItem(s.itemName));
        m_souvenirTable->setItem(row, 1, new QTableWidgetItem(s.category));
        m_souvenirTable->setItem(row, 2, new QTableWidgetItem(
            QString("$%1").arg(s.price, 0, 'f', 2)));
        m_souvenirTable->setItem(row, 3, new QTableWidgetItem(QString::number(s.id)));
    }
    m_souvenirTable->resizeColumnsToContents();
    clearSouvenirForm();
}

void AdminWidget::populateSouvenirForm(int row)
{
    m_edtSouvenirItem->setText(m_souvenirTable->item(row, 0)->text());
    m_cmbSouvenirCategory->setCurrentText(m_souvenirTable->item(row, 1)->text());
    QString p = m_souvenirTable->item(row, 2)->text(); p.remove('$');
    m_spnSouvenirPrice->setValue(p.toDouble());
    m_editingSouvenirId = m_souvenirTable->item(row, 3)->text().toInt();
    m_btnDeleteSouvenir->setEnabled(true);
}

void AdminWidget::clearSouvenirForm()
{
    m_edtSouvenirItem->clear(); m_spnSouvenirPrice->setValue(9.99);
    m_cmbSouvenirCategory->setCurrentIndex(0);
    m_editingSouvenirId = -1; m_btnDeleteSouvenir->setEnabled(false);
    m_souvenirTable->clearSelection();
}

void AdminWidget::loadPricingTable(const QString &teamName)
{
    m_pricingCache = souvenirListForTeam(teamName);
    m_pricingTable->setRowCount(0);
    for (const Souvenir &s : m_pricingCache) {
        int row = m_pricingTable->rowCount();
        m_pricingTable->insertRow(row);
        auto *ni = new QTableWidgetItem(s.itemName);
        ni->setFlags(ni->flags() & ~Qt::ItemIsEditable);
        auto *ci = new QTableWidgetItem(s.category);
        ci->setFlags(ci->flags() & ~Qt::ItemIsEditable);
        auto *pi = new QTableWidgetItem(QString::number(s.price, 'f', 2));
        // price cell is editable (default flags kept)
        auto *ii = new QTableWidgetItem(QString::number(s.id));
        ii->setFlags(Qt::NoItemFlags);
        m_pricingTable->setItem(row, 0, ni);
        m_pricingTable->setItem(row, 1, ci);
        m_pricingTable->setItem(row, 2, pi);
        m_pricingTable->setItem(row, 3, ii);
    }
    m_pricingTable->resizeColumnsToContents();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Stadiums
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::onStadiumSelectionChanged()
{
    auto rows = m_stadiumTable->selectionModel()->selectedRows();
    if (!rows.isEmpty()) populateStadiumForm(rows.first().row());
}

void AdminWidget::onStadiumSearchChanged(const QString &text)
{
    loadStadiumTable(text.trimmed());
}

void AdminWidget::onAddNewStadium()
{
    clearStadiumForm();
    m_stadiumIsNew = true;
    m_edtTeamName->setFocus();
}

void AdminWidget::onSaveStadium()
{
    mlbInfo info;
    info.teamName              = m_edtTeamName->text().trimmed().toStdString();
    info.stadiumName           = m_edtStadiumName->text().trimmed().toStdString();
    info.location              = m_edtLocation->text().trimmed().toStdString();
    info.league                = m_cmbLeague->currentText().toStdString();
    info.roofType              = m_cmbRoof->currentText().toStdString();
    info.playingSurface        = m_cmbSurface->currentText().toStdString();
    info.ballparkTypology      = m_cmbTypology->currentText().toStdString();
    info.seatingCapacity       = m_spnCapacity->value();
    info.dateOpened            = m_spnYearOpened->value();
    info.distanceToCenterField = m_edtCenterField->text().trimmed().toStdString();

    if (info.teamName.empty() || info.stadiumName.empty()) {
        QMessageBox::warning(this, "Validation", "Team and stadium name are required.");
        return;
    }

    bool ok = m_stadiumIsNew ? insertStadiumInDB(info)
                             : updateStadiumInDB(info, m_editingTeamName);
    if (ok) {
        auto &vec = m_db->GetMlbInfoVector();
        if (m_stadiumIsNew) {
            vec.push_back(info);
        } else {
            for (auto &v : vec)
                if (QString::fromStdString(v.teamName).trimmed() == m_editingTeamName)
                    { v = info; break; }
        }
        loadStadiumTable(m_stadiumSearch->text());
        clearStadiumForm();
        QMessageBox::information(this, "Saved", "Stadium saved successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save stadium.");
    }
}

void AdminWidget::onDeleteStadium()
{
    if (m_editingTeamName.isEmpty()) return;
    if (QMessageBox::question(this, "Confirm",
        QString("Remove %1 and all its souvenirs?").arg(m_editingTeamName),
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;

    if (deleteStadiumFromDB(m_editingTeamName)) {
        auto &vec = m_db->GetMlbInfoVector();
        vec.erase(std::remove_if(vec.begin(), vec.end(), [&](const mlbInfo &v){
            return QString::fromStdString(v.teamName).trimmed() == m_editingTeamName;
        }), vec.end());
        loadStadiumTable(m_stadiumSearch->text());
        clearStadiumForm();
        emit souvenirDataChanged();
    }
}

void AdminWidget::onClearStadiumForm() { clearStadiumForm(); }

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Souvenirs
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::onSouvenirTeamChanged(int)
{
    loadSouvenirTable(m_cmbSouvenirTeam->currentText());
}

void AdminWidget::onSouvenirSelectionChanged()
{
    auto rows = m_souvenirTable->selectionModel()->selectedRows();
    if (!rows.isEmpty()) populateSouvenirForm(rows.first().row());
}

void AdminWidget::onSaveSouvenir()
{
    QString item = m_edtSouvenirItem->text().trimmed();
    if (item.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Item name cannot be empty."); return;
    }
    Souvenir s;
    s.teamName = m_cmbSouvenirTeam->currentText();
    s.itemName = item;
    s.price    = m_spnSouvenirPrice->value();
    s.category = m_cmbSouvenirCategory->currentText();
    s.id       = m_editingSouvenirId;

    bool ok = (s.id == -1) ? insertSouvenir(s) : updateSouvenir(s);
    if (ok) {
        loadSouvenirTable(s.teamName);
        if (m_cmbPricingTeam->currentText() == s.teamName)
            loadPricingTable(s.teamName);
        emit souvenirDataChanged();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save souvenir.");
    }
}

void AdminWidget::onDeleteSouvenir()
{
    if (m_editingSouvenirId == -1) return;
    if (QMessageBox::question(this, "Confirm", "Remove this souvenir item?",
        QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    if (deleteSouvenir(m_editingSouvenirId)) {
        QString team = m_cmbSouvenirTeam->currentText();
        loadSouvenirTable(team);
        if (m_cmbPricingTeam->currentText() == team) loadPricingTable(team);
        emit souvenirDataChanged();
    }
}

void AdminWidget::onClearSouvenirForm() { clearSouvenirForm(); }

// ─────────────────────────────────────────────────────────────────────────────
//  Slots — Pricing
// ─────────────────────────────────────────────────────────────────────────────

void AdminWidget::onPricingTeamChanged(int)
{
    loadPricingTable(m_cmbPricingTeam->currentText());
}

void AdminWidget::onApplyPctIncrease()
{
    double pct = m_spnPctIncrease->value() / 100.0;
    for (int row = 0; row < m_pricingTable->rowCount(); ++row) {
        auto *item = m_pricingTable->item(row, 2);
        if (!item) continue;
        double np = item->text().toDouble() * (1.0 + pct);
        item->setText(QString::number(qMax(0.01, np), 'f', 2));
    }
}

void AdminWidget::onSaveAllPrices()
{
    int saved = 0, failed = 0;
    for (int row = 0; row < m_pricingTable->rowCount(); ++row) {
        Souvenir s;
        s.id       = m_pricingTable->item(row, 3)->text().toInt();
        s.itemName = m_pricingTable->item(row, 0)->text();
        s.category = m_pricingTable->item(row, 1)->text();
        s.price    = m_pricingTable->item(row, 2)->text().toDouble();
        s.teamName = m_cmbPricingTeam->currentText();
        updateSouvenir(s) ? ++saved : ++failed;
    }
    loadPricingTable(m_cmbPricingTeam->currentText());
    if (m_cmbSouvenirTeam->currentText() == m_cmbPricingTeam->currentText())
        loadSouvenirTable(m_cmbSouvenirTeam->currentText());
    emit souvenirDataChanged();
    if (failed == 0)
        QMessageBox::information(this, "Saved", QString("%1 price(s) updated.").arg(saved));
    else
        QMessageBox::warning(this, "Partial", QString("%1 saved, %2 failed.").arg(saved).arg(failed));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Database Tab
// ─────────────────────────────────────────────────────────────────────────────

QWidget* AdminWidget::buildDatabaseTab()
{
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(16);

    // ── Helper: makes a card-style section ──
    auto makeCard = [](const QString &title, const QString &desc, QLayout *btnRow) -> QWidget* {
        auto *card = new QWidget;
        card->setStyleSheet(
            "QWidget {"
            "  background:#111f33;"
            "  border:1px solid #1a2d45;"
            "  border-radius:4px;"
            "}"
            "QLabel { border:none; background:transparent; }");
        auto *cl = new QVBoxLayout(card);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->setSpacing(6);

        auto *tl = new QLabel(title);
        tl->setStyleSheet("color:#ddeeff; font-size:13px; font-weight:600; border:none;");

        auto *dl = new QLabel(desc);
        dl->setStyleSheet("color:#4a6d8c; font-size:11px; border:none;");
        dl->setWordWrap(true);

        cl->addWidget(tl);
        cl->addWidget(dl);
        cl->addSpacing(4);
        cl->addLayout(btnRow);
        return card;
    };

    // ── Import new database ──────────────────────────────────────────────────
    auto *importBtnRow = new QHBoxLayout;
    importBtnRow->setSpacing(8);

    auto *btnImport = new QPushButton("Import mlb_info.db…");
    btnImport->setStyleSheet(
        "QPushButton{"
        "  background:#1e4a7a; color:#b8d4ec;"
        "  border:none; border-radius:3px;"
        "  padding:6px 16px; font-size:12px;"
        "}"
        "QPushButton:hover{ background:#255a90; color:#ffffff; }");
    btnImport->setCursor(Qt::PointingHandCursor);
    
    // UI only — backend team wires the actual file-copy logic
    // Changed to implement backend features as of 4/29/26
    connect(btnImport, &QPushButton::clicked, this, [this]()
    {
        QString filePath;
        QStringList importedTeams;
        QString errorMessage;

        filePath = QFileDialog::getOpenFileName(this,
                                            "Import New MLB Database",
                                            "",
                                            "SQLite Database (*.db);;All Files (*)");

        if (filePath.isEmpty())
        {
            return;
        }

        ensureSouvenirTable();

        if (!importNewStadiumsFromDatabase(souvenirDB(),
                                           filePath,
                                           importedTeams,
                                           errorMessage))
        {
            QMessageBox::critical(this,
                                  "Import Failed",
                                  "The selected database could not be imported.\n\n" +
                                  errorMessage);
            return;
        }

        m_db->CloseDB();
        m_db->OpenDB();

        refresh();

        QMessageBox::information(this,
                                 "Import Complete",
                                 QString("Imported/updated %1 stadium(s).\n\n%2")
                                     .arg(importedTeams.size())
                                     .arg(importedTeams.join("\n")));
    });

    importBtnRow->addWidget(btnImport);
    importBtnRow->addStretch();

    lay->addWidget(makeCard(
        "Import New Database",
        "Replace the current mlb_info.db with a new file. "
        "All stadium and souvenir data will be reloaded from the imported database.",
        importBtnRow));

    // ── Reset to initial state ───────────────────────────────────────────────
    auto *resetBtnRow = new QHBoxLayout;
    resetBtnRow->setSpacing(8);

    auto *btnReset = new QPushButton("Reset to Initial State");
    btnReset->setStyleSheet(
        "QPushButton{"
        "  background:#5c1a1a; color:#f0c0c0;"
        "  border:none; border-radius:3px;"
        "  padding:6px 16px; font-size:12px;"
        "}"
        "QPushButton:hover{ background:#722020; color:#ffffff; }");
    btnReset->setCursor(Qt::PointingHandCursor);
    connect(btnReset, &QPushButton::clicked, this, [this]()
    {
        QMessageBox::StandardButton reply;
        QString errorMessage;

        reply = QMessageBox::warning(this,
                                     "Reset Database",
                                     "This will delete all admin changes and restore the database\n"
                                     "to its original state. This cannot be undone.\n\n"
                                     "Continue?",
                                     QMessageBox::Yes | QMessageBox::Cancel);

        if (reply != QMessageBox::Yes)
        {
            return;
        }

        m_db->CloseDB();

        if (!resetActiveMlbDatabase(errorMessage))
        {
            m_db->OpenDB();

            QMessageBox::critical(this,
                                  "Reset Failed",
                                  "The active database could not be reset.\n\n" +
                                  errorMessage);
            return;
        }

        m_db->OpenDB();

        refresh();

        QMessageBox::information(this,
                                 "Reset Complete",
                                 "The active database has been restored from the original database.");
    });

    resetBtnRow->addWidget(btnReset);
    resetBtnRow->addStretch();

    lay->addWidget(makeCard(
        "Reset to Initial State",
        "Restore mlb_info.db to the original shipped database. "
        "Any stadiums added, removed, or edited through this admin panel will be lost.",
        resetBtnRow));

    lay->addStretch();
    return page;
}
