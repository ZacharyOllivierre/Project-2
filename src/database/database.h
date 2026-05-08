#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>

using std::cout;
using std::endl;
using std::function;
using std::string;
using std::vector;

struct mlbInfo
{
    string teamName{};
    string stadiumName{};
    int seatingCapacity{};
    string location{};
    string playingSurface{};
    string league{};
    int dateOpened{};
    string distanceToCenterField{};
    string ballparkTypology{};
    string roofType{};

    mlbInfo()
        : teamName("")
        , stadiumName("")
        , seatingCapacity(0)
        , location("")
        , playingSurface("")
        , league("")
        , dateOpened(0)
        , distanceToCenterField("")
        , ballparkTypology("")
        , roofType("")
    {}
};

struct stadiumDistances
{
    string originatedStadium{};
    string destinationStadium{};
    int distance{};

    stadiumDistances()
        : originatedStadium("")
        , destinationStadium("")
        , distance(0)
    {}
};

class Database
{
private:
    QSqlDatabase mlb_info_db;
    QSqlDatabase stadium_distances_db;
    vector<mlbInfo> mlbInfoVector;
    vector<stadiumDistances> stadiumDistancesVector;

public:
    Database();
    void OpenDB();
    void CloseDB();
    ~Database();

    template<typename T>
    void InitVector(QSqlDatabase db,
                    const QString &table_name,
                    vector<T> &structVector,
                    function<T(QSqlQuery &query)> func)
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM " + table_name);
        query.exec();

        while (query.next()) {
            structVector.push_back(func(query));
        }
    }

    template<typename T>
    int partition(vector<T> &arr, int low, int high, function<bool(const T &, const T &)> compare)
    {
        T pivot = arr[high];
        int i = low - 1;

        for (int j = low; j <= high - 1; j++) {
            if (compare(arr[j], pivot)) {
                i++;
                std::swap(arr[i], arr[j]);
            }
        }

        std::swap(arr[i + 1], arr[high]);
        return i + 1;
    }

    template<typename T>
    void quickSort(vector<T> &arr, int low, int high, function<bool(const T &, const T &)> compare)
    {
        if (low < high) {
            int pi = partition(arr, low, high, compare);
            quickSort(arr, low, pi - 1, compare);
            quickSort(arr, pi + 1, high, compare);
        }
    }

    template<typename T>
    vector<T> SortVector(vector<T> &vec, function<bool(const T &, const T &)> compare)
    {
        if (!vec.empty()) {
            quickSort(vec, 0, static_cast<int>(vec.size()) - 1, compare);
        }
        return vec;
    }

    vector<mlbInfo> &GetMlbInfoVector();
    vector<stadiumDistances> &GetStadiumDistancesVector();
};

#endif
