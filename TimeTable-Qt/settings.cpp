#include "settings.h"
#include "ui_settings.h"
#include "TimeTableQt.h"
#include <qmessagebox.h>
#include <qfiledialog.h>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    pParent = (TimeTableQt*)parentWidget();
    ui->setupUi(this);
    InitializeWindow();
    std::string week = pParent->timetable.mGetCurrentTime(std::string("%a"));
    this->ui->comboBox_InfoDays->setCurrentText(QString::fromStdString(week));
    this->ui->comboBox_LessonDays->setCurrentText(QString::fromStdString(week));
    FlashList();
    this->ui->listView->setCurrentRow(0);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::InitializeWindow()
{
    this->ui->spinBox_windowLocationX->setValue(pParent->windowsettings.miWindowX);
    this->ui->spinBox_windowLocationY->setValue(pParent->windowsettings.miWindowY);
    this->ui->spinBox_DayCountInLine->setValue(pParent->windowsettings.miCountDownDayInLine);
    this->ui->spinBox_lessonInLine->setValue(pParent->windowsettings.miLessonInLine);
    this->ui->spinBox_windowSizeX->setValue(pParent->windowsettings.miWindowWeight);
    this->ui->spinBox_windowSizeY->setValue(pParent->windowsettings.miWindowWeight);
    
}

void Settings::FlashList(int index)
{
    this->ui->listView->clear();
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
            QListWidgetItem* item = new QListWidgetItem(result);
            //ItemModel->appendRow(item);
            this->ui->listView->addItem(item);
        }
        //this->ui->listView->setModel(ItemModel);
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
            //QStandardItem* item = new QStandardItem(result);
            QListWidgetItem* item = new QListWidgetItem(result);
            //ItemModel->appendRow(item);
            this->ui->listView->addItem(item);
        }
        //this->ui->listView->setModel(ItemModel);
    }
    else if (index == 2) {//Windows

    }
    else if(index==3){//path
    
    }
    this->ui->listView->setCurrentRow(0);

}

void Settings::FlashList()
{
    FlashList(ui->tabWidget->currentIndex());
}

void Settings::on_tabWidget_currentChanged(int index)
{
    std::string week = pParent->timetable.mGetCurrentTime(std::string("%a"));
    this->ui->comboBox_InfoDays->setCurrentText(QString::fromStdString(week));
    this->ui->comboBox_LessonDays->setCurrentText(QString::fromStdString(week));

    FlashList(index);
}


void Settings::on_pushButton_delete_clicked()
{
    int index = this->ui->tabWidget->currentIndex();
    switch (index)
    {
    case 0:
        pParent->timetable.deleteLesson(this->ui->listView->currentIndex().row(), std::string(this->ui->comboBox_LessonDays->currentText().toLocal8Bit()));
        break;
    case 1:
        break;
    default:
        break;
    }
    FlashList();
}


void Settings::on_pushButton_order_clicked()
{
    int index = this->ui->tabWidget->currentIndex();
    switch (index) 
    {
    case 0:
        if (this->ui->checkBox->isChecked()) {
            pParent->timetable.sortLessons(std::string(this->ui->comboBox_LessonDays->currentText().toLocal8Bit()));
        }
        else
        {
            pParent->timetable.sortLessons();
        }
        break;
    default:
        break;
    }
    
    FlashList();
}


void Settings::on_checkBox_stateChanged(int arg1)
{
    FlashList();
    if(this->ui->checkBox->isChecked()){
        this->ui->pushButton_delete->setEnabled(true);
        this->ui->pushButton_changeLesson->setEnabled(true);
    }
    else {
        this->ui->pushButton_delete->setEnabled(false);        
        this->ui->pushButton_changeLesson->setEnabled(false);
    }
}


void Settings::on_pushButton_close_clicked()
{
    close();
}


void Settings::on_comboBox_LessonDays_currentIndexChanged(int index)
{
    FlashList();
}


void Settings::on_comboBox_InfoDays_currentTextChanged(const QString &arg1)
{
    FlashList();
}


