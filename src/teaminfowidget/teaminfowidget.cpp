#include "teaminfowidget.h"
#include <QSqlDatabase>
#include <QSqlQuery>

TeamInfoWidget::TeamInfoWidget(SouvenirManager *manager, Database *db, QWidget *parent)
    : QWidget(parent), m_souvenirManager(manager), m_db(db)
{
    buildLayout();
}

void TeamInfoWidget::setTeam(const mlbInfo &team)
{
    m_currentTeamName    = team.teamName;
    m_currentStadiumName = team.stadiumName;

    m_teamName->setText(QString::fromStdString(team.teamName));
    m_stadiumName->setText(QString::fromStdString(team.stadiumName));
    m_capacity->setText(QString::number(team.seatingCapacity));
    m_location->setText(QString::fromStdString(team.location));
    m_surface->setText(QString::fromStdString(team.playingSurface));
    m_league->setText(QString::fromStdString(team.league));
    m_opened->setText(QString::number(team.dateOpened));
    m_center->setText(QString::fromStdString(team.distanceToCenterField));
    m_parkType->setText(QString::fromStdString(team.ballparkTypology));
    m_roof->setText(QString::fromStdString(team.roofType));

    m_currentSouvenirs = getSouvenirs(team.teamName);
    loadSouvenirs(m_currentSouvenirs);
}

void TeamInfoWidget::loadTeamList(const std::vector<mlbInfo> &teams)
{
    m_teams = teams;
    m_teamList->clear();
    for (const auto &t : teams)
        m_teamList->addItem(QString::fromStdString(t.teamName));
    if (!teams.empty()) {
        m_teamList->setCurrentRow(0);
        setTeam(teams[0]);
    }
}

void TeamInfoWidget::reloadSouvenirs()
{
    if (m_currentTeamName.empty()) return;
    m_currentSouvenirs = getSouvenirs(m_currentTeamName);
    loadSouvenirs(m_currentSouvenirs);
}

QList<SouvenirItem> TeamInfoWidget::getSouvenirs(const std::string &teamName)
{
    QList<SouvenirItem> items;
    QString qTeam = QString::fromStdString(teamName).trimmed();

    QSqlDatabase db = QSqlDatabase::database("MLB Info Database");
    if (db.isOpen()) {
        QSqlQuery q(db);
        q.prepare("SELECT item_name, price FROM souvenirs "
                  "WHERE trim(team_name)=trim(?) ORDER BY item_name");
        q.addBindValue(qTeam);
        if (q.exec())
            while (q.next())
                items.append({q.value(0).toString(), q.value(1).toDouble()});
    }

    if (items.isEmpty()) {
        if (qTeam.contains("Dodgers", Qt::CaseInsensitive))
            return {{"Dodgers Cap", 19.99}, {"Baseball Bat", 89.39},
                    {"Team Pennant", 17.99}, {"Autographed Baseball", 29.99},
                    {"Team Jersey", 199.99}};
        if (qTeam.contains("Yankees", Qt::CaseInsensitive))
            return {{"Yankees Cap", 21.99}, {"Baseball Bat", 89.39},
                    {"Team Pennant", 17.99}, {"Autographed Baseball", 34.99},
                    {"Team Jersey", 209.99}};
        return {{"Baseball Cap", 19.99}, {"Baseball Bat", 89.39},
                {"Team Pennant", 17.99}, {"Autographed Baseball", 29.99},
                {"Team Jersey", 199.99}};
    }
    return items;
}

