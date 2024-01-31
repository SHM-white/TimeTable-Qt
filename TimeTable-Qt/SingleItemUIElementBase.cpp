#include "SingleItemUIElementBase.h"

SingleItemUIElementBase::SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
	m_type = UIElementType::SingleItem;
}
