#pragma once

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include "souvenirmanager.h"

/**
 * PurchaseWindow
 * Shows all purchased items grouped by stadium.
 */
class PurchaseWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * Build the purchase screen.
     */
    explicit PurchaseWindow(SouvenirManager *manager, QWidget *parent = nullptr);

    /**
     * Reload the screen with latest purchases.
     */
    void refreshScreen();

private:
    SouvenirManager *m_manager = nullptr;
    QListWidget *m_purchaseList = nullptr;
    QLabel *m_grandTotalLabel = nullptr;
};
