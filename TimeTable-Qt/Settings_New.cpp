#include "Settings_New.h"

Settings_New::Settings_New(Json::Value ApplicationRootSettings, Json::Value WindowSettings, QWidget* parent)
	: BasicWindow(WindowSettings, parent), m_ApplicationSettings{ ApplicationRootSettings }
{
	ui.setupUi(this);
	m_hide = false;
	m_moveable = true;
}

Settings_New::~Settings_New()
{}

Json::Value Settings_New::SaveAsJson(Json::Value value) const
{
	return value;
}

void Settings_New::on_pushButton_close_clicked()
{
    this->close();
}


void Settings_New::on_spinBox_BackGroundAlpha_valueChanged(int arg1)
{
    this->ui.horizontalSlider_BackGroundAlpha->setValue(arg1);
}


void Settings_New::on_horizontalSlider_BackGroundAlpha_valueChanged(int value)
{
    this->ui.spinBox_BackGroundAlpha->setValue(value);
}

