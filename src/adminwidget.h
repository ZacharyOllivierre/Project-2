/**
 * @file adminwidget.h
 * @brief Declares the administrator page used to manage stadium data, souvenirs, prices, and database actions.
 */

#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include "database.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileDialog>
#include <QFileInfo>

struct Souvenir
{
    int     id       = -1;
    QString teamName;
    QString itemName;
    double  price    = 0.0;
    QString category;

    Souvenir() = default;
    Souvenir(int id, const QString &team, const QString &item,
             double price, const QString &cat)
        : id(id), teamName(team), itemName(item), price(price), category(cat) {}
};

class AdminWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWidget(Database *db, QWidget *parent = nullptr);
    void refresh();

signals:
    void souvenirDataChanged();

private slots:
    void onStadiumSelectionChanged();
    void onSaveStadium();
    void onDeleteStadium();
    void onClearStadiumForm();
    void onStadiumSearchChanged(const QString &text);
    void onAddNewStadium();

    void onSouvenirTeamChanged(int index);
    void onSouvenirSelectionChanged();
    void onSaveSouvenir();
    void onDeleteSouvenir();
    void onClearSouvenirForm();

    void onPricingTeamChanged(int index);
    void onSaveAllPrices();
    void onApplyPctIncrease();

private:
    void setupUi();
    QWidget* buildStadiumTab();
    QWidget* buildSouvenirTab();
    QWidget* buildPricingTab();
    QWidget* buildDatabaseTab();

    void styleTable(QTableWidget *t);
    QPushButton* makeBtn(const QString &label, const QString &color);

    void loadStadiumTable(const QString &filter = "");
    void populateStadiumForm(int row);
    void clearStadiumForm();

    void loadSouvenirTable(const QString &teamName);
    void populateSouvenirForm(int row);
    void clearSouvenirForm();

    void loadPricingTable(const QString &teamName);

    bool ensureSouvenirTable();
    QSqlDatabase souvenirDB();

    QList<Souvenir> souvenirListForTeam(const QString &teamName);
    bool insertSouvenir(const Souvenir &s);
    bool updateSouvenir(const Souvenir &s);
    bool deleteSouvenir(int id);

    bool updateStadiumInDB(const mlbInfo &info, const QString &originalTeamName);
    bool deleteStadiumFromDB(const QString &teamName);
    bool insertStadiumInDB(const mlbInfo &info);

    Database        *m_db;

    QTableWidget    *m_stadiumTable;
    QLineEdit       *m_stadiumSearch;
    QLineEdit       *m_edtTeamName;
    QLineEdit       *m_edtStadiumName;
    QLineEdit       *m_edtLocation;
    QComboBox       *m_cmbLeague;
    QComboBox       *m_cmbRoof;
    QComboBox       *m_cmbSurface;
    QComboBox       *m_cmbTypology;
    QSpinBox        *m_spnCapacity;
    QSpinBox        *m_spnYearOpened;
    QLineEdit       *m_edtCenterField;
    QPushButton     *m_btnSaveStadium;
    QPushButton     *m_btnDeleteStadium;
    QPushButton     *m_btnNewStadium;
    bool             m_stadiumIsNew    = false;
    QString          m_editingTeamName;

    QComboBox       *m_cmbSouvenirTeam;
    QTableWidget    *m_souvenirTable;
    QLineEdit       *m_edtSouvenirItem;
    QDoubleSpinBox  *m_spnSouvenirPrice;
    QComboBox       *m_cmbSouvenirCategory;
    QPushButton     *m_btnSaveSouvenir;
    QPushButton     *m_btnDeleteSouvenir;
    int              m_editingSouvenirId = -1;

    QComboBox       *m_cmbPricingTeam;
    QTableWidget    *m_pricingTable;
    QDoubleSpinBox  *m_spnPctIncrease;
    QList<Souvenir>  m_pricingCache;
};

#endif // ADMINWIDGET_H
