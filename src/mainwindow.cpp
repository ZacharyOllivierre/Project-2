/**
 * @file mainwindow.cpp
 * @brief Builds the main window UI, loads stadium distances, updates the cart button, and displays MST mileage.
 */

#include "mainwindow.h"
#include "teaminfowidget.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

mainwindow::mainwindow(Database* db, QWidget* parent)
    : QMainWindow(parent), m_database(db)
{
    setWindowTitle("MLB Planner");
    resize(1100, 700);

    m_purchaseWindow = new PurchaseWindow(&m_souvenirManager, this);
}

// Load every stadium distance into the graph structure
void mainwindow::loadStadiumDistances(const std::vector<stadiumDistances>& distances)
{
    for (const stadiumDistances& distance : distances)
    {
        m_stadiumGraph.addEdge(
            QString::fromStdString(distance.originatedStadium),
            QString::fromStdString(distance.destinationStadium),
            distance.distance
            );
    }
}

// Update the cart button to show current item count
void mainwindow::updateCartNotification()
{
    int count = m_souvenirManager.getTotalItemCount();

    if (m_viewPurchasesButton != nullptr)
    {
        m_viewPurchasesButton->setText(
            QString("View Purchase Screen (Cart: %1)").arg(count)
            );
    }
}

// Run Prim's algorithm and display the MST mileage
void mainwindow::showMSTResult()
{
    QStringList stadiums = m_stadiumGraph.getAllStadiums();

    if (stadiums.isEmpty())
    {
        QMessageBox::warning(
            this,
            "MST Error",
            "No stadium distance data was loaded."
            );
        return;
    }

    QString startStadium = stadiums.first();

    QList<GraphEdge> mstEdges = m_stadiumGraph.primMST(startStadium);
    int totalMileage = m_stadiumGraph.getTotalMileage(mstEdges);

    QString result;
    result += "Minimum Spanning Tree using Prim's Algorithm\n\n";
    result += "Starting Stadium: " + startStadium + "\n\n";

    for (const GraphEdge& edge : mstEdges)
    {
        result += QString("%1  ->  %2  :  %3 miles\n")
        .arg(edge.from)
            .arg(edge.to)
            .arg(edge.distance);
    }

    result += "\nAssociated MST Mileage: ";
    result += QString::number(totalMileage);
    result += " miles";

    QMessageBox::information(
        this,
        "Minimum Spanning Tree",
        result
        );
}

// Build the main UI layout and load team data
void mainwindow::loadTeams(const std::vector<mlbInfo>& teams)
{
    QWidget* central = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(central);
    layout->setSpacing(12);
    layout->setContentsMargins(12, 12, 12, 12);

    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_viewPurchasesButton = new QPushButton();
    m_viewPurchasesButton->setStyleSheet(
        "QPushButton {"
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
        "}"
        );

    updateCartNotification();

    m_mstButton = new QPushButton("Show MST Mileage");
    m_mstButton->setStyleSheet(
        "QPushButton {"
        "background-color: #16a34a;"
        "color: white;"
        "font-weight: 600;"
        "border: 1px solid #22c55e;"
        "border-radius: 8px;"
        "padding: 8px 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #22c55e;"
        "}"
        "QPushButton:pressed {"
        "background-color: #15803d;"
        "}"
        );

    QListWidget* teamList = new QListWidget();
    teamList->setMaximumWidth(240);

    for (const auto& team : teams)
    {
        teamList->addItem(QString::fromStdString(team.teamName));
    }

    leftLayout->addWidget(m_viewPurchasesButton);
    leftLayout->addWidget(m_mstButton);
    leftLayout->addWidget(teamList);

    TeamInfoWidget* teamInfo = new TeamInfoWidget(&m_souvenirManager, m_database);

    if (!teams.empty())
    {
        teamInfo->setTeam(teams[0]);
        teamList->setCurrentRow(0);
    }

    connect(teamList, &QListWidget::currentRowChanged, this,
            [teamInfo, teams](int row)
            {
                if (row >= 0 && row < static_cast<int>(teams.size()))
                {
                    teamInfo->setTeam(teams[row]);
                }
            });

    connect(m_viewPurchasesButton, &QPushButton::clicked, this,
            [this]()
            {
                if (m_purchaseWindow != nullptr)
                {
                    m_purchaseWindow->refreshScreen();
                    m_purchaseWindow->show();
                    m_purchaseWindow->raise();
                    m_purchaseWindow->activateWindow();
                }
            });

    connect(m_mstButton, &QPushButton::clicked, this,
            [this]()
            {
                showMSTResult();
            });

    connect(teamInfo, &TeamInfoWidget::cartUpdated, this,
            [this]()
            {
                updateCartNotification();
            });

    layout->addWidget(leftPanel);
    layout->addWidget(teamInfo, 1);

    setCentralWidget(central);
}
