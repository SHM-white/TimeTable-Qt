#pragma once
#include "include.h"
class WindowItem
{
public:
	bool paint(QPainter& painter);
	bool m_needUpdate{true};
	bool m_updated{false};
	QString& Text();
	QPoint position;
	QSize size;
	QFont font;
	QColor color;
	int m_lastUpdateTime;
private:
	int m_printCount{0};
	QString text;
};

