#include "TimeTableQt.h"
#include <string>
#include <windef.h>

TimeTableQt::TimeTableQt(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setFixedSize(windowsettings.miWindowWeight, windowsettings.miWindowHeight);
    time_calendar = new QTimer(this);
    connect(time_calendar, SIGNAL(timeout()), this, SLOT(UpdateWindow()));
    time_calendar->start(1000);
}

TimeTableQt::~TimeTableQt()
{
    delete time_calendar;
}

void TimeTableQt::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    //painter.begin(this);
    int i = 1;
    for (TextFormat a : windowsettings.msTextFormat) {
        QFont font;
        
        painter.setPen(QColor(GetRValue(a.color),GetGValue(a.color),GetBValue(a.color)));
        font.setFamily(QString::fromLocal8Bit(a.msFontName.c_str(),a.msFontName.size()));
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
        
        QString qtext = QString::fromLocal8Bit(Text.c_str(), Text.size());
        painter.drawText(a.mpTextLocation.x, a.mpTextLocation.y, qtext);
        i++;
        
    }
    painter.end();
    
}

void TimeTableQt::UpdateWindow() {
    update();
}

