#ifndef HUB_H
#define HUB_H

#include <QWidget>

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
    void toBrowseWidget();

private:
    Ui::Hub *ui;
};

#endif // HUB_H
