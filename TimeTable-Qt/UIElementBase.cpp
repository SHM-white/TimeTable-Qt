#include "UIElementBase.h"

UIElementBase::UIElementBase(Json::Value& setting, std::shared_ptr<TimeTable> timetable)
	:m_timetable{ timetable }, m_selfJson{ setting }
{
	m_rect = QRect(setting["Location"][0].asInt(), setting["Location"][1].asInt(), setting["Size"][0].asInt(), setting["Size"][1].asInt());
	this->update();
}

bool UIElementBase::paintRect(QPainter& painter) const
{
	painter.save();
	painter.drawRect(m_rect);
	painter.restore();
	return false;
}

bool UIElementBase::paint(QPainter& painter) const
{
	return false;
}

bool UIElementBase::update() const
{
	m_lastUpdateTime = std::chrono::system_clock::now();
	return false;
}

QRect UIElementBase::getNeededRect() const
{
	return m_rect;
}

Json::Value UIElementBase::save() const
{
	Json::Value value = m_selfJson;
	value["Location"].clear();
	value["Size"].clear();
	value["Location"].append(m_rect.left());
	value["Location"].append(m_rect.top());
	value["Size"].append(m_rect.width());
	value["Size"].append(m_rect.height());
	return SaveAsJson(value);
}

