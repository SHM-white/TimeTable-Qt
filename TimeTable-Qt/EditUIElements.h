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

private:
	std::shared_ptr<BasicWindow> m_window;
	Ui::EditUIElementsClass ui;

	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
};
