#include "mainwindow.h"
#include "teaminfowidget.h"

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
