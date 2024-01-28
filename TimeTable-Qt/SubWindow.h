#pragma once

#include <QWidget>
#include "ui_SubWindow.h"
#include "BasicWindow.h"

class SubWindow : public BasicWindow
{
	Q_OBJECT

public:
	SubWindow(Json::Value& settings, QWidget *parent = nullptr);
	~SubWindow();

private:
	Ui::SubWindowClass ui;
};
