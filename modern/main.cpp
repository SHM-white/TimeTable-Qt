#include "MainWindow.h"
#include "DesktopOverlays.h"
#include <QStyleFactory>
#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QIcon>

int main(int argc, char *argv[]) {
    QApplication app(argc,argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    QCoreApplication::setOrganizationName("TimeTableQt");
    QCoreApplication::setApplicationName("TimeTableNext");
    QCoreApplication::setApplicationVersion("4.0.0");
    QApplication::setQuitOnLastWindowClosed(false);
    QFont font;
    font.setFamilies({"Microsoft YaHei UI","Noto Sans CJK SC","Segoe UI","sans-serif"});
    font.setPointSize(10);
    app.setFont(font);
    tt::MainWindow window;
    tt::DesktopOverlays overlays(&window);
    window.setOpenOverlaySettings([&overlays]{overlays.openConfiguration();});
    overlays.start();
    return app.exec();
}
