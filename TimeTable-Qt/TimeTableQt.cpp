#pragma once

#define EXPERENCE false
#define AUTO_RUN_KEY	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

#include "TimeTableQt.h"
#include "importfromfile.h"
#include "about.h"
#include "showalllessons.h"
#include "showmoreinfo.h"
#include "changeconfigpath.h"
#include "settings.h"
#include "todayalllessons.h"
#include "include.h"

#if EXPERENCE
#ifdef Q_OS_WIN
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
#include <gdiplus.h>
#include <GdiPlusColor.h>
#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea
#pragma comment (lib,"user32.lib")
#endif
#endif // EXPERENCE


TimeTableQt::TimeTableQt(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    
    time_calendar_text = new QTimer(this);
    time_calendar_window = new QTimer(this);
    connect(time_calendar_window, SIGNAL(timeout()), this, SLOT(updateWindow()));
    connect(time_calendar_text, SIGNAL(timeout()), this, SLOT(updateTexts()));
    time_calendar_window->start((int)100/6);
    time_calendar_text->start(500);
    setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    flags = windowFlags();
    setWindowFlags(flags | Qt::WindowStaysOnTopHint);
    std::wstring path;
    Json::mGetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    windowsettings = std::make_shared<WindowSettings>(path);
    Json::mGetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
    windowsettings->msLessonInfoFile = path;
    timetable = std::make_shared<TimeTable>(path);
    mInitializeWindow();
    {
        QString application_name = QApplication::applicationName();
        QSettings settings{ AUTO_RUN_KEY, QSettings::NativeFormat };
        bool autoBoot = settings.value(application_name, "").toBool();
        this->ui.actionBootAtPowerOn->setChecked(autoBoot);
    }
}

TimeTableQt::~TimeTableQt()
{
    delete time_calendar_window;
    delete time_calendar_text;
}

