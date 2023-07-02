﻿#include "settings.h"
#include "ui_settings.h"
#include "TimeTableQt.h"
#include "TextFormat.h"
#include "include.h"


#undef max
#undef min

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    pParent = (TimeTableQt*)parentWidget();
    ui->setupUi(this);
    InitializeWindow();
    std::string week = pParent->timetable->mGetCurrentTime(std::string("%a"));
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
    const tm& structDate = pParent->windowsettings->mCountDownDay;
    QDate date(structDate.tm_year + 1900, structDate.tm_mon + 1, structDate.tm_mday);
    QTime time(structDate.tm_hour, structDate.tm_min, structDate.tm_sec);
    QDateTime dateTime(date,time);
    this->ui->spinBox_windowLocationX->setValue(pParent->windowsettings->miWindowX);
    this->ui->spinBox_windowLocationY->setValue(pParent->windowsettings->miWindowY);
    //this->ui->spinBox_DayCountInLine->setValue(pParent->windowsettings->miCountDownDayInLine);
    //this->ui->spinBox_lessonInLine->setValue(pParent->windowsettings->miLessonInLine);
    this->ui->spinBox_windowSizeX->setValue(pParent->windowsettings->miWindowWeight);
    this->ui->spinBox_windowSizeY->setValue(pParent->windowsettings->miWindowHeight);
    this->ui->dateTimeEdit->setDateTime(dateTime);
    this->ui->lineEdit_ConfigPath->setText(QString::fromStdString(pParent->windowsettings->msSettingPath));
    this->ui->lineEdit_LessonPath->setText(QString::fromStdString(pParent->windowsettings->msLessonInfoFile));
    this->ui->lineEdit_backGroundImg->setText(QString::fromStdString(pParent->windowsettings->msBackGroundImg));
}

void Settings::SaveSettings()
{
    pParent->windowsettings->miWindowX = this->ui->spinBox_windowLocationX->value();
    pParent->windowsettings->miWindowY = this->ui->spinBox_windowLocationY->value();
    //pParent->windowsettings->miCountDownDayInLine = this->ui->spinBox_DayCountInLine->value();
    //pParent->windowsettings->miLessonInLine = this->ui->spinBox_lessonInLine->value();
    pParent->windowsettings->miWindowWeight = this->ui->spinBox_windowSizeX->value();
    pParent->windowsettings->miWindowHeight = this->ui->spinBox_windowSizeY->value();
    QDateTime datetime = this->ui->dateTimeEdit->dateTime();
    tm& structDate = pParent->windowsettings->mCountDownDay;
    structDate.tm_year = datetime.date().year() - 1900;
    structDate.tm_mon = datetime.date().month() - 1;
    structDate.tm_mday = datetime.date().day();
    structDate.tm_hour = datetime.time().hour();
    structDate.tm_min = datetime.time().minute();
    structDate.tm_sec = datetime.time().second();
    pParent->windowsettings->msSettingPath = this->ui->lineEdit_ConfigPath->text().toStdString();
    pParent->windowsettings->msLessonInfoFile = this->ui->lineEdit_LessonPath->text().toStdString();
    pParent->windowsettings->msBackGroundImg = this->ui->lineEdit_backGroundImg->text().toStdString();

}

void Settings::FlashList(int index)
{
    bool hasAddLastItem{ false };
    auto count = this->ui->listView->count();
    int row = this->ui->listView->currentRow();
    this->ui->listView->clear();
    if (index == 0) {//Lessons
        bool ShowAll = !this->ui->checkBox->isChecked();
        QString result;
        std::vector<std::string> in;
        if (ShowAll) {
            for (const auto& day : Days) {
                pParent->timetable->mGetLesson(in, day);
            }
        }
        else
        {
            pParent->timetable->mGetLesson(in, Days[this->ui->comboBox_LessonDays->currentIndex()]);
        }
        for (const auto& a : in) {
            result = QString::fromStdString(a);
            QListWidgetItem* item = new QListWidgetItem(result);
            this->ui->listView->addItem(item);
        }
    }
    else if(index==1){//Infos
        QString result;
        std::vector<std::string> in;
        pParent->timetable->mGetTodayMoreInfo(in, Days[this->ui->comboBox_InfoDays->currentIndex()]);
        for (const auto& a : in) {
            result = QString::fromStdString(a);
            QListWidgetItem* item = new QListWidgetItem(result);
            this->ui->listView->addItem(item);
        }
    }

    else if(index==3||index==2){//textFormat
        QString result;
        auto& textFormat = pParent->windowsettings->msTextFormat;
        for (const auto& a : textFormat) {
            result = QString::fromStdString(std::to_string(a.mpTextLocation.x) + "\t" + std::to_string(a.mpTextLocation.y));
            QListWidgetItem* item = new QListWidgetItem(result);
            this->ui->listView->addItem(item);
        }
        for (auto i : pParent->windowsettings->changedItems) {
            if (i < pParent->windowsettings->msTextFormat.size()) {
                this->ui->listView->item(i)->setBackground(QBrush(QColor(255, 245, 70)));
            }
            else if(!hasAddLastItem){
                this->ui->listView->addItem("");
                this->ui->listView->item(pParent->windowsettings->msTextFormat.size())->setBackground(QBrush(QColor(255, 0, 0)));
                hasAddLastItem = true;
            }
        }
    }
    this->ui->listView->setCurrentRow(std::min(row, this->ui->listView->count() - 1));

}

