#include "showalllessons.h"
#include "ui_showalllessons.h"

void ShowAllLessons::mFlashList()
{
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
        pParent->timetable.mGetLesson(in, Days[this->ui->comboBox->currentIndex()]);
    }
    for (const auto& a : in) {
        result = QString::fromStdString(a);
        QStandardItem* item = new QStandardItem(result);
        ItemModel->appendRow(item);
    }
    this->ui->listView->setModel(ItemModel);
}

ShowAllLessons::ShowAllLessons(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowAllLessons)
{
    ui->setupUi(this);
    std::string week = pParent->timetable.mGetCurrentTime(std::string("%a"));
    this->ui->comboBox->setCurrentText(QString::fromStdString(week));
    mFlashList();
}

ShowAllLessons::~ShowAllLessons()
{
    delete ui;
}

void ShowAllLessons::on_checkBox_stateChanged(int arg1)
{
    mFlashList();
}


void ShowAllLessons::on_comboBox_currentIndexChanged(int index)
{
    mFlashList();
}


void ShowAllLessons::on_pushButton_clicked()
{
    QString result{ this->ui->lineEdit->text() };
    if (!result.isEmpty()) {
        //std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
        std::string Lesson = result.toStdString();
        QTime tBegin = this->ui->timeEdit->time();
        QTime tEnd = this->ui->timeEdit_2->time();
        pParent->timetable.mAddLesson(Days[this->ui->comboBox->currentIndex()], Lesson, tBegin.toString(QString("HHmm")).toStdString(), tEnd.toString(QString("HHmm")).toStdString());
    }
    mFlashList();
}


void ShowAllLessons::on_pushButton_2_clicked()
{
    close();
}

