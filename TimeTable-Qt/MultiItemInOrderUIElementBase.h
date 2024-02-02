#pragma once
#include "UIElementBase.h"
class MultiItemInOrderUIElementBase :
    public UIElementBase
{
protected:
    mutable int m_currentIndex{ 0 };
    MultiItemInOrderUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable);
};

