#include "EditUIElements.h"

EditUIElements::EditUIElements(std::shared_ptr<BasicWindow> window, Json::Value WindowSettings, QWidget* parent)
	: BasicWindow(WindowSettings,parent),m_window{window}
{
	ui.setupUi(this);
	m_hide = false;
	m_moveable = true;

	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(this, SIGNAL(QWidget::destroyed), this, SLOT(QDialog::deleteLater));
	this->setWindowModality(Qt::ApplicationModal);
}

EditUIElements::~EditUIElements()
{}

Json::Value EditUIElements::SaveAsJson(Json::Value value) const
{
	return value;
}
