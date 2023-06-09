#include "TimeTableQt.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TimeTableQt w;
    w.show();
    return a.exec();
}
