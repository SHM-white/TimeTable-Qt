#include "TimeTableQt.h"
#include <QtWidgets/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    std::string dir{ argv[0] };
    auto dir2 = std::string{ dir.begin(),dir.begin() + dir.find_last_of("\\") };
    SetCurrentDirectoryA(dir2.c_str());
    QApplication a(argc, argv);
    TimeTableQt w;
    w.show();
    return a.exec();
}


