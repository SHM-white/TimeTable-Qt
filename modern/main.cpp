#include "MainWindow.h"
#include "DesktopOverlays.h"
#include <QStyleFactory>
#include <QSettings>
#include <QTimer>
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
    const QStringList arguments=app.arguments();
    // Headless CI regression path to catch UI startup failures on both themes.
    if(arguments.contains("--smoke-test-dark")) QSettings().setValue("dark",true);
    if(arguments.contains("--smoke-test-light")) QSettings().setValue("dark",false);
    tt::MainWindow window;
    tt::DesktopOverlays overlays(&window);
    window.setOpenOverlaySettings([&overlays]{overlays.openConfiguration();});
    overlays.start();
    if(arguments.contains("--smoke-test-dark") || arguments.contains("--smoke-test-light"))
        QTimer::singleShot(500,&app,&QCoreApplication::quit);
    return app.exec();
}
