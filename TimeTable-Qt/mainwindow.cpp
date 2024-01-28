#include "MainWindow.h"
#include "qsystemtrayicon.h"
#include "qmenu.h"

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
    for (auto& i : value["Windows"]) {
        //m_windows.push_back
    }
    Json::mGetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
    //m_hide = true;
}

MainWindow::~MainWindow()
{}

void MainWindow::CreateSystemTrayIcon()
{
    // 创建QSystemTrayIcon对象并设置图标
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(QIcon::fromTheme(":/TimeTableQt/123.ico"), this);

    // 创建菜单项
    QMenu* trayIconMenu = new QMenu();
    QAction* action_show = trayIconMenu->addAction("显示所有窗口");
    QAction* action_hide = trayIconMenu->addAction("隐藏所有窗口");
    QAction* action_exit = trayIconMenu->addAction("退出");

    // 将菜单项添加到托盘图标
    trayIcon->setContextMenu(trayIconMenu);

    // 连接菜单项的信号到槽函数
    QObject::connect(action_show, &QAction::triggered, []() {
        // 实现菜单项1的功能
        });
    QObject::connect(action_hide, &QAction::triggered, []() {
        // 实现菜单项2的功能
        });
    QObject::connect(action_exit, &QAction::triggered, []() {
        // 实现菜单项2的功能
        });

    // 显示托盘图标
    trayIcon->show();

}

Json::Value MainWindow::SaveJson(Json::Value & value) const
{
	return Json::Value();
}
