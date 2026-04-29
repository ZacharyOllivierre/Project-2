#include "database.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>

Database::Database() {}

Database::~Database()
{
    CloseDB();
}

void Database::OpenDB()
{
    CloseDB();
    
    mlbInfoVector.clear();
    stadiumDistancesVector.clear();

    mlb_info_db = QSqlDatabase::addDatabase("QSQLITE", "MLB Info Database");
    stadium_distances_db = QSqlDatabase::addDatabase("QSQLITE", "Stadium Distances Database");

    QStringList mlbPaths;
    QStringList distPaths;

    mlbPaths << "databases/mlb_info.db"
             << "../databases/mlb_info.db"
             << "../../databases/mlb_info.db"
             << "../../../databases/mlb_info.db";

    distPaths << "databases/stadium_distances.db"
              << "../databases/stadium_distances.db"
              << "../../databases/stadium_distances.db"
              << "../../../databases/stadium_distances.db";

    QString mlbPath;
    QString distPath;

    for (const QString &path : mlbPaths) {
        if (QFileInfo::exists(path)) {
            mlbPath = QFileInfo(path).absoluteFilePath();
            break;
        }
    }

    for (const QString &path : distPaths) {
        if (QFileInfo::exists(path)) {
            distPath = QFileInfo(path).absoluteFilePath();
            break;
        }
    }

    if (mlbPath.isEmpty()) {
        cout << "[ERROR] Could not find mlb_info.db\n";
        return;
    }

    if (distPath.isEmpty()) {
        cout << "[ERROR] Could not find stadium_distances.db\n";
        return;
    }

    // The original MLB database should never be modified directly.
    // All admin changes are made to mlb_info_active.db.
    //
    // If mlb_info_active.db does not exist yet, create it as a copy
    // of the original mlb_info.db.
    QFileInfo originalMlbInfo(mlbPath);
    QDir databaseDir(originalMlbInfo.absolutePath());

    QString activeMlbPath;

    activeMlbPath = databaseDir.filePath("mlb_info_active.db");

    if (!QFileInfo::exists(activeMlbPath))
    {
        if (!QFile::copy(originalMlbInfo.absoluteFilePath(), activeMlbPath))
        {
            cout << "[ERROR] Could not create mlb_info_active.db\n";
            return;
        }
    }

    mlb_info_db.setDatabaseName(activeMlbPath);
    stadium_distances_db.setDatabaseName(distPath);

    if (!mlb_info_db.open()) {
        cout << "[ERROR] mlb_info.db FAILED TO OPEN\n";
        return;
    }

    if (!stadium_distances_db.open()) {
        cout << "[ERROR] stadium_distances.db FAILED TO OPEN\n";
        return;
    }

    cout << "[SUCCESS] Databases opened successfully\n";

    auto fillMlbInfo = [](QSqlQuery &query) -> mlbInfo {
        mlbInfo info;
        info.teamName = query.value(0).toString().toStdString();
        info.stadiumName = query.value(1).toString().toStdString();
        info.seatingCapacity = query.value(2).toString().remove(',').toInt();
        info.location = query.value(3).toString().toStdString();
        info.playingSurface = query.value(4).toString().toStdString();
        info.league = query.value(5).toString().toStdString();
        info.dateOpened = query.value(6).toInt();
        info.distanceToCenterField = query.value(7).toString().toStdString();
        info.ballparkTypology = query.value(8).toString().toStdString();
        info.roofType = query.value(9).toString().toStdString();
        return info;
    };

    auto fillDistances = [](QSqlQuery &query) -> stadiumDistances {
        stadiumDistances info;
        info.originatedStadium = query.value(0).toString().toStdString();
        info.destinationStadium = query.value(1).toString().toStdString();
        info.distance = query.value(2).toInt();
        return info;
    };

    InitVector(mlb_info_db, "mlb_info", mlbInfoVector, function<mlbInfo(QSqlQuery &)>(fillMlbInfo));

    InitVector(stadium_distances_db,
               "stadium_distances",
               stadiumDistancesVector,
               function<stadiumDistances(QSqlQuery &)>(fillDistances));

    cout << "[INFO] Loaded teams: " << mlbInfoVector.size() << endl;
}

void Database::CloseDB()
{
    if (mlb_info_db.isOpen())
    {
        mlb_info_db.close();
    }

    if (stadium_distances_db.isOpen())
    {
        stadium_distances_db.close();
    }

    mlb_info_db = QSqlDatabase();
    stadium_distances_db = QSqlDatabase();

    if (QSqlDatabase::contains("MLB Info Database"))
    {
        QSqlDatabase::removeDatabase("MLB Info Database");
    }

    if (QSqlDatabase::contains("Stadium Distances Database"))
    {
        QSqlDatabase::removeDatabase("Stadium Distances Database");
    }
}

vector<mlbInfo> &Database::GetMlbInfoVector()
{
    return mlbInfoVector;
}

vector<stadiumDistances> &Database::GetStadiumDistancesVector()
{
    return stadiumDistancesVector;
}
