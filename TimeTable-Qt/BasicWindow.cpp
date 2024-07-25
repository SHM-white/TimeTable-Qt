#include "BasicWindow.h"
#include "UIElementBase.h"
#include "EmptyUIElement.h"
#include "SingleItemUIElementBase.h"
#include "MultiItemInOrderUIElementBase.h"
#include "MultiItemAllDisplayUIElementBase.h"
#include "MultiTextItem.h"
#include "SingleTextItem.h"

BasicWindow::BasicWindow(Json::Value& settings, QWidget* parent)
	: QWidget(parent), m_settings{ settings }
{
	std::wstring path;
	Json::GetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
	m_TimeTable = std::make_shared<TimeTable>(path);

	InitializeWindow(settings);

	if (m_maxFPS != 0)
	{
		time_calendar_window = new QTimer(this);
		connect(time_calendar_window, SIGNAL(timeout()), this, SLOT(update()));
		time_calendar_window->start((int)1000 / m_maxFPS);
	}

	time_calendar_text = new QTimer(this);
	connect(time_calendar_text, SIGNAL(timeout()), this, SLOT(updateWindowStatus()));
	time_calendar_text->start(1000);

#ifdef DEBUG
	m_debug = true;
	m_showFPS = true;
#endif // DEBUG

}

BasicWindow::~BasicWindow()
{
	//delete time_calendar_window;
	//delete time_calendar_text;
}

Json::Value BasicWindow::save() const
{
	Json::Value Settings = m_settings;
	Settings["WindowSize"].clear();
	Settings["WindowSize"].append(miWindowWeight);
	Settings["WindowSize"].append(miWindowHeight);
	// Clear the "WindowLocation" array and append the window position and all lesson window position
	Settings["WindowLocation"].clear();
	Settings["WindowLocation"].append(miWindowX);
	Settings["WindowLocation"].append(miWindowY);
	Settings["ImgAsBackGround"] = mUseImgAsBackGround;
	Settings["BackGroundColor"].clear();
	for (auto color : miBackGroundColor) {
		Settings["BackGroundColor"].append(color);
	}
	Settings["AcrylicEffect"] = bAcrylicEffect;
	Settings["BackGroundImg"] = wtu8(msBackGroundImg);
	Settings["UIElements"].clear();
	for (const auto& i : m_UIElements) {
		Settings["UIElements"].append(i->save());
	}

	Settings["AutoOpen"] = m_AutoOpen;
	Settings["TopMost"] = m_TopMost;
	Settings["Moveable"] = m_moveable;
	Settings["FPS"] = m_maxFPS;
	Settings["AutoResize"] = m_autoResize;
	Settings["Name"] = wtu8(m_name);

	return SaveAsJson(Settings);
}

bool BasicWindow::InitializeWindow(Json::Value& value)
{
	m_settings = value;
	//if (value.isNull()&&m_rewriteEmptyJsonValue) {
	//	value = this->save();
	//}
	Json::Value Settings = value;
	miWindowHeight = Settings["WindowSize"][1].asInt();
	miWindowWeight = Settings["WindowSize"][0].asInt();
	miWindowX = Settings["WindowLocation"][0].asInt();
	miWindowY = Settings["WindowLocation"][1].asInt();

	mUseImgAsBackGround = Settings["ImgAsBackGround"].asBool();
	for (int i = 0; i < Settings["BackGroundColor"].size(); i++) {
		miBackGroundColor[i] = Settings["BackGroundColor"][i].asInt();
	}
	bAcrylicEffect = Settings["AcrylicEffect"].asBool();
	msBackGroundImg = u8tw(Settings["BackGroundImg"].asString());
	for (auto& i : Settings["UIElements"]) {
		m_UIElements.push_back(CreateUIElement(i, m_TimeTable));
	}

	m_AutoOpen = Settings["AutoOpen"].asBool();
	m_TopMost = Settings["TopMost"].asBool();
	m_moveable = Settings["Moveable"].asBool();
	m_maxFPS = (Settings["FPS"].asInt() < 0 ? 0 : Settings["FPS"].asInt());
	m_autoResize = Settings["AutoResize"].asBool();
	m_name = u8tw(Settings["Name"].asString());
	if (!(miWindowX == 0 && miWindowY == 0 && miWindowWeight == 0 && miWindowHeight == 0)) {
		this->setGeometry(miWindowX, miWindowY, miWindowWeight, miWindowHeight);
		this->resize(miWindowWeight, miWindowHeight);
	}

	QString picpath = QString::fromStdWString(msBackGroundImg);
	//QString picpath = QString::fromUtf8(wtu8(msBackGroundImg));
	pic.load(picpath);
	pic = pic.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

#ifdef DEBUG
	if (pic.isNull()) {
		OutputDebugStringW(L"pic is null");
	}
#endif // DEBUG

	setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground);
	flags = windowFlags();
	if (m_TopMost) {
		setWindowFlags(flags | Qt::WindowStaysOnTopHint);
	}
	else {
		setWindowFlags(flags);
	}
	if (m_AutoOpen) {
		m_hide = false;
	}
	else {
		m_hide = true;
	}
	return true;
}

