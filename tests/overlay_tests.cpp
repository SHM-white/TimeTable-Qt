#include "DesktopOverlays.h"
#include "DesktopLayoutStore.h"
#include "MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QEvent>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QFontMetrics>
#include <QImage>
#include <QJsonDocument>
#include <QLabel>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTimer>

using namespace tt;

namespace {
int failures=0;

void check(bool condition,const char* name) {
    if(!condition) { qCritical()<<"FAILED:"<<name; ++failures; }
}

void runEvents(int milliseconds) {
    QEventLoop loop;
    QTimer::singleShot(milliseconds,&loop,&QEventLoop::quit);
    loop.exec();
}

class PaintCounter final:public QObject {
public:
    int count=0;
protected:
    bool eventFilter(QObject* watched,QEvent* event) override {
        if(event->type()==QEvent::Paint) ++count;
        return QObject::eventFilter(watched,event);
    }
};

QJsonObject fixture(const QString& id,const QString& text,bool marquee,
                    const QString& type=QStringLiteral("elements")) {
    TextElement item;
    item.type="note";
    item.format=text;
    item.font="sans-serif";
    item.pointSize=15;
    item.color=Qt::black;
    ElementDefinition element;
    element.rect=QRect(12,14,150,40);
    element.marquee=marquee;
    element.items<<item;
    return {{"id",id},{"title",id},{"type",type},{"theme","light"},
        {"elements",ElementLayout::toJson({element})},
        {"collapsed",false},{"visible",true},{"topmost",false},
        {"locked",true},{"opacity",100},{"backgroundColor","#FFFFFFFF"},
        {"fontSize",13},{"x",20},{"y",20},{"width",300},{"height",130},
        {"expandedX",20},{"expandedY",20},
        {"expandedWidth",300},{"expandedHeight",130}};
}

QWidget* card(const QString& title) {
    for(auto* widget:QApplication::topLevelWidgets())
        if(widget->objectName()=="desktopOverlay" && widget->windowTitle()==title)
            return widget;
    return nullptr;
}

bool hasActiveAnimation(QWidget* widget) {
    if(!widget) return false;
    auto* timer=widget->findChild<QTimer*>("elementAnimationTimer");
    return timer && timer->isActive();
}
bool displaysText(QWidget* widget,const QString& text) {
    if(!widget)return false;
    for(const auto* label:widget->findChildren<QLabel*>())
        if(label->text()==text)return true;
    return false;
}
}

