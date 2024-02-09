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
	NextLesson,
	NextLessonTime,
	CountDownDay,
	TodayInfo,
	News,
	DailyWord,
	Weather,
	AllLessons,
	CurrentLessonProgress,
	WeatherBlock,
	Clock,
};

class UIElementBase
{
public:
	UIElementBase(Json::Value& setting, std::shared_ptr<TimeTable> timetable);
	virtual ~UIElementBase() = default;
	virtual bool paintRect(QPainter& painter) const;
	virtual bool paint(QPainter& painter) const = 0;
	virtual bool update() const;
	virtual QSize getNeededSize() const = 0;
	virtual QRect getNeededRect() const;
	Json::Value save() const;
	UIElementType m_type;
	QRect m_rect;
	Json::Value m_selfJson;
protected:
	std::shared_ptr<TimeTable> m_timetable;
	mutable std::chrono::steady_clock::time_point m_lastUpdateTime;
	mutable std::chrono::seconds m_updateAfterTime;
	virtual Json::Value SaveAsJson(Json::Value& value) const = 0;
};

class SingleItemUIElementBase;
class SingleTextItem;
class MultiItemInOrderUIElementBase;
class MultiTextItem;
class MultiItemAllDisplayUIElementBase;
class EmptyUIElement;

std::shared_ptr<SingleItemUIElementBase> CreateSingleItemUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable);
std::shared_ptr<MultiItemInOrderUIElementBase> CreateMultiItemInOrderUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable);
std::shared_ptr<MultiItemAllDisplayUIElementBase> CreateMultiItemAllDisplayUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable);
