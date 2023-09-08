#include "showmoreinfo.h"
#include "ui_showmoreinfo.h"
#include "TimeTableQt.h"
#include <string>
#include <vector>

void ShowMoreInfo::mFlashList()
{
    if (!(ItemModel == nullptr)) {
        delete ItemModel;
    }
    ItemModel = new QStandardItemModel(this);
    QString result;
    std::vector<std::wstring> in;
    this->ui->listView->clearMask();
    //std::wstring Days[]{ L"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    pParent->timetable->mGetTodayMoreInfo(in, Days[this->ui->comboBox->currentIndex()]);
    for (auto a : in) {
        result = QString::fromStdWString(a);
        QStandardItem* item = new QStandardItem(result);
        ItemModel->appendRow(item);
    }
    this->ui->listView->setModel(ItemModel);
}

ShowMoreInfo::ShowMoreInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowMoreInfo)
{
    ui->setupUi(this);
    std::wstring week = pParent->timetable->mGetCurrentTime(std::wstring(L"%a"));
    this->ui->comboBox->setCurrentText(QString::fromStdWString(week));
    mFlashList();
}

ShowMoreInfo::~ShowMoreInfo()
{
    delete ui;
}

void ShowMoreInfo::on_comboBox_currentTextChanged(const QString &arg1)
{
    mFlashList();
}


void ShowMoreInfo::on_pushButton_2_clicked()
{
    close();
}


void ShowMoreInfo::on_pushButton_clicked()
{
    QString result{ this->ui->lineEdit->text() };
    if (!result.isEmpty()) {
        int currentItem = this->ui->comboBox->currentIndex();
        std::wstring info = result.toStdWString();
        pParent->timetable->mAddMoreInfo(Days[currentItem], info);
    }
    mFlashList();
}

