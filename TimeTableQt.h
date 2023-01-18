#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include <qtimer.h>
#include "ui_TimeTableQt.h"
#include "WindowSettings.h"


class TimeTableQt : public QWidget
{
    Q_OBJECT

public:
    TimeTableQt(QWidget *parent = nullptr);
    ~TimeTableQt();
    WindowSettings windowsettings{ ".\\Config.json" };
    TimeTable timetable{ windowsettings.msLessonInfoFile };
private:
    Ui::TimeTableQtClass ui;
    QTimer* time_calendar;
    void paintEvent(QPaintEvent*) override;
private slots:
    void UpdateWindow();
};
