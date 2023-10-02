#include "settings.h"
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
    std::wstring week = pParent->timetable->mGetCurrentTime(std::wstring(L"%a"));
    this->ui->comboBox_InfoDays->setCurrentText(QString::fromStdWString(week));
    this->ui->comboBox_LessonDays->setCurrentText(QString::fromStdWString(week));
    FlashList();
    this->ui->listView->setCurrentRow(0);
}

Settings::~Settings()
{
    delete ui;
}

/**
 * @brief Initializes the window settings.
 */
void Settings::InitializeWindow()
{
    // Get the date from the parent window settings
    const tm& structDate = pParent->windowsettings->mCountDownDay;
    QDate date(structDate.tm_year + 1900, structDate.tm_mon + 1, structDate.tm_mday);
    QTime time(structDate.tm_hour, structDate.tm_min, structDate.tm_sec);
    QDateTime dateTime(date, time);

    // Set the values of the spin boxes
    this->ui->spinBox_windowLocationX->setValue(pParent->windowsettings->miWindowX);
    this->ui->spinBox_windowLocationY->setValue(pParent->windowsettings->miWindowY);
    this->ui->spinBox_windowSizeX->setValue(pParent->windowsettings->miWindowWeight);
    this->ui->spinBox_windowSizeY->setValue(pParent->windowsettings->miWindowHeight);

    // Set the value of the date time edit widget
    this->ui->dateTimeEdit->setDateTime(dateTime);

    this->ui->checkBox_useColor->setChecked(true);
    this->ui->checkBox_useColor->setChecked(!pParent->windowsettings->mUseImgAsBackGround);

    // Set the text of the line edit widgets
    this->ui->lineEdit_ConfigPath->setText(QString::fromStdWString(pParent->windowsettings->msSettingPath));
    this->ui->lineEdit_LessonPath->setText(QString::fromStdWString(pParent->windowsettings->msLessonInfoFile));
    this->ui->lineEdit_backGroundImg->setText(QString::fromStdWString(pParent->windowsettings->msBackGroundImg));
    COLORREF backgroundColor = RGB(pParent->windowsettings->miBackGroundColor[0], pParent->windowsettings->miBackGroundColor[1], pParent->windowsettings->miBackGroundColor[2]);
    this->ui->lineEdit_backGroundColor->setText(QString::fromStdWString(ColorRefToHexString(backgroundColor)));
    this->ui->horizontalSlider_alpha->setValue(pParent->windowsettings->miBackGroundColor[3]);
}

/**
 * @brief Saves the settings of the application.
 */
void Settings::SaveSettings()
{
    // Save window location
    pParent->windowsettings->miWindowX = this->ui->spinBox_windowLocationX->value();
    pParent->windowsettings->miWindowY = this->ui->spinBox_windowLocationY->value();

    // Save window size
    pParent->windowsettings->miWindowWeight = this->ui->spinBox_windowSizeX->value();
    pParent->windowsettings->miWindowHeight = this->ui->spinBox_windowSizeY->value();

    // Save countdown day
    QDateTime datetime = this->ui->dateTimeEdit->dateTime();
    tm& structDate = pParent->windowsettings->mCountDownDay;
    structDate.tm_year = datetime.date().year() - 1900;
    structDate.tm_mon = datetime.date().month() - 1;
    structDate.tm_mday = datetime.date().day();
    structDate.tm_hour = datetime.time().hour();
    structDate.tm_min = datetime.time().minute();
    structDate.tm_sec = datetime.time().second();

    // Save paths
    pParent->windowsettings->msSettingPath = this->ui->lineEdit_ConfigPath->text().toStdWString();
    pParent->windowsettings->msLessonInfoFile = this->ui->lineEdit_LessonPath->text().toStdWString();
    pParent->windowsettings->msBackGroundImg = this->ui->lineEdit_backGroundImg->text().toStdWString();

    // Save background color
    COLORREF backgroundColor = HexStringToColorRef(this->ui->lineEdit_backGroundColor->text().toStdWString());
    pParent->windowsettings->miBackGroundColor[0] = GetRValue(backgroundColor);
    pParent->windowsettings->miBackGroundColor[1] = GetGValue(backgroundColor);
    pParent->windowsettings->miBackGroundColor[2] = GetBValue(backgroundColor);
    pParent->windowsettings->miBackGroundColor[3] = this->ui->horizontalSlider_alpha->value();

    pParent->windowsettings->mUseImgAsBackGround = !this->ui->checkBox_useColor->isChecked();
}

