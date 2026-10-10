#include "MainWindow.h"
#include "DesktopOverlays.h"
#include <QStyleFactory>
#include <QSettings>
#include <QTimer>
#include <QStackedWidget>
#include <QGraphicsEffect>
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    font.setFamilies({"Microsoft YaHei UI","Noto Sans CJK SC","Segoe UI","sans-serif"});
#else
    font.setFamily("Microsoft YaHei UI");
#endif
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
    if(arguments.contains("--smoke-navigation")) {
        QTimer::singleShot(0,&app,[&window] {
            auto* stack=window.findChild<QStackedWidget*>();
            if(!stack || stack->count()!=5) {
                QCoreApplication::exit(11);return;
            }
            for(int page=0;page<stack->count();++page) {
                window.openPage(page);
                QCoreApplication::processEvents();
                if(stack->currentIndex()!=page ||
                   stack->currentWidget()->graphicsEffect()!=nullptr) {
                    QCoreApplication::exit(12);return;
                }
            }
            QCoreApplication::quit();
        });
    } else if(arguments.contains("--smoke-test-dark") || arguments.contains("--smoke-test-light"))
        QTimer::singleShot(500,&app,&QCoreApplication::quit);
    return app.exec();
}
