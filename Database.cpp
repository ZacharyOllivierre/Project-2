#include "Database.h"


Database::Database() {}

Database::~Database() {}

void Database::OpenDB()
{
    // assign db variables type & table name
    mlb_info_db          = QSqlDatabase::addDatabase("QSQLITE", "MLB Info Database");
    stadium_distances_db = QSqlDatabase::addDatabase("QSQLITE", "Stadium Distances Database");

    // set db variable to .db filepath
    mlb_info_db.setDatabaseName("databases/mlb_info.db");
    stadium_distances_db.setDatabaseName("databases/stadium_distances.db");
   
    // check if db opened
    if (!mlb_info_db.open()) { cout << "[ERROR] mlb_info.db FAILED TO OPEN\n"; }
    else { cout << "[SUCCESS] mlb_info.db OPENED SUCCESSFULLY!\n"; }

    if (!stadium_distances_db.open()) { cout << "[ERROR] stadium_distances.db FAILED TO OPEN\n"; }
    else  { cout << "[SUCCESS] stadium_distances.db OPENED SUCCESSFULLY!\n"; }

    // lambda to fill db struct
    auto fillMlbInfo = [](QSqlQuery& query)->mlbInfo
    {        
        mlbInfo mlbInfoStruct;
        mlbInfoStruct.teamName              = query.value(0).toString().toStdString();
        mlbInfoStruct.stadiumName           = query.value(1).toString().toStdString();
        mlbInfoStruct.seatingCapacity       = query.value(2).toString().remove(',').toInt();
        mlbInfoStruct.location              = query.value(3).toString().toStdString();
        mlbInfoStruct.playingSurface        = query.value(4).toString().toStdString();
        mlbInfoStruct.league                = query.value(5).toString().toStdString();
        mlbInfoStruct.dateOpened            = query.value(6).toInt();
        mlbInfoStruct.distanceToCenterField = query.value(7).toString().toStdString();
        mlbInfoStruct.ballparkTypology      = query.value(8).toString().toStdString();
        mlbInfoStruct.roofType              = query.value(9).toString().toStdString();

        return mlbInfoStruct;
    };

    InitVector(mlb_info_db, "mlb_info", mlbInfoVector, function<mlbInfo(QSqlQuery&)>(fillMlbInfo));

    // lambda to fill db struct
    auto fillStadiumDistances = [](QSqlQuery& query)->stadiumDistances
    {        
        stadiumDistances stadiumDistancesStruct;
        stadiumDistancesStruct.originatedStadium  = query.value(0).toString().toStdString();
        stadiumDistancesStruct.destinationStadium = query.value(1).toString().toStdString();
        stadiumDistancesStruct.distance           = query.value(2).toInt();

        return stadiumDistancesStruct;
    };

    InitVector(stadium_distances_db, "stadium_distances", stadiumDistancesVector, function<stadiumDistances(QSqlQuery&)>(fillStadiumDistances));
}

template<typename T>
void Database::InitVector(QSqlDatabase db, const QString& table_name, vector<T>& structVector, function<T(QSqlQuery& query)> func)
{
    QSqlQuery query(db);
    query.prepare("SELECT * FROM " + table_name);
    query.exec();

    while (query.next()) 
    {
        structVector.push_back(func(query));
    }
}

vector<mlbInfo>& Database::GetMlbInfoVector()
{ 
    return mlbInfoVector; 
}

vector<stadiumDistances>& Database::GetStadiumDistancesVector()
{
    return stadiumDistancesVector; 
}

