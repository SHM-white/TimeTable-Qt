#pragma once

#include <QWidget>
#include "ui_Settings_New.h"
#include "BasicWindow.h"

class Settings_New : public BasicWindow
{
	Q_OBJECT

public:
	Settings_New(Json::Value ApplicationRootSettings, Json::Value WindowSettings, QWidget *parent = nullptr);
	~Settings_New();

private:
	Ui::Settings_NewClass ui;
	Json::Value m_ApplicationSettings;
	// 通过 BasicWindow 继承
	Json::Value SaveAsJson(Json::Value& value) const override;
};
