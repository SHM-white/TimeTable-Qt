#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <qpixmap.h>
#include "ui_TimeTableQt.h"
#include "WindowSettings.h"

class QMouseEvent;

class TimeTableQt : public QWidget
{
    Q_OBJECT

public:
    TimeTableQt(QWidget *parent = nullptr);
    ~TimeTableQt();
    WindowSettings windowsettings{ ".\\Config.json" };
    TimeTable timetable{ windowsettings.msLessonInfoFile };
private:
    QPixmap pic;
    bool m_bDrag{ false };
    Ui::TimeTableQtClass ui;
    QTimer* time_calendar;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
    QPoint MenuRD;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void UpdateWindow();
    void on_actionabout_triggered();
    void on_actionexit_triggered();
    void on_actioninport_triggered();
    void on_actiontotop_triggered();
    void on_actionminimize_triggered();
    void on_actionshowall_triggered();
    void on_actionmoreinfo_triggered();
};