void TeamInfoWidget::buildLayout()
{
    setStyleSheet(R"(
        QWidget { background:#0d1c2e; color:#b8d4ec;
                  font-family:'Segoe UI',sans-serif; font-size:12px; }
        QLabel  { background:transparent; border:none; color:#b8d4ec; }
        QListWidget { background:#0a1628; border:1px solid #1a2d45;
                      border-radius:3px; outline:none; }
        QListWidget::item { padding:7px 12px; color:#b8d4ec; border:none; }
        QListWidget::item:selected { background:#1a3a5c; color:#ffffff; }
        QListWidget::item:alternate { background:#0d1f35; }
        QSpinBox { background:#0a1628; border:1px solid #1a2d45;
                   border-radius:3px; color:#b8d4ec; padding:4px 6px; }
        QScrollBar:vertical { background:#0a1628; width:7px; border:none; }
        QScrollBar::handle:vertical { background:#1a2d45; border-radius:3px; min-height:20px; }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }
    )");

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Left: team list ───────────────────────────────────────────────────────
    auto *listPanel = new QWidget;
    listPanel->setFixedWidth(200);
    listPanel->setStyleSheet("background:#111f33; border-right:1px solid #1a2d45;");
    auto *listLay = new QVBoxLayout(listPanel);
    listLay->setContentsMargins(0, 0, 0, 0);
    listLay->setSpacing(0);

    auto *listHeader = new QLabel("  Teams");
    listHeader->setFixedHeight(36);
    listHeader->setStyleSheet(
        "color:#ddeeff; font-size:12px; font-weight:600; border:none;"
        "background:#162035; border-bottom:1px solid #1a2d45; padding-left:10px;");
    listLay->addWidget(listHeader);

    m_teamList = new QListWidget;
    m_teamList->setAlternatingRowColors(true);
    m_teamList->setStyleSheet(
        "QListWidget { background:#111f33; border:none; border-radius:0px; }"
        "QListWidget::item { padding:8px 12px; color:#7aa0c0; border:none; }"
        "QListWidget::item:selected { background:#162a45; color:#ffffff; border-left:2px solid #4a9ade; }"
        "QListWidget::item:alternate { background:#0f1c30; }"
        "QListWidget::item:hover { background:#13253d; color:#b8d4ec; }");
    listLay->addWidget(m_teamList, 1);

    connect(m_teamList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0 && row < (int)m_teams.size())
            setTeam(m_teams[row]);
    });

    root->addWidget(listPanel);

    // ── Right: detail + souvenirs ─────────────────────────────────────────────
    auto *rightPanel = new QWidget;
    rightPanel->setStyleSheet("background:#0d1c2e;");
    auto *rightLay = new QVBoxLayout(rightPanel);
    rightLay->setContentsMargins(16, 16, 16, 16);
    rightLay->setSpacing(10);

    m_teamName = new QLabel("Select a Team");
    m_teamName->setStyleSheet("color:#ffffff; font-size:18px; font-weight:700; border:none;");
    m_stadiumName = new QLabel("");
    m_stadiumName->setStyleSheet("color:#7aa0c0; font-size:13px; border:none;");
    rightLay->addWidget(m_teamName);
    rightLay->addWidget(m_stadiumName);

    auto *grid = new QGridLayout;
    grid->setSpacing(8);
    grid->addWidget(makeInfoCell("Location",     m_location), 0, 0);
    grid->addWidget(makeInfoCell("League",       m_league),   0, 1);
    grid->addWidget(makeInfoCell("Capacity",     m_capacity), 0, 2);
    grid->addWidget(makeInfoCell("Opened",       m_opened),   1, 0);
    grid->addWidget(makeInfoCell("Surface",      m_surface),  1, 1);
    grid->addWidget(makeInfoCell("Center Field", m_center),   1, 2);
    grid->addWidget(makeInfoCell("Park Type",    m_parkType), 2, 0);
    grid->addWidget(makeInfoCell("Roof",         m_roof),     2, 1);
    rightLay->addLayout(grid);

    auto *div = new QWidget;
    div->setFixedHeight(1);
    div->setStyleSheet("background:#1a2d45; border:none;");
    rightLay->addWidget(div);

    auto *souvenirHeader = new QLabel("Souvenirs");
    souvenirHeader->setStyleSheet("color:#ddeeff; font-size:14px; font-weight:600; border:none;");
    rightLay->addWidget(souvenirHeader);

    m_souvenirList = new QListWidget;
    m_souvenirList->setAlternatingRowColors(true);
    m_souvenirList->setMaximumHeight(160);
    rightLay->addWidget(m_souvenirList);

    auto *buyRow = new QHBoxLayout;
    auto *qtyLbl = new QLabel("Quantity:");
    qtyLbl->setStyleSheet("color:#7aa0c0; font-size:11px; border:none;");
    m_quantitySpinBox = new QSpinBox;
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(100);
    m_quantitySpinBox->setValue(1);
    m_quantitySpinBox->setFixedWidth(70);
    m_buyButton = new QPushButton("Buy Souvenir");
    m_buyButton->setStyleSheet(
        "QPushButton { background:#1e4a7a; color:#c8e0f4; border:none; border-radius:3px;"
        "  padding:6px 16px; font-size:12px; }"
        "QPushButton:hover { background:#255a90; color:#ffffff; }");
    m_buyButton->setCursor(Qt::PointingHandCursor);
    buyRow->addWidget(qtyLbl);
    buyRow->addWidget(m_quantitySpinBox);
    buyRow->addStretch();
    buyRow->addWidget(m_buyButton);
    rightLay->addLayout(buyRow);
    rightLay->addStretch();

    connect(m_buyButton, &QPushButton::clicked, this, [this]{ buySelectedSouvenir(); });

    root->addWidget(rightPanel, 1);
}

QWidget* TeamInfoWidget::makeInfoCell(const QString &label, QLabel *&valueLabel)
{
    auto *cell = new QWidget;
    cell->setStyleSheet(
        "QWidget { background:#111f33; border:1px solid #1a2d45; border-radius:4px; }"
        "QLabel { border:none; background:transparent; }");
    auto *lay = new QVBoxLayout(cell);
    lay->setContentsMargins(10, 7, 10, 7);
    lay->setSpacing(2);
    auto *lbl = new QLabel(label);
    lbl->setStyleSheet("color:#4a6d8c; font-size:10px; letter-spacing:0.5px;");
    valueLabel = new QLabel("—");
    valueLabel->setStyleSheet("color:#ddeeff; font-size:12px; font-weight:600;");
    lay->addWidget(lbl);
    lay->addWidget(valueLabel);
    return cell;
}

void TeamInfoWidget::loadSouvenirs(const QList<SouvenirItem> &items)
{
    m_souvenirList->clear();
    for (const auto &item : items)
        m_souvenirList->addItem(
            QString("%1    $%2").arg(item.name).arg(item.price, 0, 'f', 2));
    if (!items.isEmpty())
        m_souvenirList->setCurrentRow(0);
}

void TeamInfoWidget::buySelectedSouvenir()
{
    if (!m_souvenirManager) return;
    int row = m_souvenirList->currentRow();
    if (row < 0 || row >= m_currentSouvenirs.size()) return;
    if (m_currentStadiumName.empty()) return;
    m_souvenirManager->buySouvenir(
        QString::fromStdString(m_currentStadiumName),
        m_currentSouvenirs[row],
        m_quantitySpinBox->value());
    emit cartUpdated();
}
