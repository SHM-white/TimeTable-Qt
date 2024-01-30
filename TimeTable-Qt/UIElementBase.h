#pragma once
#include "include.h"
#include "TimeTable.h"

enum UIElementType 
{
	SingleItem,
	MultiItemInOrder,
	MultiItemAllDisplay
};
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
	virtual bool paint(QPainter& painter) const;
	virtual bool update() const;
	virtual QSize getNeededSize() const = 0;
	virtual QRect getNeededRect() const;
	Json::Value save() const;
private:
	std::shared_ptr<TimeTable> m_timetable;
	UIElementType m_type{ UIElementType::SingleItem };
	mutable std::chrono::system_clock::time_point m_lastUpdateTime;
	QRect m_rect;
	Json::Value m_selfJson;
	virtual Json::Value SaveAsJson(Json::Value& value) const = 0;
};

class SingleItemUIElementBase;

class MultiItemInOrderUIElementBase;

class MultiItemAllDisplayUIElementBase;

class EmptyUIElement :
	public UIElementBase
{
public:
	EmptyUIElement(Json::Value& value,std::shared_ptr<TimeTable> timetable);
	// 通过 UIElementBase 继承
	bool paint(QPainter& painter) const override;
	bool update() const override;
	QSize getNeededSize() const override;
	Json::Value SaveAsJson(Json::Value& value) const override;
};

