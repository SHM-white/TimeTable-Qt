#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include "TimeTableQt.h"
#include <string>
#include <vector>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    void InitializeWindow();
    void SaveSettings();
    void FlashList(int index);
    void FlashList();
    void FlashTextsList();

private slots:
    void on_tabWidget_currentChanged(int index);

    void on_pushButton_delete_clicked();

    void on_pushButton_order_clicked();

    void on_checkBox_stateChanged(int arg1);

    void on_pushButton_close_clicked();

    void on_comboBox_LessonDays_currentIndexChanged(int index);

    void on_comboBox_InfoDays_currentTextChanged(const QString &arg1);

    void on_pushButton_addLesson_clicked();

    void on_pushButton_changeLesson_clicked();

    void on_listView_currentRowChanged(int currentRow);

    void on_pushButton_addInfo_clicked();

    void on_pushButton_chooseConfig_clicked();

    void on_pushButton_chooseLessons_clicked();

    void on_pushButton_applyConfigPath_clicked();

    void on_pushButton_applyLessonPath_clicked();

    void on_pushButton_chooseBackGround_clicked();

    void on_pushButton_chooseColor_clicked();

    void on_pushButton_addFormat_clicked();

    void on_pushButton_changeFormat_clicked();

    void on_pushButton_applySettings_clicked();

    void on_pushButton_cancelChange_clicked();

    void on_pushButton_saveChange_clicked();

    void on_pushButton_addTextItem_clicked();

    void on_pushButton_deleteTextItem_clicked();

    void on_listWidget_textItems_currentRowChanged(int currentRow);

    void on_pushButton_changeInfo_clicked();

    void on_pushButton_changeText_clicked();

    void on_pushButton_changeGroupSettings_clicked();

    void on_pushButton_chooseBackGroundColor_clicked();

    void on_checkBox_useColor_stateChanged(int arg1);

private:
    Ui::Settings *ui;
    TimeTableQt* pParent;
    std::string Days[7]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
    TextItem ReadTextsFromUI();
    TextFormat ReadFormatFromUI();

    static std::string ColorRefToHexString(COLORREF& color);
    static COLORREF HexStringToColorRef(const std::string& input);
};

#endif // SETTINGS_H
