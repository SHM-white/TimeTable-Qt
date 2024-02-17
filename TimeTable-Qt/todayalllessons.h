#pragma once
#include "SingleItemUIElementBase.h"
class TodayAllLessons :
    public SingleItemUIElementBase
{
public:
    TodayAllLessons(Json::Value& value, std::shared_ptr<TimeTable> timetable);
    // 通过 SingleItemUIElementBase 继承
    bool update() const override;
    bool paint(QPainter& painter) const override;
    QSize getNeededSize() const override;
    Json::Value SaveAsJson(Json::Value& value) const override;
    bool m_direction_Vertical{ 0 };
    mutable std::vector<Lesson> m_lessons;
    mutable int m_CurrentLessonIndex{ 0 };
    bool m_fullName{ false };
    mutable std::vector<QString> lessons;
};

