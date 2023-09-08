#pragma once
//#pragma execution_character_set("utf-8")

#include "include.h"
#include "WindowItem.h"
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
    std::shared_ptr<WindowSettings> windowsettings;
    std::shared_ptr<TimeTable> timetable;
    static std::wstring translateUtfToAnsi(const std::wstring& input);
    Ui::TimeTableQtClass ui;
private:
    Qt::WindowFlags flags;
    QPixmap pic;
    int timeCounter{ 0 };
    bool m_bDrag{ false };
    QTimer* time_calendar_window;
    QTimer* time_calendar_text;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
    QPoint MenuRD;
    std::vector<WindowItem> items;
    void ShowShadow();
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
private slots:
    void updateTexts();
    void updateWindow();
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