bool TimeTableQt::mInitializeWindow()
{
    

    this->setGeometry(windowsettings->miWindowX, windowsettings->miWindowY, windowsettings->miWindowWeight, windowsettings->miWindowHeight);
    this->setFixedSize(windowsettings->miWindowWeight, windowsettings->miWindowHeight);

    QString picpath = QString::fromStdWString(windowsettings->msBackGroundImg);
    pic = QPixmap(picpath);
    pic = pic.scaled(this->width(), this->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    
#if EXPERENCE
#ifdef Q_OS_WIN
    if (windowsettings->bAcrylicEffect) {

        HWND hMoudle = (HWND)(winId());
        HMODULE hDLL = LoadLibrary(TEXT("AcrylicMaterial"));

        using fun = void (*)(HWND hWnd);
        fun pSetBlur = (fun)GetProcAddress(hDLL, "setAcrylicEffect");
        pSetBlur((HWND)(winId()));
    }
#endif // Q_OS_WIN

#endif // EXPERENCE



    this->ui.menubar->setVisible(false);
    MenuRD.setX(this->ui.menubar->width());
    MenuRD.setY(this->ui.menubar->height());
    this->setMouseTracking(true);
    //Sleep(500);//等待dll加载完成后显示窗口
    show();
    return true;
}

void TimeTableQt::updateTexts()
{
    if (items.size() != windowsettings->msTextFormat.size()) {
        items.clear();
        for (auto& i : windowsettings->msTextFormat) {
            WindowItem item;
            item.color = QColor(GetRValue(i.color), GetGValue(i.color), GetBValue(i.color));
            item.font.setFamily(QString::fromStdWString(i.msFontName));
            item.font.setPointSize(i.miTextSize);
            item.position = QPoint(i.mpTextLocation.x, i.mpTextLocation.y);
            item.size = QSize(i.miSizeW, i.miSizeH);
            items.push_back(item);
        }
    }
    for (auto i = 0; i < items.size();++i) {
        if (items[i].m_needUpdate) {
            items[i].m_needUpdate = false;
            windowsettings->msTextFormat[i].update();
            items[i].m_updated = true;
        }
        auto& text = items[i].Text();
        if(windowsettings->msTextFormat.empty()||windowsettings->msTextFormat[i].Texts.empty()||windowsettings->msTextFormat[i].updateCounter>=windowsettings->msTextFormat[i].Texts.size()) {
            items[i].Text() = "";
            return;
        }
        auto& currentText = windowsettings->msTextFormat[i].Texts[windowsettings->msTextFormat[i].updateCounter];
        auto& translatedText = currentText.text;

        switch (currentText.type)
        {
        case TextType::CurrentTime:
            text = QString::fromStdWString(timetable->mGetCurrentTime(translatedText));
            break;
        case TextType::CurrentLesson:
            text = QString::fromStdWString(timetable->mGetCurrentTime(translatedText)) +
                QString::fromStdWString(timetable->mGetCurrentLesson(windowsettings->msLessonNull));
            break;
        case TextType::CountDownDay:
            text = QString::fromStdWString(timetable->mGetCountDown(windowsettings->mCountDownDay, translatedText));
            break;
        case TextType::Info:
            text = QString::fromStdWString(timetable->mGetCurrentTime(translatedText)) +
                QString::fromStdWString(timetable->mGetInfo());
            break;
        case TextType::Weather:
            static auto lastUpdate = QDateTime::currentSecsSinceEpoch();
            if (QDateTime::currentSecsSinceEpoch() - lastUpdate > 3600) {
                timetable->mUpdateWeather();
                lastUpdate = QDateTime::currentSecsSinceEpoch();
            }
            text = QString::fromStdWString(timetable->mGetWeather(_wtoi(translatedText.c_str())));
            break;
        default:
            break;
        }

    }
}

void TimeTableQt::updateWindow()
{
    if (timeCounter > 300) {
        this->ui.menubar->setVisible(true);
        timeCounter = 300;
    }
    else if (timeCounter < 1) {
        this->ui.menubar->setVisible(false);
    }
    else {
        timeCounter -= 1;
    }
    update();
}

void TimeTableQt::ShowShadow()
{

#if EXPERENCE

#ifdef Q_OS_WIN
    BOOL bEnable = true;
    ::DwmIsCompositionEnabled(&bEnable);
    if (bEnable)
    {
        DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
        ::DwmSetWindowAttribute((HWND)winId(), DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
        MARGINS margins = { -1 };
        ::DwmExtendFrameIntoClientArea((HWND)winId(), &margins);
    }

#endif // Q_OS_WIN


#endif // EXPERENCE

}

void TimeTableQt::closeEvent(QCloseEvent* event)
{
    if (this->ui.actionshowTodayAll->isChecked()) {
        this->ui.actionshowTodayAll->setChecked(false);
    }
    on_actionshowTodayAll_triggered();
    event->accept();
}

/*
TODO:
调用std::vector<WindowItem> items中每一项的paint函数
*/
void TimeTableQt::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    if (SHOW_ITEM_RECT)
    {
        painter.drawRect(rect());
    }

    
    // Draw background image
    if (pic.isNull() || !(windowsettings->mUseImgAsBackGround)
#if EXPERENCE
    || (windowsettings->bAcrylicEffect)
#endif
    ) {
        painter.fillRect(rect(), QColor(windowsettings->miBackGroundColor[0], windowsettings->miBackGroundColor[1], windowsettings->miBackGroundColor[2], windowsettings->miBackGroundColor[3]));
    } else {
        painter.fillRect(rect(), QColor(255, 255, 255, 1));
        painter.drawPixmap(0, 0, pic);
    }
    for(auto& i : items) {
        i.paint(painter);
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
    if (timeCounter < 1) {
        timeCounter = 301;
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
    bool ischecked=this->ui.actiontotop->isChecked();
    if(ischecked){
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


void TimeTableQt::on_actionChangeConfig_triggered()
{
    ChangeConfigPath* dialog = new ChangeConfigPath(this);
    dialog->show();
}

void TimeTableQt::on_actionSettings_triggered()
{
    Settings* dialog= new Settings(this);
    dialog->show();
}



void TimeTableQt::on_actionBootAtPowerOn_triggered()
{
    QString application_name = QApplication::applicationName();
    
    std::unique_ptr<QSettings> settings = std::make_unique<QSettings>(AUTO_RUN_KEY, QSettings::NativeFormat);
    //QMessageBox::information(this, "开发中", "之前的有bug，待修复", QMessageBox::Ok);
    if (this->ui.actionBootAtPowerOn->isChecked()) {
        QString application_path = QApplication::applicationFilePath();
        settings->setValue(application_name, application_path.replace("/", "\\"));
    }
    else
    {
        settings->remove(application_name);		
    }
    //LPITEMIDLIST lp;
    //SHGetSpecialFolderLocation(0, CSIDL_STARTUP, &lp);

}


void TimeTableQt::on_actionshowTodayAll_triggered()
{
    static TodayAllLessons* allLessonWindow;
    if (allLessonWindow == nullptr) {
        allLessonWindow = new TodayAllLessons(this);
    }
    if (this->ui.actionshowTodayAll->isChecked()) {
        allLessonWindow->show();
    }
    else
    {
        allLessonWindow->close();
    }
}

