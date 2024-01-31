#pragma once
#include "SingleItemUIElementBase.h"
class SingleTextItem :
    public SingleItemUIElementBase
{
public:

    SingleTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    // 通过 SingleItemUIElementBase 继承
    QSize getNeededSize() const override;
    Json::Value SaveAsJson(Json::Value& value) const override;
protected:
    std::wstring m_textFormat;
    QFont m_font;
    QColor m_color;
    ElementType m_textType;
};

