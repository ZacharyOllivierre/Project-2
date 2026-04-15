#include "TeamInfoWidget.h"

/**
 * Constructor — builds the layout once on creation.
 */
TeamInfoWidget::TeamInfoWidget(QWidget* parent) : QWidget(parent) {
    buildLayout();
}

/**
 * Updates all labels and souvenir list for the given team.
 */
void TeamInfoWidget::setTeam(const mlbInfo& team) {
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

    loadSouvenirs(getHardcodedSouvenirs(team.teamName));
}

/**
 * Builds the full right-panel layout.
 * Layout: detail card (header + info grid) stacked above souvenir card.
 */
void TeamInfoWidget::buildLayout() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Detail card ──
    QFrame* detailCard = new QFrame();
    detailCard->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout* detailLayout = new QVBoxLayout(detailCard);
    detailLayout->setSpacing(10);

    m_teamName = new QLabel("Select a team");
    m_teamName->setStyleSheet("font-size: 18px; font-weight: bold;");

    m_stadiumName = new QLabel("");
    m_stadiumName->setStyleSheet("color: gray; font-size: 13px;");

    detailLayout->addWidget(m_teamName);
    detailLayout->addWidget(m_stadiumName);

    // Info grid — 3 columns x 3 rows
    QGridLayout* infoGrid = new QGridLayout();
    infoGrid->setSpacing(8);

    infoGrid->addWidget(makeInfoCell("Location",     m_location), 0, 0);
    infoGrid->addWidget(makeInfoCell("League",       m_league),   0, 1);
    infoGrid->addWidget(makeInfoCell("Capacity",     m_capacity), 0, 2);
    infoGrid->addWidget(makeInfoCell("Opened",       m_opened),   1, 0);
    infoGrid->addWidget(makeInfoCell("Surface",      m_surface),  1, 1);
    infoGrid->addWidget(makeInfoCell("Center Field", m_center),   1, 2);
    infoGrid->addWidget(makeInfoCell("Park Type",    m_parkType), 2, 0);
    infoGrid->addWidget(makeInfoCell("Roof",         m_roof),     2, 1);

    detailLayout->addLayout(infoGrid);
    mainLayout->addWidget(detailCard);

    // ── Souvenir card ──
    QFrame* souvenirCard = new QFrame();
    souvenirCard->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout* souvenirLayout = new QVBoxLayout(souvenirCard);
    souvenirLayout->setSpacing(8);

    QLabel* souvenirTitle = new QLabel("Souvenirs");
    souvenirTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    souvenirLayout->addWidget(souvenirTitle);

    m_souvenirList = new QListWidget();
    m_souvenirList->setStyleSheet("border: none;");
    souvenirLayout->addWidget(m_souvenirList);

    mainLayout->addWidget(souvenirCard);
}

/**
 * Creates a styled info cell with a muted label and bold value.
 */
QFrame* TeamInfoWidget::makeInfoCell(const QString& label, QLabel*& valueLabel) {
    QFrame* cell = new QFrame();
    cell->setFrameShape(QFrame::StyledPanel);
    cell->setStyleSheet("background: #f5f5f5; border-radius: 6px; padding: 4px;");

    QVBoxLayout* cellLayout = new QVBoxLayout(cell);
    cellLayout->setSpacing(2);

    QLabel* fieldLabel = new QLabel(label);
    fieldLabel->setStyleSheet("font-size: 11px; color: gray;");

    valueLabel = new QLabel("—");
    valueLabel->setStyleSheet("font-size: 13px; font-weight: 500;");

    cellLayout->addWidget(fieldLabel);
    cellLayout->addWidget(valueLabel);

    return cell;
}

/**
 * Clears and repopulates the souvenir list.
 * SWAP: replace body with DB query when souvenir table is ready.
 */
void TeamInfoWidget::loadSouvenirs(const QList<SouvenirItem>& items) {
    m_souvenirList->clear();
    for (const auto& item : items) {
        QString row = QString("%1    $%2")
        .arg(item.name)
            .arg(item.price, 0, 'f', 2);
        m_souvenirList->addItem(row);
    }
}

/**
 * TEMPORARY hardcoded souvenirs.
 * DELETE this function when souvenir DB table is implemented.
 * Replace with: query souvenirs table WHERE teamName == team.teamName
 */
QList<SouvenirItem> TeamInfoWidget::getHardcodedSouvenirs(const std::string& teamName) {
    return {
            {"Baseball",  10.99},
            {"Hat",        7.99},
            {"Jersey",    89.99},
            {"Pennant",   14.99},
            };
}
