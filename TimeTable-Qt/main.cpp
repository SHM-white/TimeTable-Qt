#include "TimeTableQt.h"
#include <QtWidgets/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    {
        std::string fullPath = argv[0];
        int find = fullPath.find_last_of('\\');
        std::string path{fullPath.begin(), fullPath.begin() + find + 1};
        SetCurrentDirectory(path.c_str());
    }
    QApplication a(argc, argv);
    TimeTableQt w;
    w.show();
    return a.exec();
}


