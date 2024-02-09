#pragma once

#include <QWidget>
#include "ui_Settings_New.h"
#include "BasicWindow.h"
#include "mainwindow.h"

class Settings_New : public BasicWindow
{
	Q_OBJECT

public:
	Settings_New(std::vector<std::shared_ptr<BasicWindow>>* Windows, Json::Value WindowSettings, QWidget *parent = nullptr);
	~Settings_New();

private slots:

    void on_pushButton_close_clicked();

    void on_spinBox_BackGroundAlpha_valueChanged(int arg1);

    void on_horizontalSlider_BackGroundAlpha_valueChanged(int value);

    void on_pushButton_FreshLessonList_clicked();

    void on_pushButton_SortLessons_clicked();

    void on_pushButton_DeleteLesson_clicked();

    void on_pushButton_AddLesson_clicked();

    void on_pushButton_AddInfo_clicked();

    void on_pushButton_DeleteInfo_clicked();

    void on_pushButton_FreshInfo_clicked();

    void on_pushButton_Cancel_clicked();

    void on_pushButton_Save_clicked();

    void on_pushButton_EditElements_clicked();

    void on_pushButton_ChooseColor_clicked();

    void on_lineEdit_BackGroundColor_editingFinished();

    void on_checkBox_ColorBackGround_stateChanged(int arg1);

    void on_checkBox_AutoResize_stateChanged(int arg1);

    void on_checkBox_Moveable_stateChanged(int arg1);

    void on_checkBox_TopMost_stateChanged(int arg1);

    void on_spinBox_W_valueChanged(int arg1);

    void on_spinBox_H_valueChanged(int arg1);

    void on_spinBox_X_valueChanged(int arg1);

    void on_spinBox_Y_valueChanged(int arg1);

    void on_spinBox_FPS_valueChanged(int arg1);

    void on_checkBox_AutoOpen_stateChanged(int arg1);

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_pushButton_newWindow_clicked();

    void on_pushButton_deleteWindow_clicked();

    void on_pushButton_FreshWindowList_clicked();

    void on_pushButton_reloadWindows_clicked();

    void on_listWidget_Windows_currentRowChanged(int currentRow);

    void on_listWidget_Windows_itemChanged(QListWidgetItem *item);

    void on_listWidget_Infos_itemChanged(QListWidgetItem *item);

    void on_spinBox_scrollSpeed_valueChanged(int arg1);

    void on_comboBox_LessonDay_currentIndexChanged(int index);

    void on_LessonComboBox_TextChanged(QString arg);

    void on_tableWidget_Lessons_itemChanged(QTableWidgetItem *item);

    void on_pushButton_ChangeConfigPath_clicked();

    void on_pushButton_ChangeLessonPath_clicked();

    void on_pushButton_ApplyConfigPath_clicked();

    void on_pushButton_ApplyLessonPath_clicked();

    void on_pushButton_restoreConfigPath_clicked();

    void on_pushButton_restoreLessonPath_clicked();

    void on_lineEdit_BackgroundImg_textChanged(const QString &arg1);

    void on_pushButton_ChooseBackgroundImg_clicked();

    void on_checkBox_debug_stateChanged(int arg1);

private:
    void LessonChangeRow(int row);
	QWidget* m_pParent;
	Ui::Settings_NewClass ui;
	std::vector<std::shared_ptr<BasicWindow>>* m_Windows;
    std::shared_ptr<BasicWindow> GetCurrentWindow();
	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
    bool ListsInitialized{ false };
    std::vector<QComboBox*> comboBoxes;
};
