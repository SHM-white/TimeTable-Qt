#pragma once

#include <QWidget>
#include "ui_Settings_New.h"
#include "BasicWindow.h"

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

private:
	QWidget* m_pParent;
	Ui::Settings_NewClass ui;
	std::vector<std::shared_ptr<BasicWindow>>* m_Windows;
	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
};
