#pragma once
#include "include.h"
class WindowItem
{
public:
	bool paint(QPainter& painter);
	QString text;
	QPoint position;
	QSize size;
	QFont font;
	QColor color;
};

