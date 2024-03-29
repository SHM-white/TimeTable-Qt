﻿#include "MainWindow.h"
#include "Settings_New.h"

MainWindow::MainWindow(Json::Value& settings, QWidget* parent)
	: BasicWindow(settings, parent)
{
	ui.setupUi(this);
    m_AutoOpen = true;
    m_hide = false;
    ui.label_version->setText(QString::fromStdWString(GetCurrentVersion()));
    this->show();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SelfInitial()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
}

void MainWindow::ReopenWindows()
{
    m_windows.clear();
    OpenSubWindows();
}

void MainWindow::CreateSystemTrayIcon()
{
    // 创建QSystemTrayIcon对象并设置图标
    trayIcon = new QSystemTrayIcon(QIcon::fromTheme(":/TimeTableQt/123.ico"), this);

    // 创建菜单项
    QMenu* trayIconMenu = new QMenu();
    QAction* action_show = trayIconMenu->addAction(QString::fromStdWString(L"显示所有窗口"));
    QAction* action_hide = trayIconMenu->addAction(QString::fromStdWString(L"隐藏所有窗口"));
    action_autoLaunch = trayIconMenu->addAction(QString::fromStdWString(L"开机自启动"));
    action_autoLaunch->setCheckable(true);
    QAction* action_settings = trayIconMenu->addAction(QString::fromStdWString(L"设置"));
    QAction* action_exit = trayIconMenu->addAction(QString::fromStdWString(L"退出"));

    // 将菜单项添加到托盘图标
    trayIcon->setContextMenu(trayIconMenu);

    // 连接菜单项的信号到槽函数
    QObject::connect(action_show,       SIGNAL(triggered()), this, SLOT(ShowAllWindows()));
    QObject::connect(action_hide,       SIGNAL(triggered()), this, SLOT(HideAllWindows()));
    QObject::connect(action_autoLaunch, SIGNAL(triggered()), this, SLOT(LaunchAsSystemBoot()));
    QObject::connect(action_settings,   SIGNAL(triggered()), this, SLOT(OpenSetting()));
    QObject::connect(action_exit,       SIGNAL(triggered()), this, SLOT(Exit()));
    {
        QString application_name = QApplication::applicationName();
        QSettings settings{ AUTO_RUN_KEY, QSettings::NativeFormat };
        bool autoBoot = settings.value(application_name, "").toBool();
        action_autoLaunch->setChecked(autoBoot);
    }

    // 显示托盘图标
    trayIcon->show();

}

std::shared_ptr<BasicWindow> MainWindow::CreateSubWindows(Json::Value& settings, std::shared_ptr<TimeTable> timetable)
{
    return std::make_shared<SubWindow>(settings, timetable);
}

Json::Value MainWindow::SaveAsJson(Json::Value value) const
{
	return value;
}

void MainWindow::HideAllWindows()
{
    for (auto& i : m_windows) {
        i->m_hide = true;
    }
}

void MainWindow::updateWindowStatus()
{
    //static int counter{ 0 };
    //static const int limit{ 3 };
    //if (counter <= limit) {
    //    counter++;
    //}
    //else if (counter == limit + 1)
    //{
    //    m_hide = true;
    //}
    BasicWindow::updateWindowStatus();
}

void MainWindow::ShowAllWindows()
{
    for (auto& i : m_windows) {
        i->m_hide = false;
    }
}

void MainWindow::LaunchAsSystemBoot()
{
    QString application_name = QApplication::applicationName();
    std::unique_ptr<QSettings> settings = std::make_unique<QSettings>(AUTO_RUN_KEY, QSettings::NativeFormat);
    if (action_autoLaunch->isChecked()) {
        QString application_path = QApplication::applicationFilePath();
        settings->setValue(application_name, application_path.replace("/", "\\"));
    }
    else
    {
        settings->remove(application_name);
    }

}

void MainWindow::OpenSetting()
{
#if STRANGE_TEST
    while (true)
    {
#endif // DEBUG
        Json::Value value;
        value["Moveable"] = true;
        Settings_New* setting = new Settings_New{ &m_windows,value,this };
        setting->show();
#if STRANGE_TEST
    }
#endif // DEBUG

}

void MainWindow::OpenSubWindows()
{
    m_windows.clear();
    std::wstring path;
    Json::GetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    ITEM_SCROLL_SPEED = std::max(value["ItemScrollSpeed"].asInt(),1);
    for (auto& i : value["Windows"]) {
        try
        {
            m_windows.push_back(CreateSubWindows(i, m_TimeTable));
        }
        catch (const std::exception& e)
        {
            QMessageBox::warning(this, QString::fromStdWString(L"error"), QString::fromStdWString(std::format(L"error:{},请检查配置文件", u8tw(std::string(e.what())))), QMessageBox::Ok);
        }
    }

}

void MainWindow::Exit()
{
    for (auto& i : m_windows) {
        i->m_hide = true;
        i->close();
    }
    delete trayIcon;
    this->close();
    qApp->quit();
}

void MainWindow::SelfInitial()
{
    CreateSystemTrayIcon();
    std::wstring path;
    Json::GetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    if (value["Windows"].isNull()) {
        QMessageBox::warning(this, QString::fromStdWString(L"error"), QString::fromStdWString(L"未找到窗口设置，请检查配置文件设置"), QMessageBox::Ok);
    }
    else
    {
        m_successfulInitialized = true;
    }
    bool lowVersion{ false };
    for (int i = 0; i < 3; i++) {
        lowVersion |= value["version"][i].asInt() < minVersion_global[i];
    }
    if (lowVersion) {
        auto choose = QMessageBox::warning(this, QString::fromStdWString(L"error"), QString::fromStdWString(L"配置文件版本过低，是否继续加载？"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        switch (choose)
        {
        case QMessageBox::Yes:
            lowVersion = false;
            break;
        case QMessageBox::No:
        default:
            lowVersion = true;
            break;
        }
    }
    if (!lowVersion)
    {
        OpenSubWindows();
#ifdef DEBUG
        OpenSetting();
#endif // DEBUG
    }
    else
    {
        qApp->quit();
    }
    m_hide = true;
    delete timer;
}
