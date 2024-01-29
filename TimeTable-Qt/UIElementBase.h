#pragma once
#include "include.h"

enum UIElementType
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
	UIElementBase(Json::Value& setting);
	virtual bool paintRect(QPainter& painter) const;
	virtual bool paint(QPainter& painter) const = 0;
	virtual bool update(QPainter& painter) const = 0;
	virtual QSize getNeededSize() const = 0;
	Json::Value save() const;
private:
	UIElementType m_type;
	QRect m_rect;
	virtual Json::Value SaveJson(Json::Value& value) const = 0;
};