/**
 * @brief Refreshes the list view based on the provided index.
 * 
 * @param index The index to determine the type of data to display in the list view.
 */
void Settings::FlashList(int index)
{
    bool hasAddLastItem{ false }; // Flag to track if the last item has been added

    auto count = this->ui->listView->count(); // Get the count of items in the list view
    int row = this->ui->listView->currentRow(); // Get the current selected row in the list view
    this->ui->listView->clear(); // Clear the list view

    if (index == 0) { // Lessons
        bool ShowAll = !this->ui->checkBox->isChecked(); // Check if all lessons should be shown
        QString result;
        std::vector<std::wstring> in;

        if (ShowAll) {
            for (const auto& day : Days) {
                pParent->timetable->mGetLesson(in, day); // Get all lessons for each day
            }
        }
        else {
            pParent->timetable->mGetLesson(in, Days[this->ui->comboBox_LessonDays->currentIndex()]); // Get lessons for the selected day
        }

        for (const auto& a : in) {
            result = QString::fromStdWString(a);
            QListWidgetItem* item = new QListWidgetItem(result); // Create a new list widget item
            this->ui->listView->addItem(item); // Add the item to the list view
        }
    }
    else if (index == 1) { // Infos
        QString result;
        std::vector<std::wstring> in;
        pParent->timetable->mGetTodayMoreInfo(in, Days[this->ui->comboBox_InfoDays->currentIndex()]); // Get additional info for today
        for (const auto& a : in) {
            result = QString::fromStdWString(a);
            QListWidgetItem* item = new QListWidgetItem(result); // Create a new list widget item
            this->ui->listView->addItem(item); // Add the item to the list view
        }
    }
    else if (index == 3 || index == 2) { // textFormat
        QString result;
        auto& textFormat = pParent->windowsettings->msTextFormat;

        for (const auto& a : textFormat) {
            result = QString::fromStdWString(L"X:" + std::to_wstring(a.mpTextLocation.x) + L"\tY:" + std::to_wstring(a.mpTextLocation.y));
            QListWidgetItem* item = new QListWidgetItem(result); // Create a new list widget item
            this->ui->listView->addItem(item); // Add the item to the list view
        }

        for (auto i : pParent->windowsettings->changedItems) {
            if (i < pParent->windowsettings->msTextFormat.size()) {
                this->ui->listView->item(i)->setBackground(QBrush(QColor(255, 245, 70))); // Set background color for changed items
            }
            else if (!hasAddLastItem) {
                this->ui->listView->addItem(""); // Add an empty item
                this->ui->listView->item(pParent->windowsettings->msTextFormat.size())->setBackground(QBrush(QColor(255, 0, 0))); // Set background color for the last item
                hasAddLastItem = true;
            }
        }

        FlashTextsList(); // Flash the texts list
    }

    this->ui->listView->setCurrentRow(std::min(row, this->ui->listView->count() - 1)); // Set the current selected row in the list view
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
        this->ui->listWidget_textItems->addItem(QString::fromStdWString(i.text));
    }
    this->ui->listWidget_textItems->setCurrentRow(0);
}

