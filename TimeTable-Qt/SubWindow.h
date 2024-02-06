#pragma once

#include <QWidget>
#include "ui_SubWindow.h"
#include "BasicWindow.h"
#include "Timetable.h"

class SubWindow : public BasicWindow
{
	Q_OBJECT

public:
	SubWindow(Json::Value& settings,std::shared_ptr<TimeTable> timetable, QWidget *parent = nullptr);
	~SubWindow();

private:
	Ui::SubWindowClass ui;

	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
};
