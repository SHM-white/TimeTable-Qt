#pragma once

#include <QWidget>
#include "include.h"
#include "UIElementBase.h"

class BasicWindow : public QWidget
{
	Q_OBJECT

public:
	BasicWindow(Json::Value& settings, QWidget* parent = nullptr);
	~BasicWindow();

	Json::Value save() const;

	bool InitializeWindow(Json::Value& value);

	void hideEvent(QHideEvent* event) override;
	void closeEvent(QCloseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;

	int miWindowHeight{ 120 };
	int miWindowWeight{ 250 };
	int miWindowX{ 20 };
	int miWindowY{ 20 };

	bool mUseImgAsBackGround{ true };
	std::wstring msBackGroundImg{ L"" };
	bool bAcrylicEffect{ false };
	int miBackGroundColor[4]{ 255,255,255,255 };

	bool m_AutoOpen{ false };
	bool m_TopMost{ false };

	bool m_debug{ false };
private:
	Qt::WindowFlags flags;
	QPixmap pic;
	int timeCounter{ 0 };
	bool m_bDrag{ false };
	QTimer* time_calendar_window;
	QTimer* time_calendar_text;
	QPoint mouseStartPoint;

	virtual Json::Value SaveJson(Json::Value& value) const = 0;

	std::vector<std::shared_ptr<UIElementBase>> m_UIElements;

};
