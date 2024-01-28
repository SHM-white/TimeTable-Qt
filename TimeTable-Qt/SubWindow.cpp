#include "SubWindow.h"

SubWindow::SubWindow(Json::Value& settings, std::shared_ptr<TimeTable> timetable, QWidget* parent)
	: BasicWindow(settings, parent)
{
	m_TimeTable = timetable;
	ui.setupUi(this);
}

SubWindow::~SubWindow()
{}

Json::Value SubWindow::SaveJson(Json::Value & value) const
{
	return value;
}
