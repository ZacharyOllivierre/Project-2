#pragma once

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QButtonGroup>
#include <QSpinBox>
#include <QScrollArea>
#include <QFrame>
#include <vector>
#include <string>

#include "../database/database.h"
#include "../souvenir/souvenirmanager.h"
#include "../stadiumgraph/stadiumgraph.h"
#include "../trip/astarrunner.h"

struct TripStop
{
    QString stadiumName;
    QString teamName;
    int     distFromPrev = 0;
};

class TripWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TripWidget(Database *db, SouvenirManager *mgr, QWidget *parent = nullptr);
    void refresh();
    void showPlanPage();   // called by mainwindow nav
    void showRoutePage();  // called by mainwindow nav

signals:
    void cartUpdated();
    void routeReady();  // emitted when Calculate Route succeeds

private slots:
    void onCalculateRoute();
    void onModeChanged(QAbstractButton *btn);
    void onBackToPlanner();
    void onBuyStop();
    void onStopSelected(int row);

private:
    QWidget*     buildPlanPage();
    QWidget*     buildRoutePage();
    QPushButton* makeBtn(const QString &label, const QString &bg);
    void         styleCombo(QComboBox *c);

    // Algorithms — A* and MST are wired; others are placeholders
    QList<TripStop> runAstar(const QString &start, const QString &end);
    QList<TripStop> runDijkstra(const QString &start, const QString &end);
    QList<TripStop> runMST(const QString &start);
    QList<TripStop> runDFS(const QString &start);
    QList<TripStop> runBFS(const QString &start);

    void loadRoute(const QList<TripStop> &stops);
    void loadSouvenirList(const QString &stadiumName);
    QList<SouvenirItem> getSouvenirs(const QString &stadiumName);

    // Data
    Database        *m_db;
    SouvenirManager *m_mgr;
    StadiumGraph     m_graph;

    QList<TripStop>  m_currentRoute;
    int              m_totalMiles = 0;
    QString          m_currentMode;

    // Plan page
    QStackedWidget  *m_outerStack    = nullptr;   // 0=plan, 1=route
    QComboBox       *m_cmbStart      = nullptr;
    QComboBox       *m_cmbEnd        = nullptr;
    QButtonGroup    *m_modeGroup     = nullptr;
    QPushButton     *m_btnCalc       = nullptr;

    // Right panel stack — swaps based on selected mode:
    //   0 = two-stop (A*, Dijkstra): start + end pickers
    //   1 = start-only (MST, Marlins): just start picker
    //   2 = fixed (DFS, BFS): no picker, fixed start shown as label
    QStackedWidget  *m_inputStack    = nullptr;
    QComboBox       *m_cmbStartSolo  = nullptr;   // used in page 1 (start-only)
    QLabel          *m_lblFixedStart = nullptr;   // used in page 2 (fixed start)

    // Route page
    QListWidget     *m_stopList      = nullptr;
    QLabel          *m_lblTotalMiles = nullptr;
    QLabel          *m_lblTotalSpent = nullptr;
    QLabel          *m_lblStopTitle  = nullptr;
    QListWidget     *m_souvenirList  = nullptr;
    QSpinBox        *m_spnQty        = nullptr;
    QPushButton     *m_btnBuy        = nullptr;
    QList<SouvenirItem> m_stopSouvenirs;
};
