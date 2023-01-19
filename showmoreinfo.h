#ifndef SHOWMOREINFO_H
#define SHOWMOREINFO_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include <string>
#include <vector>
#include "TimeTableQt.h"

namespace Ui {
class ShowMoreInfo;
}

class ShowMoreInfo : public QDialog
{
    Q_OBJECT

public:
    void mFlashList();
    explicit ShowMoreInfo(QWidget *parent = nullptr);
    ~ShowMoreInfo();

private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    std::string Days[7]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    Ui::ShowMoreInfo* ui;
    QStandardItemModel* ItemModel = nullptr;
    TimeTableQt* pParent = (TimeTableQt*)parentWidget();
};

#endif // SHOWMOREINFO_H
