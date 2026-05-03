#include "hub.h"
#include "ui_hub.h"

#include "../browse/browsewidget.h"
#include "../database/database.h"

#include <QVBoxLayout>

Hub::Hub(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Hub)
    , browseWidget(nullptr)
{
    ui->setupUi(this);

    Database db;
    db.OpenDB();

    browseWidget = new BrowseWidget(db.GetMlbInfoVector(), ui->pageBrowse);

    QVBoxLayout *browseLayout = new QVBoxLayout(ui->pageBrowse);
    browseLayout->setContentsMargins(0, 0, 0, 0);
    browseLayout->addWidget(browseWidget);

    ui->stackedWidget->setCurrentWidget(ui->pageHome);
}

Hub::~Hub()
{
    delete ui;
}

void Hub::on_buttonTeamBrowse_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageBrowse);
}