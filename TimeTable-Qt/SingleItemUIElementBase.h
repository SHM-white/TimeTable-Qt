#pragma once
#include "UIElementBase.h"
class SingleItemUIElementBase :
    public UIElementBase
{
public:
    SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    virtual bool SetChanged();
    mutable bool m_CanChange{ false };
    QFont m_font;
    QColor m_color;
    ContentType m_textType;
    mutable Json::Value m_Data;
protected:
    mutable std::chrono::seconds m_CanChangeAfterTime;
    mutable std::chrono::system_clock::time_point m_lastChangedTime;
    ContentType m_ItemType;
};

