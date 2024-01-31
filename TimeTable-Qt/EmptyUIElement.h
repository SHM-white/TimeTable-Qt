#pragma once
#include "UIElementBase.h"
class EmptyUIElement :
	public UIElementBase
{
public:
	EmptyUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable);
	// 通过 UIElementBase 继承
	bool paint(QPainter& painter) const override;
	bool update() const override;
	QSize getNeededSize() const override;
	Json::Value SaveAsJson(Json::Value& value) const override;
};

