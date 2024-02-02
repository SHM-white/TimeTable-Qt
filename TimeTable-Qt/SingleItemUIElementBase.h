#pragma once
#include "UIElementBase.h"
class SingleItemUIElementBase :
    public UIElementBase
{
public:
    SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    bool SetChanged() const;
    mutable bool m_CanChange{ false };
protected:
    std::chrono::milliseconds m_CanChangeAfterTime;
    mutable std::chrono::system_clock::time_point m_lastChangedTime;
    ElementType m_ItemType;
};

