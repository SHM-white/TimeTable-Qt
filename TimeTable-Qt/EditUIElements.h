#pragma once

#include <QWidget>
#include "ui_EditUIElements.h"
#include "BasicWindow.h"
#include "include.h"

class EditUIElements : public BasicWindow
{
	Q_OBJECT

public:
	EditUIElements(std::shared_ptr<BasicWindow> window, Json::Value WindowSettings, QWidget* parent = nullptr);
	~EditUIElements();

private slots:
    void on_pushButton_close_clicked();

    void on_lineEdit_BackGroundColor_editingFinished();

    void on_pushButton_chooseColor_clicked();

    void on_fontComboBox_currentFontChanged(const QFont &f);

    void on_lineEdit_editingFinished();

    void on_pushButton_checkAndSaveData_clicked();

    void on_pushButton_addUIElement_clicked();

    void on_pushButton_deleteUIElemrnt_clicked();

    void on_pushButton_AddItem_clicked();

    void on_pushButton_deleteItem_clicked();

private:
    void FreshUIElementsList();
    void FreshElementsList();
	std::shared_ptr<BasicWindow> m_window;
	Ui::EditUIElementsClass ui;
    UIElementBase* GetCurrentElement();
	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
};
