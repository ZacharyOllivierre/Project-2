#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

namespace Ui {
class homepage;
}

class homepage : public QWidget
{
    Q_OBJECT

public:
    explicit homepage(QWidget *parent = nullptr);
    ~homepage();

    void setDatabaseCounts(int teamCount, int stadiumCount, int totalCapacity = 1286462);

signals:
    void toBrowseWidget();
    void toTeamInfoWidget();
    void toTripPlannerWidget();
    void toPathViewerWidget();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::homepage *ui;
    QLabel *m_capacityLabel = nullptr;
};

#endif // HOMEPAGE_H
