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
#include "database.h"
#include "souvenirmanager.h"

/**
 * TeamInfoWidget
 * Shows team info and lets user buy souvenirs.
 */
class TeamInfoWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Build the widget.
     */
    explicit TeamInfoWidget(SouvenirManager *manager, QWidget *parent = nullptr);

    /**
     * Update right panel for selected team.
     */
    void setTeam(const mlbInfo &team);

signals:
    /**
     * Tell main window the cart count changed.
     */
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
    QSpinBox *m_quantitySpinBox;
    QPushButton *m_buyButton;

    std::string m_currentTeamName;
    std::string m_currentStadiumName;
    QList<SouvenirItem> m_currentSouvenirs;

    SouvenirManager *m_souvenirManager = nullptr;

    void buildLayout();
    QFrame *makeInfoCell(const QString &label, QLabel *&valueLabel);
    void loadSouvenirs(const QList<SouvenirItem> &items);
    QList<SouvenirItem> getHardcodedSouvenirs(const std::string &teamName);
    void buySelectedSouvenir();
};
