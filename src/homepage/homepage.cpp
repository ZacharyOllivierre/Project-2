#include "homepage.h"
#include "src/homepage/ui_homepage.h"
#include "ui_homepage.h"


homepage::homepage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::homepage)
{
    ui->setupUi(this);

    // Make the boxes look clickable
    ui->browseBox->setCursor(Qt::PointingHandCursor);
    ui->teamInfoBox->setCursor(Qt::PointingHandCursor);
    ui->tripPlannerBox->setCursor(Qt::PointingHandCursor);

    // Install event filters on the group boxes
    ui->browseBox->installEventFilter(this);
    ui->teamInfoBox->installEventFilter(this);
    ui->tripPlannerBox->installEventFilter(this);

    // Also install them on the text edits because QTextEdit can steal the mouse click
    ui->browseText->installEventFilter(this);
    ui->teamInfoText->installEventFilter(this);
    ui->tripPlannerText->installEventFilter(this);

    // Optional: make text boxes not editable
    ui->browseText->setReadOnly(true);
    ui->teamInfoText->setReadOnly(true);
    ui->tripPlannerText->setReadOnly(true);
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
    }

    return QWidget::eventFilter(watched, event);
}