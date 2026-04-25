#pragma once

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include "../database/database.h"
#include "../souvenir/souvenirmanager.h"

class TeamInfoWidget : public QWidget
{
    Q_OBJECT

public:
    // Pass db pointer so we can query souvenirs live from the DB
    explicit TeamInfoWidget(SouvenirManager *manager, Database *db, QWidget *parent = nullptr);

    // Update right panel for selected team
    void setTeam(const mlbInfo &team);

public slots:
    // Called by AdminWidget::souvenirDataChanged — reloads souvenir list for current team
    void reloadSouvenirs();

signals:
    void cartUpdated();

private:
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

    QListWidget *m_souvenirList;
    QSpinBox    *m_quantitySpinBox;
    QPushButton *m_buyButton;

    std::string m_currentTeamName;
    std::string m_currentStadiumName;
    QList<SouvenirItem> m_currentSouvenirs;

    SouvenirManager *m_souvenirManager = nullptr;
    Database        *m_db              = nullptr;

    void buildLayout();
    QFrame *makeInfoCell(const QString &label, QLabel *&valueLabel);
    void loadSouvenirs(const QList<SouvenirItem> &items);

    // Queries souvenirs table in DB; falls back to hardcoded if table is empty
    QList<SouvenirItem> getSouvenirs(const std::string &teamName);

    void buySelectedSouvenir();
};
