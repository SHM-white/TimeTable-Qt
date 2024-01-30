#include "UIElementBase.h"

UIElementBase::UIElementBase(Json::Value& setting, std::shared_ptr<TimeTable> timetable)
	:m_timetable{timetable}
{
	m_rect = QRect(setting["X"].asInt(), setting["Y"].asInt(), setting["W"].asInt(), setting["H"].asInt());
}

bool UIElementBase::paintRect(QPainter& painter) const
{
	
	painter.drawRect(m_rect);
	return false;
}

Json::Value UIElementBase::save() const
{
	Json::Value value;
	value["X"] = m_rect.left();
	value["Y"] = m_rect.top();
	value["W"] = m_rect.width();
	value["H"] = m_rect.height();
	return SaveAsJson(value);
}

DefaultUIElement::DefaultUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
}

bool DefaultUIElement::paint(QPainter& painter) const
{
	return false;
}

bool DefaultUIElement::update(QPainter& painter) const
{
	return false;
}

QSize DefaultUIElement::getNeededSize() const
{
	return QSize();
}

Json::Value DefaultUIElement::SaveAsJson(Json::Value& value) const
{
	return value;
}
