#include "EmptyUIElement.h"

EmptyUIElement::EmptyUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
	m_type = UIElementType::SingleItem;
}

bool EmptyUIElement::paint(QPainter& painter) const
{
	return false;
}

bool EmptyUIElement::update() const
{
	return false;
}

QSize EmptyUIElement::getNeededSize() const
{
	return QSize();
}

Json::Value EmptyUIElement::SaveAsJson(Json::Value& value) const
{
	return value;
}
