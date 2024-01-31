﻿#include "MultiItemInOrderUIElementBase.h"

MultiItemInOrderUIElementBase::MultiItemInOrderUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
	m_type = UIElementType::MultiItemInOrder;
}
