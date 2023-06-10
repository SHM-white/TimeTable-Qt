#include "todayalllessons.h"
#include "ui_todayalllessons.h"
#include <string>
#include <windef.h>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <vector>


TodayAllLessons::TodayAllLessons(QWidget *parent) :
    QWidget(nullptr),
    ui(new Ui::TodayAllLessons)
{
    ui->setupUi(this);
    pParent = (TimeTableQt*)parent;
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

}

TodayAllLessons::~TodayAllLessons()
{
    delete ui;
}

void TodayAllLessons::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawRect(rect());
    std::vector<std::string> allLessons;
    pParent->timetable.mGetLesson(allLessons, pParent->timetable.mGetCurrentTime("%a"));
    painter.fillRect(rect(), QColor(255, 255, 255, 10));
}

void TodayAllLessons::closeEvent(QCloseEvent* event)
{
    pParent->ui.actionshowTodayAll->setChecked(false);
    event->accept();
}

void TodayAllLessons::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bDrag = true;
        mouseStartPoint = event->globalPos();
        windowTopLeftPoint = this->frameGeometry().topLeft();
        setCursor(QCursor(Qt::SizeAllCursor));
    }
}

void TodayAllLessons::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDrag)
    {
        QPoint distance = event->globalPos() - mouseStartPoint;
        this->move(windowTopLeftPoint + distance);
    }
}

void TodayAllLessons::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bDrag = false;
    }
    setCursor(QCursor(Qt::ArrowCursor));
}
