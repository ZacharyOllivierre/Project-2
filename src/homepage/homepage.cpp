#include "homepage.h"
#include <QGroupBox>
#include <QTextEdit>
#include <QHBoxLayout>
#include "src/homepage/ui_homepage.h"
#include "ui_homepage.h"


homepage::homepage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::homepage)
{
    ui->setupUi(this);

    // Add total capacity box to basicInfoFrame's horizontal layout
    auto *capacityBox = new QGroupBox("Total Seating Capacity");
    auto *capLayout = new QVBoxLayout(capacityBox);
    auto *capText = new QTextEdit();
    capText->setReadOnly(true);
    capText->setPlainText("1,286,462");
    capLayout->addWidget(capText);
    // Find basicInfoFrame's layout and add the new box
    if (auto *hlay = qobject_cast<QHBoxLayout*>(ui->basicInfoFrame->layout()))
        hlay->addWidget(capacityBox);
    m_capacityLabel = nullptr;  // not used, capacity set directly above

    // Make the boxes look clickable
    ui->browseBox->setCursor(Qt::PointingHandCursor);
    ui->teamInfoBox->setCursor(Qt::PointingHandCursor);
    ui->tripPlannerBox->setCursor(Qt::PointingHandCursor);
    ui->pathViewerBox->setCursor(Qt::PointingHandCursor);

    // Install event filters on the group boxes
    ui->browseBox->installEventFilter(this);
    ui->teamInfoBox->installEventFilter(this);
    ui->tripPlannerBox->installEventFilter(this);
    ui->pathViewerBox->installEventFilter(this);

    // Also install them on the text edits because QTextEdit can steal the mouse click
    ui->browseText->installEventFilter(this);
    ui->teamInfoText->installEventFilter(this);
    ui->tripPlannerText->installEventFilter(this);
    ui->pathViewerText->installEventFilter(this);
}

homepage::~homepage()
{
    delete ui;
}

bool homepage::eventFilter(QObject *watched, QEvent *event)
{
    // Check for mouse click release
    if (event->type() == QEvent::MouseButtonRelease)
    {
        // Check if browse box or its text was clicked
        if (watched == ui->browseBox || watched == ui->browseText)
        {
            emit toBrowseWidget();
            return true;
        }

        // Check if team info box or its text was clicked
        if (watched == ui->teamInfoBox || watched == ui->teamInfoText)
        {
            emit toTeamInfoWidget();
            return true;
        }

        // Check if trip planner box or its text was clicked
        if (watched == ui->tripPlannerBox || watched == ui->tripPlannerText)
        {
            emit toTripPlannerWidget();
            return true;
        }

        // Check if path viewer box or its text was clicked
        if (watched == ui->pathViewerBox || watched == ui->pathViewerText)
        {
            emit toPathViewerWidget();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void homepage::setDatabaseCounts(int teamCount, int stadiumCount, int totalCapacity)
{
    // ui->teamCounterText->setPlainText(QString::number(teamCount));
    // ui->openStadiumText->setPlainText(QString::number(stadiumCount));
    Q_UNUSED(totalCapacity);  // capacity displayed statically as 1,286,462
}