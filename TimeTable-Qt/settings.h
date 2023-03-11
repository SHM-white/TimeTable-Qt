#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include "TimeTableQt.h"
#include <string>
#include <vector>

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

    void on_checkBox_stateChanged(int arg1);

    void on_pushButton_close_clicked();

private:
    Ui::Settings *ui;
    TimeTableQt* pParent;
    std::string Days[7]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    QStandardItemModel* ItemModel = nullptr;
};

#endif // SETTINGS_H
