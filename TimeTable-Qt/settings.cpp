#include "settings.h"
#include "ui_settings.h"
#include "TimeTableQt.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    pParent = (TimeTableQt*)parentWidget();
    ui->setupUi(this);
    FlashList();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::FlashList(int index)
{
    if (index == 0) {//Lessons
        if (!(ItemModel == nullptr)) {
            delete ItemModel;
        }
        bool ShowAll = !this->ui->checkBox->isChecked();
        ItemModel = new QStandardItemModel(this);
        QString result;
        std::vector<std::string> in;
        this->ui->listView->clearMask();
        if (ShowAll) {
            for (const auto& day : Days) {
                pParent->timetable.mGetLesson(in, day);
            }
        }
        else
        {
            pParent->timetable.mGetLesson(in, Days[this->ui->comboBox_LessonDays->currentIndex()]);
        }
        for (const auto& a : in) {
            result = QString::fromStdString(a);
            QStandardItem* item = new QStandardItem(result);
            ItemModel->appendRow(item);
        }
        this->ui->listView->setModel(ItemModel);
    }
    else if(index==1){//Infos
        if (!(ItemModel == nullptr)) {
            delete ItemModel;
        }
        ItemModel = new QStandardItemModel(this);
        QString result;
        std::vector<std::string> in;
        this->ui->listView->clearMask();
        pParent->timetable.mGetTodayMoreInfo(in, Days[this->ui->comboBox_InfoDays->currentIndex()]);
        for (const auto& a : in) {
            result = QString::fromStdString(a);
            QStandardItem* item = new QStandardItem(result);
            ItemModel->appendRow(item);
        }
        this->ui->listView->setModel(ItemModel);
    }
    else if (index == 2) {//Windows

    }
    else if(index==3){//path
    
    }
    
}

void Settings::FlashList()
{
    FlashList(ui->tabWidget->currentIndex());
}

void Settings::on_tabWidget_currentChanged(int index)
{
    FlashList(index);
}


void Settings::on_pushButton_delete_clicked()
{
    pParent->timetable.deleteLesson(this->ui->listView->currentIndex().row(), std::string(this->ui->comboBox_addLesson->currentText().toLocal8Bit()));
    FlashList();
}


void Settings::on_pushButton_order_clicked()
{
    if (this->ui->checkBox->isChecked()) {
        pParent->timetable.sortLessons(std::string(this->ui->comboBox_addLesson->currentText().toLocal8Bit()));
    }
    else
    {
        pParent->timetable.sortLessons();
    }
    FlashList();
}


void Settings::on_checkBox_stateChanged(int arg1)
{
    FlashList();
    if(this->ui->checkBox->isChecked()){
        this->ui->pushButton_delete->setEnabled(true);
        //this->ui->pushButton_order->setCheckable(false);
    }
    else {
        this->ui->pushButton_delete->setEnabled(false);        
        //this->ui->pushButton_order->setCheckable(true);
    }
}


void Settings::on_pushButton_close_clicked()
{
    close();
}

