﻿#include "todayalllessons.h"
#include "ui_todayalllessons.h"
#include <string>
#include <windef.h>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <algorithm>
#include <vector>


TodayAllLessons::TodayAllLessons(QWidget *parent) :
    QWidget(nullptr),
    ui(new Ui::TodayAllLessons)
{
    time_calendar = new QTimer(this);
    connect(time_calendar, SIGNAL(timeout()), this, SLOT(update()));
    time_calendar->start(5000);

    ui->setupUi(this);
    pParent = (TimeTableQt*)parent;
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QRect windowRect = geometry();
    windowRect.setY(10);
    windowRect.setX(pParent->windowsettings.miWindowX-500);
    setGeometry(windowRect);
}

TodayAllLessons::~TodayAllLessons()
{
    delete ui;
}

void TodayAllLessons::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    std::vector<Lesson> allLessons;
    pParent->timetable.mGetLesson(allLessons, pParent->timetable.mGetCurrentTime("%a"));
    std::vector<QString> qAllLessons;
    for (const auto& i : allLessons) {
        qAllLessons.push_back(QString::fromStdString(i.mGetName()));
    }
    painter.fillRect(rect(), QColor(255, 255, 255, 100));
    painter.setPen(QPen(QBrush(QColor(0, 0, 0, 255)), 2));
    painter.drawRect(rect());
    QFont font(QString("微软雅黑"),14);
    painter.setFont(font);
    int maxLength{ 0 };
    for (const auto& i : qAllLessons) {
        maxLength = max(maxLength, i.length());
    }
    QFontMetrics fontMetrics = painter.fontMetrics();
    setFixedSize(qAllLessons.size() * fontMetrics.maxWidth(), (int)(maxLength * fontMetrics.height())+5);
    for (int i = 0; i < qAllLessons.size(); i++) {
        QRect rect(i * fontMetrics.maxWidth(), 0, fontMetrics.maxWidth(), maxLength * fontMetrics.height());
        painter.drawText(rect, Qt::TextWordWrap | Qt::AlignVCenter, qAllLessons[i]);
    }
    int currentLessonIndex = pParent->timetable.mGetCurrentLesson(0);
    painter.setPen(QPen(QBrush(QColor(0, 128, 0, 255)), 2));
    painter.drawRect(currentLessonIndex * fontMetrics.maxWidth()-2, 2, fontMetrics.maxWidth(), maxLength * fontMetrics.height());
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
