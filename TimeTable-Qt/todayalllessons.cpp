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
    QWidget(parent),
    ui(new Ui::TodayAllLessons)
{
    ui->setupUi(this);
    pParent = (TimeTableQt*)parentWidget();
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

}

TodayAllLessons::~TodayAllLessons()
{
    delete ui;
}

void TodayAllLessons::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.drawRect(rect());
    std::vector<std::string> allLessons;
    pParent->timetable.mGetLesson(allLessons, pParent->timetable.mGetCurrentTime("%a"));

}
