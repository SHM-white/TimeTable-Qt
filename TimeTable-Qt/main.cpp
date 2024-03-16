#include "include.h"
#include <QtWidgets/QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
#ifdef QT5
    //QApplication::setAttribute(Qt::AA_Use96Dpi);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    //QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
    //QApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    QApplication::setFont(QFont("Microsoft YaHei", 9));

#endif

    {
        std::string dir{ argv[0] };
        auto dir2 = std::string{ dir.begin(),dir.begin() + dir.find_last_of("\\") };
        SetCurrentDirectoryA(dir2.c_str());
    }
    QApplication a(argc, argv);
#ifdef QT5
    a.setQuitOnLastWindowClosed(false);
#endif // QT5
    std::wstring path;
    Json::GetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    MainWindow w{ value["MainWindow"] };
    w.show();
    return a.exec();
}


