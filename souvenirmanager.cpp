#include "souvenirmanager.h"

/**
 * Constructor
 * Start totals at 0.
 */
SouvenirManager::SouvenirManager()
    : m_grandTotal(0.0)
{}

/**
 * Add one purchase to a stadium.
 */
void SouvenirManager::buySouvenir(const QString &stadiumName, const SouvenirItem &item, int quantity)
{
    QList<PurchasedItem> &purchases = m_purchasesByStadium[stadiumName];

    bool found = false;

    for (PurchasedItem &purchase : purchases) {
        if (purchase.name == item.name) {
            purchase.quantity += quantity;
            found = true;
            break;
        }
    }

    if (!found) {
        PurchasedItem newPurchase;
        newPurchase.name = item.name;
        newPurchase.price = item.price;
        newPurchase.quantity = quantity;
        purchases.append(newPurchase);
    }

    double addedCost = item.price * quantity;
    m_stadiumTotals[stadiumName] += addedCost;
    m_grandTotal += addedCost;
}

/**
 * Return all purchases for one stadium.
 */
QList<PurchasedItem> SouvenirManager::getPurchasesForStadium(const QString &stadiumName) const
{
    return m_purchasesByStadium.value(stadiumName);
}

/**
 * Return subtotal for one stadium.
 */
double SouvenirManager::getStadiumTotal(const QString &stadiumName) const
{
    return m_stadiumTotals.value(stadiumName, 0.0);
}

/**
 * Return total spent across all stadiums.
 */
double SouvenirManager::getGrandTotal() const
{
    return m_grandTotal;
}

/**
 * Return every stadium that has purchases.
 */
QStringList SouvenirManager::getAllStadiumNames() const
{
    return m_purchasesByStadium.keys();
}

/**
 * Count total number of items in the cart.
 */
int SouvenirManager::getTotalItemCount() const
{
    int totalCount = 0;

    for (const QString &stadium : m_purchasesByStadium.keys()) {
        const QList<PurchasedItem> &purchases = m_purchasesByStadium[stadium];

        for (const PurchasedItem &item : purchases) {
            totalCount += item.quantity;
        }
    }

    return totalCount;
}
