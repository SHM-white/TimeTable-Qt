#pragma once

#include <QWidget>
#include "include.h"
#include "UIElementBase.h"
#include "Timetable.h"
#include "qcoreapplication.h"

class BasicWindow : public QWidget
{
	Q_OBJECT

public:
	BasicWindow(Json::Value& settings, QWidget* parent = nullptr);
	~BasicWindow();

	Json::Value save() const;

	bool InitializeWindow(Json::Value& value);
	
	void closeEvent(QCloseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	int miWindowHeight{ 0 };
	int miWindowWeight{ 0 };
	int miWindowX{ 0 };
	int miWindowY{ 0 };

	int m_maxFPS{ 30 };

	bool mUseImgAsBackGround{ false };
	std::wstring msBackGroundImg{ L"" };
	bool bAcrylicEffect{ false };
	int miBackGroundColor[4]{ 255,255,255,128 };

	bool m_AutoOpen{ true };
	bool m_TopMost{ false };
	bool m_moveable{ false };
	bool m_autoResize{ false };

	bool m_debug{ false };
	bool m_showFPS{ false };

	bool m_hide{ false };
	bool m_rewriteEmptyJsonValue{ true };
private:
	Qt::WindowFlags flags;
	QPixmap pic;
	int timeCounter{ 0 };
	int m_lastFPS{ 0 };
	int m_currentFPS{ 0 };
	bool m_bDrag{ false };
	QTimer* time_calendar_window;
	QTimer* time_calendar_text;
	QPoint mouseStartPoint;
	QPoint windowTopLeftPoint;

	virtual Json::Value SaveAsJson(Json::Value& value) const = 0;

	std::shared_ptr<UIElementBase> CreateUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable);

	std::vector<std::shared_ptr<UIElementBase>> m_UIElements;
	Json::Value m_settings;
protected:
	std::shared_ptr<TimeTable> m_TimeTable;
	bool m_successfulInitialized{ false };
protected slots:
	virtual void updateWindowStatus();

};
