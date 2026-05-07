/**
 * @file homepage.h
 * @brief Declares the home page widget and its navigation signals for the main project screens.
 */

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

namespace Ui {
class homepage;
}

class homepage : public QWidget
{
    Q_OBJECT

public:
    explicit homepage(QWidget *parent = nullptr);
    ~homepage();

signals:
    void toBrowseWidget();
    void toTeamInfoWidget();
    void toTripPlannerWidget();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::homepage *ui;
};

#endif // HOMEPAGE_H
