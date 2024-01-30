#pragma once
#include "include.h"
#include "TimeTable.h"

enum ElementType
{
	Default,
	CurrentTime,
	CurrentLesson,
	CurrentLessonProgress,
	NextLesson,
	NextLessonTime,
	CountDownDay,
	TodayInfo,
	AllLessons,
	News,
	Weather,
	WeatherBlock,
	Clock,
	DailyWord,
};

class UIElementBase
{
public:
	UIElementBase(Json::Value& setting, std::shared_ptr<TimeTable> timetable);
	virtual bool paintRect(QPainter& painter) const;
	virtual bool paint(QPainter& painter) const = 0;
	virtual bool update(QPainter& painter) const = 0;
	virtual QSize getNeededSize() const = 0;
	Json::Value save() const;
private:
	std::shared_ptr<TimeTable> m_timetable;
	ElementType m_type{ ElementType::Default };
	QRect m_rect;
	virtual Json::Value SaveAsJson(Json::Value& value) const = 0;
};

class DefaultUIElement :
	public UIElementBase
{
public:
	DefaultUIElement(Json::Value& value,std::shared_ptr<TimeTable> timetable);
	// 通过 UIElementBase 继承
	bool paint(QPainter& painter) const override;
	bool update(QPainter& painter) const override;
	QSize getNeededSize() const override;
	Json::Value SaveAsJson(Json::Value& value) const override;
};

