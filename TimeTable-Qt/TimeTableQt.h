#pragma once
//#pragma execution_character_set("utf-8")

#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <qpixmap.h>
#include "ui_TimeTableQt.h"
#include "WindowSettings.h"
//#include "todayalllessons.h"

class QMouseEvent;

class TimeTableQt : public QWidget
{
    Q_OBJECT

public:
    TimeTableQt(QWidget *parent = nullptr);
    ~TimeTableQt();
    bool mInitializeWindow();
    WindowSettings windowsettings{ ".\\Config.json" };
    TimeTable timetable{ windowsettings.msLessonInfoFile };
    static std::string translateUtfToAnsi(const std::string& input);
    Ui::TimeTableQtClass ui;
private:
    Qt::WindowFlags flags;
    QPixmap pic;
    int timeCounter{ 0 };
    bool m_bDrag{ false };
    QTimer* time_calendar;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
    QPoint MenuRD;
    void ShowShadow();
    void showEvent(QShowEvent* event) override;
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
    void on_actionChangeConfig_triggered();
    void on_actionSettings_triggered();
    void on_actionBootAtPowerOn_triggered();
    void on_actionshowTodayAll_triggered();
};
