#include "BasicWindow.h"

BasicWindow::BasicWindow(Json::Value& settings, QWidget* parent)
	: QWidget(parent)
{
	time_calendar_window = new QTimer(this);
	connect(time_calendar_window, SIGNAL(timeout()), this, SLOT(updateWindow()));
	time_calendar_window->start((int)100 / 6);
	InitializeWindow(settings);
#ifdef DEBUG
	m_debug = true;
#endif // DEBUG

}

BasicWindow::~BasicWindow()
{}

Json::Value BasicWindow::save() const
{
	Json::Value value;
	return SaveJson(value);
}

bool BasicWindow::InitializeWindow(Json::Value& value)
{
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

	m_AutoOpen = Settings["AutoOpen"].asBool();
	m_TopMost = Settings["TopMost"].asBool();
	m_moveable = Settings["Moveable"].asBool();

	if (!(miWindowX == 0 && miWindowY == 0 && miWindowWeight == 0 && miWindowHeight == 0)) {
		this->setGeometry(miWindowX, miWindowY, miWindowWeight, miWindowHeight);
		this->setFixedSize(miWindowWeight, miWindowHeight);
	}

	QString picpath = QString::fromStdWString(msBackGroundImg);
	pic = QPixmap(picpath);
	pic = pic.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

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
		this->show();
	}
	else {
		this->hide();
	}
	return true;
}

void BasicWindow::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (m_debug)
	{
		painter.drawRect(rect());
	}
	// Draw background image
	if (pic.isNull() || !(mUseImgAsBackGround)) {
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


