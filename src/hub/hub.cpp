#include "hub.h"
#include "ui_hub.h"

#include "../browse/browsewidget.h"
#include "../database/database.h"

Hub::Hub(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Hub)
{
    ui->setupUi(this);
}

Hub::~Hub()
{
    delete ui;
}

void Hub::toBrowseWidget()
{
    Database db;
    db.OpenDB();

    BrowseWidget* browse = new BrowseWidget(db.GetMlbInfoVector());
    browse->setAttribute(Qt::WA_DeleteOnClose);
    browse->show();

    this->close();
}