#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(windowsettings.miWindowWeight,windowsettings.miWindowHeight);
    time_calendar= new QTimer(this);
    connect(time_calendar,SIGNAL(timeout()),this,SLOT(UpdateWindow()));
    time_calendar->start(1000);
}

MainWindow::~MainWindow()
{
    delete time_calendar;
    delete ui;
}

void MainWindow::UpdateWindow(){
    //Q_UNUSED(event);
    QPainter painter(this);
    //painter.begin(0);
    int i=1;
    for(TextFormat a:windowsettings.msTextFormat){
        QFont font;
        painter.setPen(a.color);
        font.setFamily(a.msFontName.c_str());
        font.setPointSize(a.miTextSize);
        painter.setFont(font);
        std::string Text;
        if (i == windowsettings.miLessonInLine) {
            Text = timetable.mGetCurrentTime(a.msTextFormat) + timetable.mGetCurrentLesson(windowsettings.msLessonNull);
        }
        else if (i == windowsettings.miCountDownDayInLine) {
            Text = timetable.mGetCountDown(windowsettings.mCountDownDay, a.msTextFormat);
        }
        else {
            Text = timetable.mGetCurrentTime(a.msTextFormat);
        }
        painter.drawText(a.mpTextLocation.x,a.mpTextLocation.y,Text.c_str());
        i++;
    }
    //painter.end();
}
