#include "MainWindow.h"
#include "Settings_New.h"

MainWindow::MainWindow(Json::Value& settings, QWidget* parent)
	: BasicWindow(settings, parent)
{
	ui.setupUi(this);
    m_AutoOpen = true;
    m_hide = false;
    ui.label_version->setText(QString::fromStdString(std::format("v{}.{}.{}", currentVersion_global[0], currentVersion_global[1], currentVersion_global[2])));
    this->show();
    CreateSystemTrayIcon();
    std::wstring path;
    Json::mGetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    Json::mGetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
    m_TimeTable = std::make_shared<TimeTable>(path);
    if (value["Windows"].isNull()) {
        QMessageBox::warning(this, QString::fromStdWString(L"error"), QString::fromStdWString(L"未找到窗口设置，请检查配置文件设置"), QMessageBox::Ok);
    }
    else
    {
        m_successfulInitialized = true;
    }
    for (auto& i : value["Windows"]) {
        m_windows.push_back(CreateSubWindows(i,m_TimeTable));
    }

#ifdef DEBUG
    OpenSetting();
#endif // DEBUG
}

MainWindow::~MainWindow()
{
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

Json::Value MainWindow::SaveAsJson(Json::Value & value) const
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
    static int counter{ 0 };
    static const int limit{ 3 };
    if (counter <= limit) {
        counter++;
    }
    else if (counter == limit + 1)
    {
        m_hide = true;
    }
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
    Json::Value value;
    value["Moveable"] = true;
    Settings_New* setting = new Settings_New{ Json::GetRootJsonValue(Json::mGetTextItem(L"configPath",DEFAULT_CONFIG_PATH,0)),value,this };
    setting->show();
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
