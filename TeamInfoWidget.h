#pragma once

#include <QWidget>
#include <QLabel>
#include <QListWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFrame>
#include "Database.h"

/**
 * Represents a single souvenir item for display.
 * Replace with DB struct when souvenir table is implemented.
 */
struct SouvenirItem {
    QString name;
    double  price;
};

/**
 * TeamInfoWidget
 * Displays team details and souvenirs for a selected MLB team.
 * Call setTeam() whenever the selected team changes.
 */
class TeamInfoWidget : public QWidget {
    Q_OBJECT

public:
    explicit TeamInfoWidget(QWidget* parent = nullptr);

    /**
     * Updates the right panel with the selected team's data.
     * @param team - mlbInfo struct from the database
     */
    void setTeam(const mlbInfo& team);

private:
    // Info grid labels
    QLabel*      m_teamName;
    QLabel*      m_stadiumName;
    QLabel*      m_capacity;
    QLabel*      m_location;
    QLabel*      m_surface;
    QLabel*      m_league;
    QLabel*      m_opened;
    QLabel*      m_center;
    QLabel*      m_parkType;
    QLabel*      m_roof;
    QListWidget* m_souvenirList;

    /**
     * Builds the full widget layout.
     * Called once in constructor.
     */
    void buildLayout();

    /**
     * Creates a styled info cell for the detail grid.
     * @param label      - field name shown in muted text above
     * @param valueLabel - pointer set to the value QLabel for later updates
     * @return QFrame* - the completed cell widget
     */
    QFrame* makeInfoCell(const QString& label, QLabel*& valueLabel);

    /**
     * Populates the souvenir list widget.
     * SWAP THIS: replace body with DB query when souvenir table exists.
     * @param items - list of SouvenirItem to display
     */
    void loadSouvenirs(const QList<SouvenirItem>& items);

    /**
     * Returns hardcoded souvenirs for a given team.
     * TEMPORARY — delete and replace with DB query when ready.
     * @param teamName - std::string team name from mlbInfo
     * @return QList<SouvenirItem>
     */
    QList<SouvenirItem> getHardcodedSouvenirs(const std::string& teamName);
};
