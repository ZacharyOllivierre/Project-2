#include "purchasewindow.h"

/**
 * Constructor
 * Build the purchase window once.
 */
PurchaseWindow::PurchaseWindow(SouvenirManager *manager, QWidget *parent)
    : QDialog(parent)
    , m_manager(manager)
{
    setWindowTitle("Purchase Screen");
    resize(700, 500);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel("Purchased Items By Stadium");
    title->setStyleSheet("font-size: 18px;"
                         "font-weight: bold;"
                         "color: white;");

    m_purchaseList = new QListWidget();

    m_grandTotalLabel = new QLabel("Grand Total: $0.00");
    m_grandTotalLabel->setStyleSheet("font-size: 14px;"
                                     "font-weight: bold;"
                                     "color: white;");

    layout->addWidget(title);
    layout->addWidget(m_purchaseList);
    layout->addWidget(m_grandTotalLabel);

    refreshScreen();
}

/**
 * Refresh all rows in the purchase screen.
 */
void PurchaseWindow::refreshScreen()
{
    m_purchaseList->clear();

    if (m_manager == nullptr) {
        m_purchaseList->addItem("No purchase manager connected.");
        return;
    }

    QStringList stadiumNames = m_manager->getAllStadiumNames();

    if (stadiumNames.isEmpty()) {
        m_purchaseList->addItem("No souvenirs purchased yet.");
        m_grandTotalLabel->setText("Grand Total: $0.00");
        return;
    }

    for (const QString &stadium : stadiumNames) {
        m_purchaseList->addItem("========================================");
        m_purchaseList->addItem("Stadium: " + stadium);

        QList<PurchasedItem> purchases = m_manager->getPurchasesForStadium(stadium);

        for (const PurchasedItem &item : purchases) {
            double lineTotal = item.price * item.quantity;

            QString row = QString("  %1  x%2  @ $%3  =  $%4")
                              .arg(item.name)
                              .arg(item.quantity)
                              .arg(item.price, 0, 'f', 2)
                              .arg(lineTotal, 0, 'f', 2);

            m_purchaseList->addItem(row);
        }

        m_purchaseList->addItem(
            QString("Stadium Total: $%1").arg(m_manager->getStadiumTotal(stadium), 0, 'f', 2));
    }

    m_grandTotalLabel->setText(
        QString("Grand Total: $%1").arg(m_manager->getGrandTotal(), 0, 'f', 2));
}
