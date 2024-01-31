#pragma once
#include "UIElementBase.h"
class MultiItemInOrderUIElementBase :
    public UIElementBase
{
protected:
    MultiItemInOrderUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable);
};

