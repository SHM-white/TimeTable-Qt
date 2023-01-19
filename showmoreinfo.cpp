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
    std::vector<std::string> in;
    this->ui->listView->clearMask();
    //std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    pParent->timetable.mGetTodayMoreInfo(in, Days[this->ui->comboBox->currentIndex()]);
    for (auto a : in) {
        result = QString::fromLocal8Bit(a.c_str());
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
        //std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
        std::string info = result.toStdString();
        pParent->timetable.mAddMoreInfo(Days[currentItem], info);
    }
    mFlashList();
}

