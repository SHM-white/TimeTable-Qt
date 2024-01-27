#include "UIElementBase.h"

UIElementBase::UIElementBase(Json::Value& setting)
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
	return SaveJson(value);
}
