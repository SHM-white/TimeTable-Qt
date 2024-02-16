#pragma once
#include "SingleItemUIElementBase.h"
class TodayAllLessons :
    public SingleItemUIElementBase
{
public:
    TodayAllLessons(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    // 通过 SingleItemUIElementBase 继承
    bool paint(QPainter& painter) const override;
    QSize getNeededSize() const override;
    Json::Value SaveAsJson(Json::Value& value) const override;
    
};

