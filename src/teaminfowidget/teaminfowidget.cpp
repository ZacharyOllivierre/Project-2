#include "teaminfowidget.h"
#include <QSqlDatabase>
#include <QSqlQuery>

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

TeamInfoWidget::TeamInfoWidget(SouvenirManager *manager, Database *db, QWidget *parent)
    : QWidget(parent)
    , m_souvenirManager(manager)
    , m_db(db)
{
    buildLayout();
}

// ─────────────────────────────────────────────────────────────────────────────
//  setTeam
// ─────────────────────────────────────────────────────────────────────────────

void TeamInfoWidget::setTeam(const mlbInfo &team)
{
    m_currentTeamName   = team.teamName;
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

// ─────────────────────────────────────────────────────────────────────────────
//  reloadSouvenirs  (slot — called after admin edits)
// ─────────────────────────────────────────────────────────────────────────────

void TeamInfoWidget::reloadSouvenirs()
{
    if (m_currentTeamName.empty()) return;
    m_currentSouvenirs = getSouvenirs(m_currentTeamName);
    loadSouvenirs(m_currentSouvenirs);
}

// ─────────────────────────────────────────────────────────────────────────────
//  getSouvenirs  —  live DB query, hardcoded fallback
// ─────────────────────────────────────────────────────────────────────────────

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
        if (q.exec()) {
            while (q.next())
                items.append({q.value(0).toString(), q.value(1).toDouble()});
        }
    }

    // If the souvenirs table is empty for this team, fall back to defaults
    // so the UI is never blank before the admin has added anything
    if (items.isEmpty()) {
        if (qTeam.contains("Dodgers", Qt::CaseInsensitive))
            return {{"Dodgers Cap", 19.99}, {"Baseball Bat", 89.39},
                    {"Team Pennant", 17.99}, {"Autographed Baseball", 29.99},
                    {"Team Jersey", 199.99}};
        if (qTeam.contains("Yankees", Qt::CaseInsensitive))
            return {{"Yankees Cap", 21.99}, {"Baseball Bat", 89.39},
                    {"Team Pennant", 17.99}, {"Autographed Baseball", 34.99},
                    {"Team Jersey", 209.99}};
        // Generic fallback
        return {{"Baseball Cap", 19.99}, {"Baseball Bat", 89.39},
                {"Team Pennant", 17.99}, {"Autographed Baseball", 29.99},
                {"Team Jersey", 199.99}};
    }

    return items;
}

// ─────────────────────────────────────────────────────────────────────────────
//  buildLayout  (unchanged from original)
// ─────────────────────────────────────────────────────────────────────────────

void TeamInfoWidget::buildLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    QFrame *detailCard = new QFrame();
    QVBoxLayout *detailLayout = new QVBoxLayout(detailCard);

    m_teamName = new QLabel("Select a Team");
    m_teamName->setStyleSheet("font-size: 20px; font-weight: bold; color: white;");

    m_stadiumName = new QLabel("");
    m_stadiumName->setStyleSheet("font-size: 13px; color: #cbd5e1;");

    detailLayout->addWidget(m_teamName);
    detailLayout->addWidget(m_stadiumName);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(8);
    grid->addWidget(makeInfoCell("Location",     m_location), 0, 0);
    grid->addWidget(makeInfoCell("League",       m_league),   0, 1);
    grid->addWidget(makeInfoCell("Capacity",     m_capacity), 0, 2);
    grid->addWidget(makeInfoCell("Opened",       m_opened),   1, 0);
    grid->addWidget(makeInfoCell("Surface",      m_surface),  1, 1);
    grid->addWidget(makeInfoCell("Center Field", m_center),   1, 2);
    grid->addWidget(makeInfoCell("Park Type",    m_parkType), 2, 0);
    grid->addWidget(makeInfoCell("Roof",         m_roof),     2, 1);
    detailLayout->addLayout(grid);
    mainLayout->addWidget(detailCard);

    QFrame *souvenirCard = new QFrame();
    QVBoxLayout *souvenirLayout = new QVBoxLayout(souvenirCard);

    QLabel *souvenirTitle = new QLabel("Souvenirs");
    souvenirTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");

    m_souvenirList = new QListWidget();

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    QLabel *quantityLabel = new QLabel("Quantity:");
    quantityLabel->setStyleSheet("color: white; font-weight: 600;");

    m_quantitySpinBox = new QSpinBox();
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(100);
    m_quantitySpinBox->setValue(1);
    m_quantitySpinBox->setStyleSheet(
        "QSpinBox { background-color:#1b2333; color:white; border:1px solid #2f3b52;"
        "border-radius:6px; padding:4px; }");

    m_buyButton = new QPushButton("Buy Souvenir");
    m_buyButton->setStyleSheet(
        "QPushButton { background-color:#2563eb; color:white; font-weight:600;"
        "border:1px solid #3b82f6; border-radius:8px; padding:8px 14px; }"
        "QPushButton:hover { background-color:#3b82f6; }"
        "QPushButton:pressed { background-color:#1d4ed8; }");

    controlsLayout->addWidget(quantityLabel);
    controlsLayout->addWidget(m_quantitySpinBox);
    controlsLayout->addStretch();
    controlsLayout->addWidget(m_buyButton);

    souvenirLayout->addWidget(souvenirTitle);
    souvenirLayout->addWidget(m_souvenirList);
    souvenirLayout->addLayout(controlsLayout);
    mainLayout->addWidget(souvenirCard);

    connect(m_buyButton, &QPushButton::clicked, this, [this]() { buySelectedSouvenir(); });
}

QFrame *TeamInfoWidget::makeInfoCell(const QString &label, QLabel *&valueLabel)
{
    QFrame *cell = new QFrame();
    cell->setStyleSheet("background-color:#2563eb; border:1px solid #3b82f6;"
                        "border-radius:8px; padding:6px;");
    QVBoxLayout *layout = new QVBoxLayout(cell);
    layout->setSpacing(2);
    QLabel *fieldLabel = new QLabel(label);
    fieldLabel->setStyleSheet("font-size:11px; color:#dbeafe;");
    valueLabel = new QLabel("—");
    valueLabel->setStyleSheet("font-size:13px; font-weight:600; color:white;");
    layout->addWidget(fieldLabel);
    layout->addWidget(valueLabel);
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

    SouvenirItem selectedItem = m_currentSouvenirs[row];
    int quantity = m_quantitySpinBox->value();
    m_souvenirManager->buySouvenir(
        QString::fromStdString(m_currentStadiumName), selectedItem, quantity);
    emit cartUpdated();
}