void Settings::FlashList()
{
    FlashList(ui->tabWidget->currentIndex());
}

void Settings::FlashTextsList()
{
    int row = this->ui->listView->currentRow();
    if (row < 0) {
        return;
    }
    this->ui->listWidget_textItems->clear();
    for (auto& i : pParent->windowsettings->msTextFormat[row].Texts) {
        this->ui->listWidget_textItems->addItem(QString::fromStdString(i.text));
    }
    this->ui->listWidget_textItems->setCurrentRow(0);
}

void Settings::on_tabWidget_currentChanged(int index)
{
    std::string week = pParent->timetable->mGetCurrentTime(std::string("%a"));
    this->ui->comboBox_InfoDays->setCurrentText(QString::fromStdString(week));
    this->ui->comboBox_LessonDays->setCurrentText(QString::fromStdString(week));
    this->ui->checkBox->setChecked(true);
    if (index == 0) {
        this->ui->pushButton_order->setEnabled(true);
    }
    else {
        this->ui->pushButton_order->setEnabled(false);
    }
    FlashList(index);
}


void Settings::on_pushButton_delete_clicked()
{
    int row = this->ui->listView->currentRow();
    if (
        (row >= 0)
        && (row < this->ui->listView->count())
        ) 
    {
        int index = this->ui->tabWidget->currentIndex();
        switch (index)
        {
        case 0:
            pParent->timetable->deleteLesson(this->ui->listView->currentIndex().row(), std::string(this->ui->comboBox_LessonDays->currentText().toStdString()));
            break;
        case 1:
            pParent->timetable->deleteInfo(this->ui->listView->currentIndex().row(), std::string(this->ui->comboBox_InfoDays->currentText().toStdString()));
            break;
        case 3:
            pParent->windowsettings->msTextFormat.erase(pParent->windowsettings->msTextFormat.begin() + row);
            pParent->windowsettings->changedItems.push_back(row);
            break;
        default:
            break;
        }
        FlashList();
        this->ui->listView->setCurrentRow(std::min(row, this->ui->listView->count()-1));
    }
}


