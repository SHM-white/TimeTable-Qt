#include "SubWindow.h"

SubWindow::SubWindow(Json::Value& settings, QWidget* parent)
	: BasicWindow(settings, parent)
{
	ui.setupUi(this);
}

SubWindow::~SubWindow()
{}
