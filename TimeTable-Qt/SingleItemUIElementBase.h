#pragma once
#include "UIElementBase.h"
class SingleItemUIElementBase :
    public UIElementBase
{
public:
    SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable);
protected:
    ElementType m_ItemType;
};

