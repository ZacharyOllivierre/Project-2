#include "browseWidget.h"
#include "ui_browsewidget.h"

BrowseWidget::BrowseWidget(const vector<mlbInfo>& teams, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BrowseWidget)
{
    ui->setupUi(this);

    // Set current teams
    mInfo = teams;

    // Set headers for tables
    headers = {
        "Team Name", "Stadium Name", "Capacity", "Location",
        "Surface", "League", "Opened", "Center Field",
        "Typology", "Roof Type"
    };

    // Set default sort type
    sortType = SortType::TeamName;

    // init combo boxes - prevent signals for combo box changed while intiiallizing
    ui->leagueTypeCombo->blockSignals(true);
    ui->roofTypeCombo->blockSignals(true);
    ui->sortTypeCombo->blockSignals(true);

    initLeagueTypeCombo();
    initRoofTypeCombo();
    initSortTypeCombo();

    ui->leagueTypeCombo->blockSignals(false);
    ui->roofTypeCombo->blockSignals(false);
    ui->sortTypeCombo->blockSignals(false);

    // init table
    initTable();

    // populate table
    populateTable();
}

BrowseWidget::~BrowseWidget()
{
    delete ui;
}

// Init league type combo box with all unique league types found in mlb info
void BrowseWidget::initLeagueTypeCombo()
{
    set<string> uniqueLeagues;

    uniqueLeagues.insert("All Leagues");

    for (auto &team : mInfo)
    {
        uniqueLeagues.insert(team.league);
    }

    for (auto& league : uniqueLeagues)
    {
        ui->leagueTypeCombo->addItem(QString::fromStdString(league));
    }
}

// Inits roof type combo box with unique roof types found in mlb info
void BrowseWidget::initRoofTypeCombo()
{
    set<string> uniqueRoofs;

    uniqueRoofs.insert("All Roof Types");

    for (auto& team : mInfo)
    {
        uniqueRoofs.insert(team.roofType);
    }

    for (auto& roof : uniqueRoofs)
    {
        ui->roofTypeCombo->addItem(QString::fromStdString(roof));
    }
}

// Inits sort type combo with predetermined SortType options
void BrowseWidget::initSortTypeCombo()
{
    ui->sortTypeCombo->addItem("Team Name");
    ui->sortTypeCombo->addItem("Stadium Name");
    ui->sortTypeCombo->addItem("Seating Capacity");
    ui->sortTypeCombo->addItem("Date Opened");
    ui->sortTypeCombo->addItem("Distance to Center Field");
}

void BrowseWidget::initTable()
{
    ui->table->setColumnCount(headers.count());
    ui->table->setHorizontalHeaderLabels(headers);

    ui->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table->setSelectionBehavior(QAbstractItemView::SelectRows);
}

// Populates table with sub set of teams / in specified sort type
void BrowseWidget::populateTable()
{
    // Filter types
    QString selectedLeague = ui->leagueTypeCombo->currentText();
    QString selectedRoof = ui->roofTypeCombo->currentText();

    // Get list of teams that match filter requirements
    vector<mlbInfo> filteredList;
    for (auto& team : mInfo)
    {
        // Check if current team matches in both league and roof type
        bool matchesLeague = (selectedLeague == "All Leagues" ||
                              selectedLeague == QString::fromStdString(team.league));

        bool matchesRoof = (selectedRoof == "All Roof Types" ||
                            selectedRoof == QString::fromStdString(team.roofType));

        if (matchesLeague && matchesRoof)
        {
            filteredList.push_back(team);
        }
    }

    // Get sorted list
    sortData(filteredList);

    // Populate table with sorted / filtered list
    ui->table->setRowCount(filteredList.size());
    ui->table->clearContents();

    for (int row = 0; row < filteredList.size(); row++)
    {
        addRow(row, filteredList[row]);
    }

}

// Adds row to table
void BrowseWidget::addRow(int row, mlbInfo& t)
{
    QVector<QString> values =
        {
            QString::fromStdString(t.teamName),
            QString::fromStdString(t.stadiumName),
            QString::number(t.seatingCapacity),
            QString::fromStdString(t.location),
            QString::fromStdString(t.playingSurface),
            QString::fromStdString(t.league),
            QString::number(t.dateOpened),
            QString::fromStdString(t.distanceToCenterField),
            QString::fromStdString(t.ballparkTypology),
            QString::fromStdString(t.roofType)
        };

    for (int col = 0; col < values.size(); col++)
    {
        ui->table->setItem(row, col, new QTableWidgetItem(values[col]));
    }
}

// Sorts given mlb info list by sort type - switch to db sort
void BrowseWidget::sortData(vector<mlbInfo>& data)
{
    switch (sortType)
    {
    case SortType::TeamName:

        sort(data.begin(), data.end(),
             [](const mlbInfo& a, const mlbInfo& b)
             {return a.teamName < b.teamName; });

        break;

    case SortType::StadiumName:

        sort(data.begin(), data.end(),
             [](const mlbInfo& a, const mlbInfo& b)
             {return a.stadiumName < b.stadiumName;});

        break;

    case SortType::SeatingCapacity:

        sort(data.begin(), data.end(),
             [](const mlbInfo& a, const mlbInfo& b)
             {return a.seatingCapacity < b.seatingCapacity;});

        break;

    case SortType::DateOpened:

        sort(data.begin(), data.end(),
             [](const mlbInfo& a, const mlbInfo& b)
             {return a.dateOpened < b.dateOpened;});

        break;

    case SortType::DistanceToCenter:

        sort(data.begin(), data.end(),
             [](const mlbInfo& a, const mlbInfo& b)
             {return std::stoi(a.distanceToCenterField) < std::stoi(b.distanceToCenterField);});

        break;
    }
}

void BrowseWidget::on_leagueTypeCombo_currentIndexChanged(int index)
{
    populateTable();
}


void BrowseWidget::on_roofTypeCombo_currentIndexChanged(int index)
{
    populateTable();
}

void BrowseWidget::on_sortTypeCombo_currentIndexChanged(int index)
{
    sortType = static_cast<SortType>(index);
    populateTable();
}


