#pragma once

#include <QWidget>
#include "ui_MainWindow.h"
#include "BasicWindow.h"

class MainWindow : public BasicWindow
{
	Q_OBJECT

public:
	MainWindow(Json::Value& settings, QWidget* parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindowClass ui;

	// 通过 BasicWindow 继承
	Json::Value SaveJson(Json::Value& value) const override;
};
