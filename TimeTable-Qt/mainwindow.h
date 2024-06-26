﻿#pragma once

#include <QWidget>
#include "include.h"
#include "ui_MainWindow.h"
#include "BasicWindow.h"
#include "SubWindow.h"
#include "Timetable.h"


class MainWindow : public BasicWindow
{
	Q_OBJECT

public:
	MainWindow(Json::Value& settings, QWidget* parent = nullptr);
	~MainWindow();
	void ReopenWindows();
	std::vector<std::shared_ptr<BasicWindow>> m_windows;
private:
	Ui::MainWindowClass ui;
	void CreateSystemTrayIcon();
	std::shared_ptr<BasicWindow> CreateSubWindows(Json::Value& settings,std::shared_ptr<TimeTable> timetable);
	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value value) const override;
	QSystemTrayIcon* trayIcon;
	QAction* action_autoLaunch;
	QTimer* timer;
private slots:

	void HideAllWindows();
	void updateWindowStatus() override;
	void ShowAllWindows();
	void LaunchAsSystemBoot();
	void OpenSetting();
	void OpenSubWindows();
	void Exit();
	void SelfInitial();
};