void Settings::on_tabWidget_currentChanged(int index)
{
    std::wstring week = pParent->timetable->mGetCurrentTime(std::wstring(L"%a"));
    this->ui->comboBox_InfoDays->setCurrentText(QString::fromStdWString(week));
    this->ui->comboBox_LessonDays->setCurrentText(QString::fromStdWString(week));
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
            pParent->timetable->deleteLesson(this->ui->listView->currentIndex().row(), std::wstring(this->ui->comboBox_LessonDays->currentText().toStdWString()));
            break;
        case 1:
            pParent->timetable->deleteInfo(this->ui->listView->currentIndex().row(), std::wstring(this->ui->comboBox_InfoDays->currentText().toStdWString()));
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
            pParent->timetable->sortLessons(std::wstring(this->ui->comboBox_LessonDays->currentText().toStdWString()));
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
        std::wstring Lesson = result.toStdWString();
        QTime tBegin = this->ui->timeEdit_begin->time();
        QTime tEnd = this->ui->timeEdit_end->time();
        pParent->timetable->mAddLesson(Days[this->ui->comboBox_LessonDays->currentIndex()], Lesson, tBegin.toString(QString::fromStdWString(L"HHmm")).toStdWString(), tEnd.toString(QString::fromStdWString(L"HHmm")).toStdWString());
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
        std::wstring lesson = result.toStdWString();
        QTime tBegin = this->ui->timeEdit_begin->time();
        QTime tEnd = this->ui->timeEdit_end->time();
        pParent->timetable->changeLesson(this->ui->listView->currentRow(), this->ui->comboBox_LessonDays->currentText().toStdWString(), Lesson(this->ui->comboBox_LessonDays->currentText().toStdWString(), this->ui->comboBox_addLesson->currentText().toStdWString(), tBegin.toString(QString::fromStdWString(L"HHmm")).toInt(), tEnd.toString(QString::fromStdWString(L"HHmm")).toInt()));
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
        Lesson lesson = pParent->timetable->mGetLesson(this->ui->comboBox_LessonDays->currentText().toStdWString(), currentRow);
        this->ui->comboBox_addLesson->setCurrentText(QString::fromStdWString(lesson.mGetName()));
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
        this->ui->lineEdit_color->setText(QString::fromStdWString(ColorRefToHexString(textFormat.color)));
        //this->ui->lineEdit_textFormat->setText(QString::fromStdWString(textFormat.));
        this->ui->fontComboBox_textFont->setCurrentText(QString::fromStdWString(textFormat.msFontName));
        this->ui->spinBox_ItemLocationX->setValue((int)textFormat.mpTextLocation.x);
        this->ui->spinBox_ItemLocationY->setValue((int)textFormat.mpTextLocation.y);
        this->ui->spinBox_FontSize->setValue(textFormat.miTextSize);
        this->ui->spinBox_ItemWeight->setValue(textFormat.miSizeW);
        this->ui->spinBox_ItemHeight->setValue(textFormat.miSizeH);
        FlashTextsList();
    }
    
}


void Settings::on_pushButton_addInfo_clicked()
{
    QString result{ this->ui->lineEdit_changeInfo->text() };
    if (!result.isEmpty()) {
        int currentItem = this->ui->comboBox_InfoDays->currentIndex();
        std::wstring info = result.toStdWString();
        pParent->timetable->mAddMoreInfo(Days[currentItem], info);
    }
    FlashList();
    int count = this->ui->listView->count();
    this->ui->listView->setCurrentRow(count - 1);
}


void Settings::on_pushButton_chooseConfig_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择需要打开json"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"JSON文件(*.json);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
    }
    else {
        this->ui->lineEdit_ConfigPath->setText(openFileName);
    }
}


void Settings::on_pushButton_chooseLessons_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择需要打开json"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"JSON文件(*.json);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
    }
    else {
        this->ui->lineEdit_LessonPath->setText(openFileName);
    }
}


void Settings::on_pushButton_applyConfigPath_clicked()
{
    pParent->windowsettings->mChangeConfigPath(this->ui->lineEdit_ConfigPath->text().toStdWString());
    pParent->windowsettings->mGetWindowSettings();
    pParent->timetable->mReplacePath(pParent->windowsettings->msLessonInfoFile);
    this->ui->lineEdit_LessonPath->setText(QString::fromStdWString(pParent->windowsettings->msLessonInfoFile));
    pParent->mInitializeWindow();
}


void Settings::on_pushButton_applyLessonPath_clicked()
{
    pParent->windowsettings->msLessonInfoFile = this->ui->lineEdit_LessonPath->text().toStdWString();
    pParent->timetable->mReplacePath(pParent->windowsettings->msLessonInfoFile);
    pParent->timetable->mReloadLesson();
}


void Settings::on_pushButton_chooseBackGround_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择图片"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"图片文件(*.jpg;*.png);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
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
        this->ui->lineEdit_color->setText(QString::fromStdWString(ColorRefToHexString(colorRef)));
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
    else if (this->ui->radioButton_weather->isChecked()) {
        type = TextType::Weather;
    }
    else if (this->ui->radioButton_news->isChecked()) {
        type = TextType::News;
    }
    else {
        type = TextType::Default;
    }
    TextItem format;
    format.text = this->ui->lineEdit_textFormat->text().toStdWString();
    format.type = type;
    return format;
}

TextFormat Settings::ReadFormatFromUI()
{
    int formatIndex = this->ui->listView->currentRow();
    auto format = pParent->windowsettings->msTextFormat[formatIndex];
    format.color = HexStringToColorRef(this->ui->lineEdit_color->text().toStdWString());
    format.msFontName = this->ui->fontComboBox_textFont->currentText().toStdWString();
    format.mpTextLocation.x = this->ui->spinBox_ItemLocationX->value();
    format.mpTextLocation.y = this->ui->spinBox_ItemLocationY->value();
    format.miTextSize = this->ui->spinBox_FontSize->value();
    format.miSizeW = this->ui->spinBox_ItemWeight->value();
    format.miSizeH = this->ui->spinBox_ItemHeight->value();
    return format;
}

