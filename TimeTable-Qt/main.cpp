#include "include.h"
#include <QtWidgets/QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    {
        std::string dir{ argv[0] };
        auto dir2 = std::string{ dir.begin(),dir.begin() + dir.find_last_of("\\") };
        SetCurrentDirectoryA(dir2.c_str());
    }
    QApplication a(argc, argv);

    std::wstring path;
    Json::mGetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    MainWindow w{ value["MainWindow"] };
    w.show();
    return a.exec();
}


