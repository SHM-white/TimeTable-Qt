#include "TimeTableQt.h"
#include "importfromfile.h"
#include "about.h"
#include "showalllessons.h"
#include "showmoreinfo.h"
#include <string>
#include <windef.h>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

TimeTableQt::TimeTableQt(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    this->setGeometry(windowsettings.miWindowX, windowsettings.miWindowY, windowsettings.miWindowWeight, windowsettings.miWindowHeight);
    this->setFixedSize(windowsettings.miWindowWeight, windowsettings.miWindowHeight);
    time_calendar = new QTimer(this);
    connect(time_calendar, SIGNAL(timeout()), this, SLOT(UpdateWindow()));
    time_calendar->start(1000);
    Qt::WindowFlags flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint);
    QString picpath = QString::fromLocal8Bit(windowsettings.msBackGroundImg.c_str());
    pic = QPixmap(picpath);
    this->ui.menubar->setVisible(false);
    MenuRD.setX(this->ui.menubar->width());
    MenuRD.setY(this->ui.menubar->height());
    this->setMouseTracking(true);
    show();
}

TimeTableQt::~TimeTableQt()
{
    delete time_calendar;
}

void TimeTableQt::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    
    if (!pic.isNull()) {
        pic = pic.scaled(this->width(), this->height());
        painter.drawPixmap(0, 0, this->width(), this->height(), pic);
    }
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

void TimeTableQt::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bDrag = true;
        mouseStartPoint = event->globalPos();
        windowTopLeftPoint = this->frameGeometry().topLeft();
        setCursor(QCursor(Qt::SizeAllCursor));
    }
}

void TimeTableQt::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bDrag)
    {
        QPoint distance = event->globalPos() - mouseStartPoint;
        this->move(windowTopLeftPoint + distance);
    }
    QPoint CurrentPos = event->pos();
    if (CurrentPos.y() < MenuRD.y()) {
        this->ui.menubar->setVisible(true);
    }
    else {
        this->ui.menubar->setVisible(false);
    }
    
}

void TimeTableQt::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bDrag = false;
    }
    setCursor(QCursor(Qt::ArrowCursor));
}

void TimeTableQt::UpdateWindow() {
    update();
}


void TimeTableQt::on_actionabout_triggered()
{
    About* dialog = new About(this);
    dialog->show();
}

void TimeTableQt::on_actionexit_triggered()
{
    close();
}

void TimeTableQt::on_actioninport_triggered()
{
    ImportFromFile* dialog = new ImportFromFile(this);
    dialog->show();
}

void TimeTableQt::on_actiontotop_triggered()
{
    Qt::WindowFlags flags;
    bool ischecked=this->ui.actiontotop->isChecked();
    if(ischecked){
        flags=windowFlags();
        setWindowFlags(flags|Qt::WindowStaysOnTopHint);
        show();
    }
    else{
        setWindowFlags(flags);
        show();
    }
}


void TimeTableQt::on_actionminimize_triggered()
{
     this->setWindowState(Qt::WindowMinimized);
}


void TimeTableQt::on_actionshowall_triggered()
{
    ShowAllLessons* dialog = new ShowAllLessons(this);
    dialog->show();
}


void TimeTableQt::on_actionmoreinfo_triggered()
{
    ShowMoreInfo* dialog = new ShowMoreInfo(this);
    dialog->show();
}

