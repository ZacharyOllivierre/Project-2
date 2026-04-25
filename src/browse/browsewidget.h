#ifndef BROWSEWIDGET_H
#define BROWSEWIDGET_H

#include <QWidget>
#include <set>
#include <string>
#include <vector>

#include "../database/database.h"

using std::set;
using std::string;
using std::vector;

enum class SortType
{
    TeamName,
    StadiumName,
    SeatingCapacity,
    DateOpened,
    DistanceToCenter
};

QT_BEGIN_NAMESPACE
namespace Ui {
class BrowseWidget;
}
QT_END_NAMESPACE

class BrowseWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BrowseWidget(const vector<mlbInfo>& teams, QWidget *parent = nullptr);
    ~BrowseWidget() override;

    inline void updateTeams(const vector<mlbInfo>& teams) {mInfo = teams;}


private slots:
    void on_leagueTypeCombo_currentIndexChanged(int index);

    void on_roofTypeCombo_currentIndexChanged(int index);

    void on_sortTypeCombo_currentIndexChanged(int index);

    void toHub();

private:
    Ui::BrowseWidget *ui;

    vector<mlbInfo> mInfo;
    QStringList headers;

    SortType sortType;

    void initLeagueTypeCombo();
    void initRoofTypeCombo();
    void initSortTypeCombo();

    void initTable();

    void populateTable();
    void addRow(int row, mlbInfo& t);

    void sortData(vector<mlbInfo>& data);
};

#endif // BROWSEWIDGET_H