void BasicWindow::updateWindowStatus()
{
	m_lastFPS = m_currentFPS;
	m_currentFPS = 0;
	if (m_hide) {
		this->hide();
	}
	else
	{
		this->show();
	}
	if (m_autoResize) {
		QRect rect;
		for (auto& i : m_UIElements) {
			auto a = i->getNeededRect();
			rect.setRight(std::max(a.right(), rect.right()));
			rect.setBottom(std::max(a.bottom(), rect.bottom()));
		}
		setFixedSize(rect.right(), rect.bottom());
	}
}

void BasicWindow::closeEvent(QCloseEvent* event)
{
	m_hide = true;
	event->accept();
}

void BasicWindow::paintEvent(QPaintEvent* event)
{
	if (m_hide == true) {
		return;
	}
	m_currentFPS += 1;
	QPainter painter(this);
	// Draw background image
	if (pic.isNull() || !(mUseImgAsBackGround)) {
		if (mUseImgAsBackGround) {
			//QMessageBox::warning(this, "", "pixmap is null");
		}
		painter.fillRect(rect(), QColor(miBackGroundColor[0], miBackGroundColor[1], miBackGroundColor[2], miBackGroundColor[3]));
	}
	else {
		painter.fillRect(rect(), QColor(255, 255, 255, 1));
		painter.drawPixmap(0, 0, pic);
	}

	for (auto& i : m_UIElements) {
		i->paint(painter);
		if (m_debug) {
			i->paintRect(painter);
		}
	}	
	if (m_debug)
	{
		painter.drawRect(rect());
	}
	if (m_showFPS) {
		painter.save();
		painter.setPen(QColor(0, 255, 0));
		QRect temp_fpsRect{ 0, 0, 20, 15 };
		painter.drawText(temp_fpsRect, Qt::AlignCenter, QString::number(m_lastFPS));
		painter.restore();
	}
	painter.end();
}

void BasicWindow::mousePressEvent(QMouseEvent* event)
{
	if ((event->button() == Qt::LeftButton)&&m_moveable)
	{
		m_bDrag = true;
		mouseStartPoint = event->globalPos();
		windowTopLeftPoint = this->frameGeometry().topLeft();
		setCursor(QCursor(Qt::SizeAllCursor));
	}
}

void BasicWindow::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bDrag)
	{
		QPoint distance = event->globalPos() - mouseStartPoint;
		this->move(windowTopLeftPoint + distance);
	}
}

void BasicWindow::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bDrag = false;
	}
	setCursor(QCursor(Qt::ArrowCursor));

}

bool BasicWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	MSG* msg = static_cast<MSG*>(message);
	if (msg->message == WM_NCHITTEST) {
		*result = 0;
		const int BORDER_WIDTH = 8; // 拉伸边框的宽度
		RECT winrect;
		GetWindowRect(reinterpret_cast<HWND>(winId()), &winrect);
		long x = GET_X_LPARAM(msg->lParam);
		long y = GET_Y_LPARAM(msg->lParam);
		// 判断鼠标位置是否在拉伸区域内
		if (y < winrect.top + BORDER_WIDTH) {
			*result = HTTOP;
		}
		if (y > winrect.bottom - BORDER_WIDTH) {
			*result = HTBOTTOM;
		}
		if (x < winrect.left + BORDER_WIDTH) {
			*result = HTLEFT;
		}
		if (x > winrect.right - BORDER_WIDTH) {
			*result = HTRIGHT;
		}
		if (y < winrect.top + BORDER_WIDTH && x < winrect.left + BORDER_WIDTH) {
			*result = HTTOPLEFT;
		}
		if (y < winrect.top + BORDER_WIDTH && x > winrect.right - BORDER_WIDTH) {
			*result = HTTOPRIGHT;
		}
		if (y > winrect.bottom - BORDER_WIDTH && x < winrect.left + BORDER_WIDTH) {
			*result = HTBOTTOMLEFT;
		}
		if (y > winrect.bottom - BORDER_WIDTH && x > winrect.right - BORDER_WIDTH) {
			*result = HTBOTTOMRIGHT;
		}
		if (*result != 0) {
			return QWidget::nativeEvent(eventType, message, result);
		}
	}
	return QWidget::nativeEvent(eventType, message, result);
}

std::shared_ptr<UIElementBase> BasicWindow::CreateUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
{
	switch (UIElementType(value["ElementType"].asInt()))
	{
	case SingleItem:
		return std::dynamic_pointer_cast<UIElementBase, SingleItemUIElementBase>(CreateSingleItemUIElement(value, timetable));
		break;
	case MultiItemInOrder:
		return std::dynamic_pointer_cast<UIElementBase, MultiItemInOrderUIElementBase>(CreateMultiItemInOrderUIElement(value, timetable));
		break;
	case MultiItemAllDisplay:
		//return std::dynamic_pointer_cast<UIElementBase, MultiItemAllDisplayUIElementBase>(CreateMultiItemAllDisplayUIElement(value, timetable));
		//break;
	default:
		return std::dynamic_pointer_cast<UIElementBase, EmptyUIElement>(std::make_shared<EmptyUIElement>(value, timetable));
		break;
	}
}
