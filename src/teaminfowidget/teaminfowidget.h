#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <vector>
#include "../database/database.h"
#include "../souvenir/souvenirmanager.h"

class TeamInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TeamInfoWidget(SouvenirManager *manager, Database *db, QWidget *parent = nullptr);

    void setTeam(const mlbInfo &team);

    // Populates the left-side team list — call after DB is loaded
    void loadTeamList(const std::vector<mlbInfo> &teams);

public slots:
    void reloadSouvenirs();

signals:
    void cartUpdated();

private:
    // Left panel
    QListWidget *m_teamList      = nullptr;
    std::vector<mlbInfo> m_teams;

    // Right panel — info cells
    QLabel *m_teamName;
    QLabel *m_stadiumName;
    QLabel *m_capacity;
    QLabel *m_location;
    QLabel *m_surface;
    QLabel *m_league;
    QLabel *m_opened;
    QLabel *m_center;
    QLabel *m_parkType;
    QLabel *m_roof;

    // Right panel — souvenirs
    QListWidget *m_souvenirList    = nullptr;
    QSpinBox    *m_quantitySpinBox = nullptr;
    QPushButton *m_buyButton       = nullptr;

    std::string m_currentTeamName;
    std::string m_currentStadiumName;
    QList<SouvenirItem> m_currentSouvenirs;

    SouvenirManager *m_souvenirManager = nullptr;
    Database        *m_db              = nullptr;

    void buildLayout();
    QWidget* makeInfoCell(const QString &label, QLabel *&valueLabel);
    void loadSouvenirs(const QList<SouvenirItem> &items);
    QList<SouvenirItem> getSouvenirs(const std::string &teamName);
    void buySelectedSouvenir();
};
