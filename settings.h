#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include "TimeTableQt.h"

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    void FlashList(int index);
    void FlashList();

private slots:
    void on_tabWidget_currentChanged(int index);

    void on_pushButton_delete_clicked();

    void on_pushButton_order_clicked();

private:
    Ui::Settings *ui;
    QWidget* pParent;
};

#endif // SETTINGS_H
