#ifndef HUB_H
#define HUB_H

#include <QWidget>

class BrowseWidget;

namespace Ui {
class Hub;
}

class Hub : public QWidget
{
    Q_OBJECT

public:
    explicit Hub(QWidget *parent = nullptr);
    ~Hub();

private slots:
    void on_buttonTeamBrowse_clicked();

private:
    Ui::Hub *ui;
    BrowseWidget *browseWidget;
};

#endif // HUB_H