int main(int argc,char** argv) {
    QTemporaryDir temporary;
    if(!temporary.isValid()) {
        qCritical()<<"Unable to create isolated desktop test directory";
        return 1;
    }
    const QString settingsRoot=temporary.path()+"/config";
    const QString dataRoot=temporary.path()+"/data";
    QDir().mkpath(settingsRoot);
    QDir().mkpath(dataRoot);
    qputenv("XDG_CONFIG_HOME",settingsRoot.toUtf8());
    qputenv("XDG_DATA_HOME",dataRoot.toUtf8());
    qputenv("APPDATA",dataRoot.toUtf8());
    qputenv("LOCALAPPDATA",dataRoot.toUtf8());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsRoot);
    QStandardPaths::setTestModeEnabled(true);
    QApplication app(argc,argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName("TimeTableQtTests");
    const QString applicationName="overlay-tests-"+QFileInfo(temporary.path()).fileName();
    QCoreApplication::setApplicationName(applicationName);
    const QString appDataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString longText="A smooth desktop marquee moves this deliberately long fixed sentence "
        "across a narrow card, without waiting for the one-second schedule refresh.";
    auto cyclingFixture=fixture("cycling","Short",true);
    auto cycling=ElementLayout::fromJson(cyclingFixture.value("elements").toArray()).first();
    cycling.mode="cycle";
    cycling.intervalSeconds=1;
    TextElement longCycleItem=cycling.items.first();
    longCycleItem.format=longText;
    cycling.items<<longCycleItem;
    cyclingFixture.insert("elements",ElementLayout::toJson({cycling}));
    auto resizingFixture=fixture("resizing",
        "Resize this desktop card until the whole fixed sentence fits.",true);
    auto resizing=ElementLayout::fromJson(resizingFixture.value("elements").toArray()).first();
    const auto resizingItem=resizing.items.first();
    const int resizingTextWidth=QFontMetrics(QFont(resizingItem.font,resizingItem.pointSize))
        .horizontalAdvance(resizingItem.format);
    resizing.rect.setWidth(resizingTextWidth+20);
    resizingFixture.insert("elements",ElementLayout::toJson({resizing}));
    auto zeroFPS=fixture("zero-fps",longText,true);
    zeroFPS.insert("fps",0);
    auto slowFPS=fixture("slow-fps",longText,true);
    slowFPS.insert("fps",20);
    auto hiddenAtStartup=fixture("startup-hidden",longText,false,"clock");
    hiddenAtStartup.insert("visible",false);
    auto autoSized=fixture("auto-sized","Short",false);
    autoSized.insert("autoSize",true);autoSized.insert("elementInset",0);
    auto combinedDuty=fixture("combined-duty","",false,"combined");
    combinedDuty.insert("combinedItems","duty");
    const QJsonArray windows{
        fixture("scrolling",longText,true),
        fixture("short","Short",true),
        fixture("disabled",longText,false),
        fixture("ordinary",longText,true,"clock"),
        cyclingFixture,resizingFixture,zeroFPS,slowFPS,hiddenAtStartup,combinedDuty,autoSized};
    QSettings().setValue("desktop/windows",QJsonDocument(windows).toJson(QJsonDocument::Compact));

    {
        MainWindow planner;
        DesktopOverlays overlays(&planner);
        QElapsedTimer started;
        started.start();
        overlays.start();
        // Let initial exposure and the 190 ms window fade finish before measuring.
        runEvents(250);
        auto* scrolling=card("scrolling");
        check(scrolling && scrolling->isVisible(),"real desktop element card is visible");
        check(hasActiveAnimation(scrolling),"overflowing text starts a dedicated animation timer");
        check(card("short") && !hasActiveAnimation(card("short")),
            "short element text has no active animation timer");
        check(card("disabled") && !hasActiveAnimation(card("disabled")),
            "disabling marquee stops frame updates");
        check(card("ordinary") && !hasActiveAnimation(card("ordinary")),
            "ordinary desktop card has no active element animation timer");
        check(card("zero-fps") && !hasActiveAnimation(card("zero-fps")),
            "FPS zero disables continuous repaint for overflowing text");
        auto* slowTimer=card("slow-fps")?card("slow-fps")->findChild<QTimer*>("elementAnimationTimer"):nullptr;
        check(slowTimer&&slowTimer->isActive()&&slowTimer->interval()==50,
            "card frame rate independently controls animation timer interval");
        check(card("startup-hidden")&&!card("startup-hidden")->isVisible(),
            "startup-hidden card remains hidden on initial launch");
        auto* cyclingCard=card("cycling");
        check(cyclingCard && !hasActiveAnimation(cyclingCard),
            "short cycle item initially keeps frame animation stopped");
        auto* cycleTimer=cyclingCard?cyclingCard->findChild<QTimer*>("elementCycleTimer"):nullptr;
        check(cycleTimer && cycleTimer->isActive() && cycleTimer->isSingleShot() &&
            cycleTimer->timerType()==Qt::PreciseTimer,
            "short cycle item schedules its precise next boundary");

        if(auto* resizingCard=card("resizing")) {
            check(hasActiveAnimation(resizingCard),"clipped element starts scrolling before resize");
            resizingCard->resize(resizing.rect.right()+25,130);
            QApplication::processEvents();
            check(!hasActiveAnimation(resizingCard),"growing desktop card removes text overflow");
            resizingCard->resize(300,130);
            QApplication::processEvents();
            check(hasActiveAnimation(resizingCard),"shrinking desktop card restores text scrolling");
        } else check(false,"resizable desktop element card exists");

        if(scrolling) {
            auto* timer=scrolling->findChild<QTimer*>("elementAnimationTimer");
            check(timer && timer->timerType()==Qt::PreciseTimer,
                "desktop animation uses a precise timer");
            PaintCounter counter;
            scrolling->installEventFilter(&counter);
            const QRect textArea(17,18,150,40);
            const QImage before=scrolling->grab(textArea).toImage();
            counter.count=0;
            runEvents(350);
            const int animatedPaints=counter.count;
            const QImage after=scrolling->grab(textArea).toImage();
            check(animatedPaints>=8,"desktop marquee repaints repeatedly between schedule ticks");
            check(!before.isNull() && !after.isNull() && before!=after,
                "fixed long text visibly moves on the real desktop card");

            scrolling->hide();
            runEvents(30);
            check(!hasActiveAnimation(scrolling),"hiding desktop card stops animation timer");
            counter.count=0;
            runEvents(120);
            check(counter.count==0,"hidden desktop card receives no periodic paints");

            scrolling->show();
            runEvents(30);
            check(hasActiveAnimation(scrolling),"showing desktop card resumes animation timer");
            counter.count=0;
            runEvents(200);
            check(counter.count>=8,"restored desktop card resumes smooth frame updates");
            scrolling->removeEventFilter(&counter);
        }
        // A normal one-second model refresh must retain the static-window decision.
        overlays.refresh();
        check(card("short") && !hasActiveAnimation(card("short")),
            "model refresh keeps short text animation disabled");

        // Check actual timer transitions, not just the renderer's cycle selection.
        // Wait relative to card creation to check both one-second boundaries.
        if(started.elapsed()<1200) runEvents(int(1200-started.elapsed()));
        check(hasActiveAnimation(cyclingCard),"one-second cycle boundary starts long-text animation");
        if(started.elapsed()<2200) runEvents(int(2200-started.elapsed()));
        check(cyclingCard && !hasActiveAnimation(cyclingCard),
            "two-second cycle boundary stops animation for the short item");
        if(cyclingCard) {
            cyclingCard->hide();
            QApplication::processEvents();
            check(cycleTimer && !cycleTimer->isActive(),
                "hiding desktop card cancels its pending cycle boundary");
            check(!hasActiveAnimation(cyclingCard),
                "hiding cyclic desktop card keeps frame animation stopped");
        }
        // Rebuilding resets card clocks while the manager's data timer keeps its
        // old phase. The new short item must wake at its own next boundary.
        overlays.toggleCollapsed("cycling");
        runEvents(1200);
        check(hasActiveAnimation(card("cycling")),
            "rebuilt cyclic card starts scrolling independently of the data timer phase");
        const auto preferences=overlays.configurations();
        overlays.hideAll();
        QApplication::processEvents();
        check(!card("scrolling")->isVisible()&&!hasActiveAnimation(card("scrolling")),
            "temporary hide stops both visibility and animation");
        bool unchanged=preferences.size()==overlays.configurations().size();
        for(int i=0;i<preferences.size()&&unchanged;++i)
            unchanged=preferences[i].visible==overlays.configurations()[i].visible;
        check(unchanged,"temporary hide preserves every startup preference");
        overlays.showAll();QApplication::processEvents();
        check(card("startup-hidden")->isVisible(),"temporary show-all can reveal startup-hidden windows");
        unchanged=true;
        for(int i=0;i<preferences.size()&&unchanged;++i)
            unchanged=preferences[i].visible==overlays.configurations()[i].visible;
        check(unchanged,"temporary show-all preserves every startup preference");
        overlays.setStartupVisible("ordinary",false);
        check(!overlays.configurations()[3].visible,"startup visibility can be explicitly configured");
        const QString oldPath=overlays.configurationPath();
        const QString malformed=temporary.filePath("invalid-layout.json");
        QFile invalid(malformed);invalid.open(QIODevice::WriteOnly);invalid.write("{ broken");invalid.close();
        QString error;
        check(!overlays.setConfigurationPath(malformed,&error)&&overlays.configurationPath()==oldPath,
            "invalid custom configuration cannot replace active path");
        const int previousCount=overlays.configurations().size();
        check(!overlays.restoreLayout(malformed,&error)&&overlays.configurations().size()==previousCount,
            "invalid restore leaves active windows unchanged");
        const QString custom=temporary.filePath("nested/desktop.json");
        check(overlays.setConfigurationPath(custom,&error)&&overlays.configurationPath()==custom&&QFileInfo::exists(custom),
            "new custom configuration path atomically copies the current layout");
        QVector<OverlayConfig> persisted;
        check(DesktopLayoutStore::read(custom,&persisted,&error)&&persisted.size()==overlays.configurations().size(),
            "custom configuration persists the complete window list");
        runEvents(30);
        if(auto* autoCard=card("auto-sized")) {
            const auto bounds=ElementLayout::contentBounds(overlays.configurations().last().elements,
                planner.schedule(),QDateTime::currentDateTime());
            check(autoCard->size()==QSize(bounds.right()+1,bounds.bottom()+1),
                "automatic desktop size follows shared dynamic content bounds");
            autoCard->resize(400,200);overlays.refresh();
            check(autoCard->size()==QSize(bounds.right()+1,bounds.bottom()+1),
                "model refresh reapplies automatic element card sizing");
        } else check(false,"auto-sized element card exists");
        const int weekday=QDate::currentDate().dayOfWeek();
        const QString previousDuty=QStringLiteral("Duty from previous semester");
        QSettings().setValue("desktop/duty/"+QString::number(weekday),previousDuty);
        planner.setDayInfos(weekday,{QStringLiteral("Imported duty A"),QStringLiteral("Imported duty B")});
        auto* combinedCard=card("combined-duty");
        check(displaysText(combinedCard,QStringLiteral("Imported duty A · Imported duty B"))&&
            !displaysText(combinedCard,previousDuty),
            "combined duty displays live imported schedule information instead of stale settings");
        planner.setDayInfos(weekday,{QStringLiteral("New semester duty")});
        check(displaysText(combinedCard,QStringLiteral("New semester duty"))&&
            !displaysText(combinedCard,QStringLiteral("Imported duty A · Imported duty B")),
            "combined duty updates when active schedule information changes");
    }
    {
        MainWindow planner;
        DesktopOverlays overlays(&planner);overlays.start();runEvents(30);
        check(card("ordinary")&&!card("ordinary")->isVisible(),
            "restart restores explicitly hidden startup preference");
        check(card("startup-hidden")&&!card("startup-hidden")->isVisible(),
            "temporary show-all is not persisted across restart");
        QFile stored(overlays.configurationPath());stored.open(QIODevice::ReadOnly);
        const auto before=stored.readAll();stored.close();
        overlays.hideAll();overlays.showAll();
        stored.open(QIODevice::ReadOnly);
        check(stored.readAll()==before,"temporary visibility operations leave layout file untouched");
    }
    // QStandardPaths may use a platform test directory instead of XDG_DATA_HOME.
    // Only remove the randomly named application directory created by this test.
    if(QFileInfo(appDataPath).fileName()==applicationName)
        QDir(appDataPath).removeRecursively();
    if(!failures) qInfo()<<"Desktop overlay animation tests passed";
    return failures?1:0;
}
