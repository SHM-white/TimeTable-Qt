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

