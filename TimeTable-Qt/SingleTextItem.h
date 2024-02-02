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
    std::wstring m_textFormat;
    mutable std::wstring m_formatedText;
    QFont m_font;
    QColor m_color;
    ElementType m_textType;
    mutable Json::Value m_Data;
    // 通过 SingleItemUIElementBase 继承
    bool update() const override;
    bool paint(QPainter& painter) const override;
};

