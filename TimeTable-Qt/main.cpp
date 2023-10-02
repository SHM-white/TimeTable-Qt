#include "TimeTableQt.h"
#include <QtWidgets/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    {
        char szBuf[512] = { 0 };
        getcwd(szBuf, sizeof(szBuf)-1);
        SetCurrentDirectoryA(szBuf);
    }
    QApplication a(argc, argv);
    TimeTableQt w;
    w.show();
    return a.exec();
}


