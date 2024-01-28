#pragma once

#include <QWidget>
#include "include.h"
#include "ui_MainWindow.h"
#include "BasicWindow.h"

class MainWindow : public BasicWindow
{
	Q_OBJECT

public:
	MainWindow(Json::Value& settings, QWidget* parent = nullptr);
	~MainWindow();
	std::vector<std::shared_ptr<BasicWindow>> m_windows;
private:
	Ui::MainWindowClass ui;
	void CreateSystemTrayIcon();
	// 通过 BasicWindow 继承
	Json::Value SaveJson(Json::Value& value) const override;
};
