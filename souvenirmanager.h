#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>

/**
 * One souvenir item that can be sold.
 */
struct SouvenirItem
{
    QString name;
    double  price;
};

/**
 * One purchased souvenir entry.
 */
struct PurchasedItem
{
    QString name;
    double  price;
    int     quantity;
};

/**
 * SouvenirManager
 * Handles all souvenir purchase tracking.
 */
class SouvenirManager
{
public:
    SouvenirManager();

    /**
     * Add a purchase to one stadium.
     * If item already exists, add quantity.
     */
    void buySouvenir(const QString& stadiumName,
                     const SouvenirItem& item,
                     int quantity);

    /**
     * Return all purchases for one stadium.
     */
    QList<PurchasedItem> getPurchasesForStadium(const QString& stadiumName) const;

    /**
     * Return subtotal for one stadium.
     */
    double getStadiumTotal(const QString& stadiumName) const;

    /**
     * Return total across all stadiums.
     */
    double getGrandTotal() const;

    /**
     * Return all stadium names that have purchases.
     */
    QStringList getAllStadiumNames() const;

    /**
     * Count every item in the cart.
     * Example: 2 hats + 3 bats = 5 items.
     */
    int getTotalItemCount() const;

private:
    // stadium -> list of purchased items
    QMap<QString, QList<PurchasedItem>> m_purchasesByStadium;

    // stadium -> subtotal
    QMap<QString, double> m_stadiumTotals;

    // total across all stadiums
    double m_grandTotal;
};
