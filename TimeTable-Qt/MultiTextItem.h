#pragma once
#include "MultiItemInOrderUIElementBase.h"
class MultiTextItem :
    public MultiItemInOrderUIElementBase
{
public:
    MultiTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    std::vector<std::shared_ptr<SingleTextItem>> m_Texts;

    // 通过 MultiItemInOrderUIElementBase 继承
    QSize getNeededSize() const override;
    Json::Value SaveAsJson(Json::Value& value) const override;
    bool update() const override;
    bool paint(QPainter& painter) const override;
};

