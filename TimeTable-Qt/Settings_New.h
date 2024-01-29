#pragma once

#include <QWidget>
#include "ui_Settings_New.h"

class Settings_New : public QWidget
{
	Q_OBJECT

public:
	Settings_New(QWidget *parent = nullptr);
	~Settings_New();

private:
	Ui::Settings_NewClass ui;
};
