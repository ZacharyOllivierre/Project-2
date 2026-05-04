#include "mainwindow.h"
#include "../teaminfowidget/teaminfowidget.h"
#include "../admin/adminwidget.h"
#include "../browse/browsewidget.h"
#include "../homepage/homepage.h"

#include <QCryptographicHash>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>

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

    // Page 0 — Home (No longer blank yippeee - Brandon)
    homepage *home = new homepage;
    m_homePage = home;
    m_homePage->setStyleSheet("background:#0d1c2e;");
    m_stack->addWidget(m_homePage);

    // Page 1 — Team Info
    m_teamInfoPage = new TeamInfoWidget(&m_souvenirManager, m_db);
    if (!teams.empty())
        m_teamInfoPage->setTeam(teams[0]);
    connect(m_teamInfoPage, &TeamInfoWidget::cartUpdated,
            this, &mainwindow::updateCartNotification);
    m_stack->addWidget(m_teamInfoPage);

    // Page 2 — Browse
    m_browsePage = new BrowseWidget(teams);
    m_browsePage->hide();   // prevent it auto-showing before user navigates to it
    m_stack->addWidget(m_browsePage);

    // Page 3 — Plan a Trip placeholder
    auto *tripPage = new QWidget;
    tripPage->setStyleSheet("background:#0d1c2e;");
    m_stack->addWidget(tripPage);

    // Page 4 — Admin
    m_adminPage = new AdminWidget(m_db, this);
    m_adminPage->refresh();
    connect(m_adminPage, &AdminWidget::souvenirDataChanged,
            m_teamInfoPage, &TeamInfoWidget::reloadSouvenirs);
    m_stack->addWidget(m_adminPage);

    root->addWidget(m_stack, 1);

    // Default to Home — Browse won't appear until user clicks it
    setActivePage(m_homePage, m_navHome);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Sidebar
// ─────────────────────────────────────────────────────────────────────────────

QWidget* mainwindow::buildSidebar()
{
    QWidget *sidebar = new QWidget;
    sidebar->setFixedWidth(200);
    // Sidebar is one tone lighter than page bg — it visually "sits in front"
    sidebar->setStyleSheet("background:#111f33;");

    QVBoxLayout *lay = new QVBoxLayout(sidebar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // Logo — lightest strip, topmost layer
    QWidget *logo = new QWidget;
    logo->setFixedHeight(52);
    // border:none on every element — prevents Qt from drawing a default focus/widget border
    logo->setStyleSheet("QWidget { background:#162035; border:none; }");
    QVBoxLayout *logoLay = new QVBoxLayout(logo);
    logoLay->setContentsMargins(14, 0, 14, 0);
    QLabel *logoTitle = new QLabel("⚾  MLB Planner");
    logoTitle->setStyleSheet("color:#ffffff; font-size:14px; font-weight:700; background:transparent; border:none;");
    logoLay->addWidget(logoTitle);
    lay->addWidget(logo);

    // Cart button — actionable, near-white text
    m_viewPurchasesButton = new QPushButton("View Purchase Screen (Cart: 0)");
    m_viewPurchasesButton->setStyleSheet(
        "QPushButton{"
        "  background:#1a3a60;"
        "  color:#d0e8ff;"
        "  border:none;"
        "  border-bottom:1px solid #1a2d45;"
        "  padding:9px 14px;"
        "  font-size:11px;"
        "  font-weight:600;"
        "  text-align:left;"
        "}"
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

    // Section label — very dark, recedes, just a structural divider
    auto addSection = [&](const QString &label) {
        QLabel *sec = new QLabel(label.toUpper());
        sec->setStyleSheet(
            "color:#2e4d6a;"
            "font-size:10px;"
            "letter-spacing:1.2px;"
            "padding:12px 14px 3px;");
        lay->addWidget(sec);
    };

    m_navHome     = new QPushButton("  Home");
    m_navTeamInfo = new QPushButton("  Team Info");
    m_navBrowse   = new QPushButton("  Browse");
    m_navPlanTrip = new QPushButton("  Plan a Trip");
    m_navAdmin    = new QPushButton("  Manage Data");

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
    lay->addWidget(m_navPlanTrip);

    addSection("Admin");
    styleNavBtn(m_navAdmin);
    lay->addWidget(m_navAdmin);

    lay->addStretch();

    connect(m_navHome,     &QPushButton::clicked, this, [this]{ setActivePage(m_homePage,         m_navHome); });
    connect(m_navTeamInfo, &QPushButton::clicked, this, [this]{ setActivePage(m_teamInfoPage,     m_navTeamInfo); });
    connect(m_navBrowse,   &QPushButton::clicked, this, [this]{ setActivePage(m_browsePage,       m_navBrowse); });
    connect(m_navPlanTrip, &QPushButton::clicked, this, [this]{ setActivePage(m_stack->widget(3), m_navPlanTrip); });
    connect(m_navAdmin, &QPushButton::clicked, this, [this]
    {
        bool ok;
        QString password;
        QString hash;
        QString expectedHash;

        ok = false;

        password = QInputDialog::getText(this,
                                         "Admin Login",
                                         "Enter administrator password:",
                                         QLineEdit::Password,
                                         "",
                                         &ok);

        if (!ok)
        {
            return;
        }

        hash = QString(
            QCryptographicHash::hash(
                QString("cs1d_mlb_admin_salt_%1").arg(password).toUtf8(),
                QCryptographicHash::Sha256
            ).toHex()
        );

        expectedHash = "757ccc78485530665f59a01a4d4bcf2818d3ef57d68290a0d58021d3f89463ce";

        if (hash != expectedHash)
        {
            QMessageBox::warning(this,
                                 "Access Denied",
                                 "Incorrect administrator password.");
            return;
        }

        if (m_adminPage)
        {
            AdminWidget *adminWidget;

            adminWidget = qobject_cast<AdminWidget*>(m_adminPage);

            if (adminWidget)
            {
                adminWidget->refresh();
            }
        }

        setActivePage(m_adminPage, m_navAdmin);
    });

    return sidebar;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

void mainwindow::setActivePage(QWidget *page, QPushButton *activeBtn)
{
    if (!page || !m_stack) return;
    m_stack->setCurrentWidget(page);
    for (auto *btn : {m_navHome, m_navTeamInfo, m_navBrowse, m_navPlanTrip, m_navAdmin})
        styleNavBtn(btn, btn == activeBtn);
}

void mainwindow::styleNavBtn(QPushButton *btn, bool active)
{
    if (active) {
        // Active item: white text, subtle lighter bg, blue left accent strip
        btn->setStyleSheet(
            "QPushButton{"
            "  background:#162a45;"
            "  color:#ffffff;"
            "  border:none;"
            "  border-left:2px solid #4a9ade;"
            "  padding:8px 14px;"
            "  font-size:12px;"
            "  text-align:left;"
            "}"
            "QPushButton:hover{ background:#162a45; }");
    } else {
        // Inactive: muted mid-blue text, transparent, recedes into sidebar
        btn->setStyleSheet(
            "QPushButton{"
            "  background:transparent;"
            "  color:#5a80a0;"
            "  border:none;"
            "  border-left:2px solid transparent;"
            "  padding:8px 14px;"
            "  font-size:12px;"
            "  text-align:left;"
            "}"
            "QPushButton:hover{"
            "  background:#13253d;"
            "  color:#a0c4e0;"
            "}");
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
