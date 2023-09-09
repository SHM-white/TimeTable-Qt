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
    std::vector<std::wstring> in;
    this->ui->listView->clearMask();
    if (ShowAll) {
        for (const auto& day : Days) {
            pParent->timetable->mGetLesson(in, day);
        }
    }
    else
    {
        pParent->timetable->mGetLesson(in, Days[this->ui->comboBox->currentIndex()]);
    }
    for (const auto& a : in) {
        result = QString::fromStdWString(a);
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
    std::wstring week = pParent->timetable->mGetCurrentTime(std::wstring(L"%a"));
    this->ui->comboBox->setCurrentText(QString::fromStdWString(week));
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
        //std::wstring Days[]{ L"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
        std::wstring Lesson = result.toStdWString();
        QTime tBegin = this->ui->timeEdit->time();
        QTime tEnd = this->ui->timeEdit_2->time();
        pParent->timetable->mAddLesson(Days[this->ui->comboBox->currentIndex()], Lesson, tBegin.toString(QString::fromStdWString(L"HHmm")).toStdWString(), tEnd.toString(QString::fromStdWString(L"HHmm")).toStdWString());
    }
    mFlashList();
}


void ShowAllLessons::on_pushButton_2_clicked()
{
    close();
}