std::wstring Settings::ColorRefToHexString(COLORREF& color)
{
    int red = GetRValue(color);
    int blue = GetBValue(color);
    int green = GetGValue(color);
    return std::wstring(std::format(L"#{:02x}{:02x}{:02x}",red,green,blue));
}

COLORREF Settings::HexStringToColorRef(const std::wstring& input)
{
    int red;
    int blue;
    int green;
    int i = swscanf_s(input.c_str(), L"#%02x%02x%02x", &red,&green,&blue);
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
    int currentRow = this->ui->listView->currentRow();
    
    if (currentRow >= 0 && currentRow < pParent->windowsettings->msTextFormat.size())
    {
        auto& textFormat = pParent->windowsettings->msTextFormat[currentRow];
        int rowToDelete = this->ui->listWidget_textItems->currentRow();
        
        if (rowToDelete >= 0 && rowToDelete < textFormat.Texts.size())
        {
            textFormat.Texts.erase(textFormat.Texts.begin() + rowToDelete);
            pParent->windowsettings->changedItems.push_back(currentRow);
            FlashList();
            this->ui->listWidget_textItems->setCurrentRow(this->ui->listWidget_textItems->count() - 1);
        }
    }
}

void Settings::on_listWidget_textItems_currentRowChanged(int currentRow)
{
    if (currentRow < 0) {
        return;
    }
    auto& textItem = pParent->windowsettings->msTextFormat[this->ui->listView->currentRow()];
    ui->lineEdit_textFormat->setText(QString::fromStdWString(textItem.Texts[currentRow].text));
    switch (pParent->windowsettings->msTextFormat[ui->listView->currentRow()].Texts[currentRow].type)
    {
    case TextType::CurrentTime:
        this->ui->radioButton_time->setChecked(true);
        break;
    case TextType::CurrentLesson:
        this->ui->radioButton_lesson->setChecked(true);
        break;
    case TextType::CountDownDay:
        this->ui->radioButton_countDownDay->setChecked(true);
        break;
    case TextType::Info:
        this->ui->radioButton_info->setChecked(true);
        break;
    case TextType::Weather:
        this->ui->radioButton_weather->setChecked(true);
        break;
    case TextType::News:
        this->ui->radioButton_news->setChecked(true);
        break;
    default:
        break;
    }
}

//reload settings form file to ui
void Settings::on_pushButton_changeInfo_clicked()
{
    pParent->timetable->changeInfo(this->ui->listView->currentRow(),this->ui->comboBox_InfoDays->currentText().toStdWString(), this->ui->lineEdit_changeInfo->text().toStdWString());
    pParent->timetable->mReloadLesson();
    FlashList();
}


void Settings::on_pushButton_changeText_clicked()
{
    pParent->windowsettings->msTextFormat[this->ui->listView->currentRow()].Texts[this->ui->listWidget_textItems->currentRow()] = ReadTextsFromUI();
    pParent->windowsettings->changedItems.push_back(this->ui->listView->currentRow());
    FlashList();
}


void Settings::on_pushButton_changeGroupSettings_clicked()
{
    auto textFormat = ReadFormatFromUI();
    pParent->windowsettings->msTextFormat[this->ui->listView->currentRow()] = textFormat;
    pParent->windowsettings->changedItems.push_back(this->ui->listView->currentRow());
    FlashList();
}


void Settings::on_pushButton_chooseBackGroundColor_clicked()
{
    QColor color = QColorDialog::getColor(this->ui->lineEdit_backGroundColor->text(), this);
    if (color.isValid()) {
        COLORREF colorRef{ RGB(color.red(), color.green(), color.blue()) };
        this->ui->lineEdit_backGroundColor->setText(QString::fromStdWString(ColorRefToHexString(colorRef)));
    }
}


void Settings::on_checkBox_useColor_stateChanged(int arg1)
{
    this->ui->lineEdit_backGroundColor->setEnabled(arg1 == Qt::Checked);
    this->ui->horizontalSlider_alpha->setEnabled(arg1 == Qt::Checked);
    this->ui->pushButton_chooseBackGroundColor->setEnabled(arg1 == Qt::Checked);
    this->ui->lineEdit_backGroundImg->setEnabled(arg1 != Qt::Checked);
    this->ui->pushButton_chooseBackGround->setEnabled(arg1 != Qt::Checked);
}

