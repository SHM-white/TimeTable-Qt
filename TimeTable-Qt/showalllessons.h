#ifndef SHOWALLLESSONS_H
#define SHOWALLLESSONS_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include "TimeTableQt.h"
#include <string>
#include <vector>

namespace Ui {
class ShowAllLessons;
}

class ShowAllLessons : public QDialog
{
    Q_OBJECT

public:
    void mFlashList();
    explicit ShowAllLessons(QWidget *parent = nullptr);
    ~ShowAllLessons();

private slots:
    void on_checkBox_stateChanged(int arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    std::wstring Days[7]{ L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
    Ui::ShowAllLessons *ui;
    QStandardItemModel* ItemModel = nullptr;
    TimeTableQt* pParent = (TimeTableQt*)parentWidget();
};

#endif // SHOWALLLESSONS_H