void Settings::on_pushButton_order_clicked()
{
    int index = this->ui->tabWidget->currentIndex();
    switch (index) 
    {
    case 0:
        if (this->ui->checkBox->isChecked()) {
            pParent->timetable->sortLessons(std::string(this->ui->comboBox_LessonDays->currentText().toLocal8Bit()));
        }
        else
        {
            pParent->timetable->sortLessons();
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
        pParent->timetable->mAddLesson(Days[this->ui->comboBox_LessonDays->currentIndex()], Lesson, tBegin.toString(QString("HHmm")).toStdString(), tEnd.toString(QString("HHmm")).toStdString());
    }
    FlashList();
    int count = this->ui->listView->count();
    this->ui->listView->setCurrentRow(count-1);
}


void Settings::on_pushButton_changeLesson_clicked()
{
    QString result{ this->ui->comboBox_addLesson->currentText() };
    if (
        (!result.isEmpty())
        && (this->ui->listView->currentRow() >= 0)
        && (this->ui->listView->currentRow() < this->ui->listView->count())
        )
    {
        std::string lesson = result.toStdString();
        QTime tBegin = this->ui->timeEdit_begin->time();
        QTime tEnd = this->ui->timeEdit_end->time();
        pParent->timetable->changeLesson(this->ui->listView->currentRow(), this->ui->comboBox_LessonDays->currentText().toStdString(), Lesson(this->ui->comboBox_LessonDays->currentText().toStdString(), this->ui->comboBox_addLesson->currentText().toStdString(), tBegin.toString(QString("HHmm")).toInt(), tEnd.toString(QString("HHmm")).toInt()));
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
    if (tab == 0&&this->ui->checkBox->isChecked()) //lessons
    {
        Lesson lesson = pParent->timetable->mGetLesson(this->ui->comboBox_LessonDays->currentText().toStdString(), currentRow);
        this->ui->comboBox_addLesson->setCurrentText(QString::fromStdString(lesson.mGetName()));
        this->ui->timeEdit_begin->setTime(QTime(Lesson::getHourFromHHmm(lesson.mGetBeginTime()), Lesson::getMinFromHHmm(lesson.mGetBeginTime())));
        this->ui->timeEdit_end->setTime(QTime(Lesson::getHourFromHHmm(lesson.mGetEndTime()), Lesson::getMinFromHHmm(lesson.mGetEndTime())));
    }
    else if (tab == 1)//info
    {
        //read info item and send to line edit
        this->ui->lineEdit_changeInfo->setText(this->ui->listView->currentItem()->text());

    }
    else if (tab == 3) //textFormat
    {
        if (currentRow >= pParent->windowsettings->msTextFormat.size()) {
            this->ui->listView->setCurrentRow(currentRow - 1);
            return;
        }
        TextFormat& textFormat = pParent->windowsettings->msTextFormat[currentRow];
        this->ui->lineEdit_color->setText(QString::fromStdString(ColorRefToHexString(textFormat.color)));
        //this->ui->lineEdit_textFormat->setText(QString::fromStdString(textFormat.));
        this->ui->fontComboBox_textFont->setCurrentText(QString::fromStdString(textFormat.msFontName));
        this->ui->spinBox_ItemLocationX->setValue((int)textFormat.mpTextLocation.x);
        this->ui->spinBox_ItemLocationY->setValue((int)textFormat.mpTextLocation.y);
        this->ui->spinBox_FontSize->setValue(textFormat.miTextSize);
        FlashTextsList();
    }
    
}


void Settings::on_pushButton_addInfo_clicked()
{
    QString result{ this->ui->lineEdit_changeInfo->text() };
    if (!result.isEmpty()) {
        int currentItem = this->ui->comboBox_InfoDays->currentIndex();
        std::string info = result.toStdString();
        pParent->timetable->mAddMoreInfo(Days[currentItem], info);
    }
    FlashList();
    int count = this->ui->listView->count();
    this->ui->listView->setCurrentRow(count - 1);
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
    pParent->windowsettings->mChangeConfigPath(this->ui->lineEdit_ConfigPath->text().toStdString());
    pParent->windowsettings->mGetWindowSettings();
    pParent->timetable->mReplacePath(pParent->windowsettings->msLessonInfoFile);
    this->ui->lineEdit_LessonPath->setText(QString::fromStdString(pParent->windowsettings->msLessonInfoFile));
    pParent->mInitializeWindow();
}


void Settings::on_pushButton_applyLessonPath_clicked()
{
    pParent->windowsettings->msLessonInfoFile = this->ui->lineEdit_LessonPath->text().toStdString();
    pParent->timetable->mReplacePath(pParent->windowsettings->msLessonInfoFile);
    pParent->timetable->mReloadLesson();
}


void Settings::on_pushButton_chooseBackGround_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString("请选择图片"), QString(".\\"), QString("图片文件(*.jpg;*.png);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    else {
        this->ui->lineEdit_backGroundImg->setText(openFileName);
    }
}


void Settings::on_pushButton_chooseColor_clicked()
{
    QColor color = QColorDialog::getColor(this->ui->lineEdit_color->text(), this);
    if (color.isValid()) {
        COLORREF colorRef{ RGB(color.red(), color.green(), color.blue()) };
        this->ui->lineEdit_color->setText(QString::fromStdString(ColorRefToHexString(colorRef)));
    }
}

TextItem Settings::ReadTextsFromUI()
{
    TextType type;
    if (this->ui->radioButton_time->isChecked()) {
        type = TextType::CurrentTime;
    }
    else if (this->ui->radioButton_lesson->isChecked()) {
        type = TextType::CurrentLesson;
    }
    else if (this->ui->radioButton_countDownDay->isChecked()) {
        type = TextType::CountDownDay;
    }
    else if (this->ui->radioButton_info->isChecked()) {
        type = TextType::Info;
    }
    TextItem format;
    format.text = this->ui->lineEdit_textFormat->text().toStdString();
    format.type = type;
    return format;
}

TextFormat Settings::ReadFormatFromUI()
{

    return TextFormat();
}

std::string Settings::ColorRefToHexString(COLORREF& color)
{
    int red = GetRValue(color);
    int blue = GetBValue(color);
    int green = GetGValue(color);
    return std::string(std::format("#{:02x}{:02x}{:02x}",red,green,blue));
}

COLORREF Settings::HexStringToColorRef(const std::string& input)
{
    int red;
    int blue;
    int green;
    int i = sscanf_s(input.c_str(), "#%02x%02x%02x", &red,&green,&blue);
    return RGB(red,green,blue);
}


void Settings::on_pushButton_addFormat_clicked()
{
    int row = this->ui->listView->currentRow();
    pParent->windowsettings->msTextFormat.push_back(TextFormat());
    pParent->windowsettings->changedItems.push_back(pParent->windowsettings->msTextFormat.size()-1);
    FlashList();
    this->ui->listView->setCurrentRow(this->ui->listView->count()-1);
}


void Settings::on_pushButton_changeFormat_clicked()
{
    TextFormat& format = pParent->windowsettings->msTextFormat[this->ui->listView->currentRow()];
    format.Texts[ui->listWidget_textItems->currentRow()] = ReadTextsFromUI();
    int row = this->ui->listView->currentRow();
    pParent->windowsettings->changedItems.push_back(row);
    FlashList();
    this->ui->listView->setCurrentRow(row);
}


void Settings::on_pushButton_applySettings_clicked()
{
    SaveSettings();
    pParent->windowsettings->save();
    pParent->mInitializeWindow();
}


void Settings::on_pushButton_cancelChange_clicked()
{
    pParent->windowsettings->mGetWindowSettings();
    pParent->windowsettings->changedItems.clear();
    FlashList();
}


void Settings::on_pushButton_saveChange_clicked()
{
    pParent->windowsettings->changedItems.clear();
    pParent->windowsettings->save();
    FlashList();
}


void Settings::on_pushButton_addTextItem_clicked()
{
    int row = this->ui->listView->currentRow();
    pParent->windowsettings->msTextFormat[row].Texts.push_back(ReadTextsFromUI());
    this->ui->listWidget_textItems->setCurrentRow(this->ui->listWidget_textItems->count() - 1);
    pParent->windowsettings->changedItems.push_back(row);
    FlashList();
    this->ui->listView->setCurrentRow(row);
}


void Settings::on_pushButton_deleteTextItem_clicked()
{

}

void Settings::on_listWidget_textItems_currentRowChanged(int currentRow)
{
    if (currentRow < 0) {
        return;
    }
    auto textItem = pParent->windowsettings->msTextFormat[this->ui->listView->currentRow()];
    ui->lineEdit_textFormat->setText(QString::fromStdString(textItem.Texts[currentRow].text));
    switch (pParent->windowsettings->msTextFormat[ui->listView->currentRow()].Texts[currentRow].type)
    {
    case TextType::CurrentTime:
        this->ui->radioButton_time->setChecked(true);
        //this->ui->radioButton_lesson->setChecked(false);
        //this->ui->radioButton_countDownDay->setChecked(false);
        //this->ui->radioButton_info->setChecked(false);
        break;
    case TextType::CurrentLesson:
        //this->ui->radioButton_time->setChecked(false);
        this->ui->radioButton_lesson->setChecked(true);
        //this->ui->radioButton_countDownDay->setChecked(false);
        //this->ui->radioButton_info->setChecked(false);
        break;
    case TextType::CountDownDay:
        //this->ui->radioButton_time->setChecked(false);
        //this->ui->radioButton_lesson->setChecked(false);
        this->ui->radioButton_countDownDay->setChecked(true);
        //this->ui->radioButton_info->setChecked(false);
        break;
    case TextType::Info:
        //this->ui->radioButton_time->setChecked(false);
        //this->ui->radioButton_lesson->setChecked(false);
        //this->ui->radioButton_countDownDay->setChecked(false);
        this->ui->radioButton_info->setChecked(true);
        break;
    default:
        break;
    }
}


void Settings::on_pushButton_changeInfo_clicked()
{

}