void Settings::on_pushButton_addLesson_clicked()
{
    QString result{ this->ui->comboBox_addLesson->currentText()};
    if (!result.isEmpty()) {
        std::string Lesson = result.toStdString();
        QTime tBegin = this->ui->timeEdit_begin->time();
        QTime tEnd = this->ui->timeEdit_end->time();
        pParent->timetable.mAddLesson(Days[this->ui->comboBox_LessonDays->currentIndex()], Lesson, tBegin.toString(QString("HHmm")).toStdString(), tEnd.toString(QString("HHmm")).toStdString());
    }
    FlashList();
    int count = this->ui->listView->count();
    this->ui->listView->setCurrentRow(count-1);
}


void Settings::on_pushButton_changeLesson_clicked()
{
    QString result{ this->ui->comboBox_addLesson->currentText() };
    if (!result.isEmpty()) {
        std::string lesson = result.toStdString();
        QTime tBegin = this->ui->timeEdit_begin->time();
        QTime tEnd = this->ui->timeEdit_end->time();
        pParent->timetable.changeLesson(this->ui->listView->currentRow(), this->ui->comboBox_LessonDays->currentText().toStdString(), Lesson(this->ui->comboBox_LessonDays->currentText().toStdString(), this->ui->comboBox_addLesson->currentText().toStdString(), tBegin.toString(QString("HHmm")).toInt(), tEnd.toString(QString("HHmm")).toInt()));
    }
    int row = this->ui->listView->currentRow();
    FlashList();
    this->ui->listView->setCurrentRow(row);

}


void Settings::on_listView_currentRowChanged(int currentRow)
{
    if (currentRow < 0) {
       return;
    }
    int tab{ ui->tabWidget->currentIndex() };
    if (tab == 0&&this->ui->checkBox->isChecked()) {
        Lesson lesson = pParent->timetable.mGetLesson(this->ui->comboBox_LessonDays->currentText().toStdString(), currentRow);
        this->ui->comboBox_addLesson->setCurrentText(QString::fromStdString(lesson.mGetName()));
        this->ui->timeEdit_begin->setTime(QTime(Lesson::getHourFromHHmm(lesson.mGetBeginTime()), Lesson::getMinFromHHmm(lesson.mGetBeginTime())));
        this->ui->timeEdit_end->setTime(QTime(Lesson::getHourFromHHmm(lesson.mGetEndTime()), Lesson::getMinFromHHmm(lesson.mGetEndTime())));
    }
    
}


void Settings::on_pushButton_addInfo_clicked()
{
    QString result{ this->ui->lineEdit_changeInfo->text() };
    if (!result.isEmpty()) {
        int currentItem = this->ui->comboBox_InfoDays->currentIndex();
        std::string info = result.toStdString();
        pParent->timetable.mAddMoreInfo(Days[currentItem], info);
    }
    FlashList();
}


void Settings::on_pushButton_chooseConfig_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString("请选择需要打开json"), QString(".\\"), QString("JSON文件(*.json);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    else {
        this->ui->lineEdit_ConfigPath->setText(openFileName);
    }
}


void Settings::on_pushButton_chooseLessons_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString("请选择需要打开json"), QString(".\\"), QString("JSON文件(*.json);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    else {
        this->ui->lineEdit_LessonPath->setText(openFileName);
    }
}


void Settings::on_pushButton_applyConfigPath_clicked()
{
    pParent->windowsettings.mChangeConfigPath(this->ui->lineEdit_ConfigPath->text().toStdString());
    pParent->windowsettings.mGetWindowSettings();
    this->ui->lineEdit_LessonPath->setText(QString::fromStdString(pParent->windowsettings.msLessonInfoFile));
    pParent->mInitializeWindow();
}


void Settings::on_pushButton_applyLessonPath_clicked()
{
    pParent->windowsettings.msLessonInfoFile = this->ui->lineEdit_LessonPath->text().toStdString();
    pParent->timetable.mReplacePath(pParent->windowsettings.msLessonInfoFile);
}

