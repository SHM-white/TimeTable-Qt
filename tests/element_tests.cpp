#include "ElementLayout.h"
#include <QApplication>
#include <QImage>
#include <QJsonDocument>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QFontMetrics>
#include <QTemporaryDir>
using namespace tt;
static int failures=0;
void check(bool condition,const char* name) {
    if(!condition){qCritical()<<"FAILED:"<<name;++failures;}
}
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    QJsonArray items{
        QJsonObject{{"ElementType",1},{"Location",QJsonArray{22,9}},
            {"Size",QJsonArray{230,33}},
            {"Data",QJsonArray{
                QJsonObject{{"Type",1},{"TextFormat","%H:%M:%S"},{"Font","SimSun"},
                    {"TextColor",QJsonArray{128,64,32,200}},{"TextSize",18}},
                QJsonObject{{"Type",5},{"TextFormat","距离考试 %03d天 %02d小时"},
                    {"TextSize",12},{"Data",QJsonObject{{"TargetTime",QJsonArray{2026,12,21,8,0,0}}}}}
            }}}};
    int skipped=0;
    const auto restored=ElementLayout::fromLegacy(items,&skipped);
    check(restored.size()==1&&skipped==0,"legacy element restored");
    check(restored.first().mode=="cycle","legacy cycle preserved");
    check(restored.first().rect==QRect(22,9,230,33),"legacy precise rectangle");
    check(restored.first().items.size()==2,"legacy cycling options");
    check(restored.first().items.first().font=="SimSun","legacy font");
    check(restored.first().items.first().color==QColor(128,64,32,200),"legacy RGBA");
    check(restored.first().items.first().pointSize==18,"legacy point size");
    check(restored.first().items.last().target.date()==QDate(2026,12,21),"legacy countdown date");
    const auto loaded=ElementLayout::fromJson(ElementLayout::toJson(restored));
    check(loaded.size()==1&&loaded.first().items.size()==2,"layout JSON roundtrip");
    check(loaded.first().items.first().color.alpha()==200,"alpha preserved");
    check(loaded.first().items.last().target.isValid(),"target preserved");
    const QDateTime now(QDate(2026,12,20),QTime(6,0));
    const QDateTime target(QDate(2026,12,21),QTime(8,0));
    check(ElementLayout::countdown(target,now,"距离考试%03d天%02d小时")==
          QStringLiteral("距离考试001天02小时"),"countdown placeholders are safe");
    check(ElementLayout::countdown(target,now,"%s %n %03d")==
          QStringLiteral("%s %n 001"),"unsupported printf controls are printed literally");
    ScheduleStore store;
    store.semesterMonday=QDate(2026,12,14);
    store.totalWeeks=4;
    store.courses.clear();
    Course course;course.name="Algorithms";course.weekday=7;
    course.start=QTime(6,0);course.end=QTime(7,0);course.weeks={1};
    store.courses<<course;
    TextElement text;text.type="current";text.format="正在：{value}";
    check(ElementLayout::evaluate(text,store,now)==QStringLiteral("正在：Algorithms"),
          "explicit lesson template");
    TextElement shortText;shortText.type="note";shortText.format="Short";
    TextElement longText=shortText;
    longText.format="This deliberately long sentence must scroll inside a narrow element.";
    ElementDefinition cycling;
    cycling.rect=QRect(0,0,120,30);
    cycling.mode="cycle";cycling.intervalSeconds=1;
    cycling.items={shortText,longText};
    const QRect available(0,0,300,120);
    check(!ElementLayout::hasScrollingText(available,{cycling},store,now,0),
          "short visible cycle item does not animate");
    check(ElementLayout::hasScrollingText(available,{cycling},store,now,1000),
          "overflowing visible cycle item animates");
    cycling.mode="all";
    check(ElementLayout::hasScrollingText(available,{cycling},store,now,0),
          "simultaneous items animate when one overflows");
    cycling.marquee=false;
    check(!ElementLayout::hasScrollingText(available,{cycling},store,now,0),
          "disabled marquee remains static");
    cycling.marquee=true;cycling.rect.moveLeft(400);
    check(!ElementLayout::hasScrollingText(available,{cycling},store,now,0),
          "off-canvas elements do not animate");
    QImage canvas(300,120,QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    ElementLayout::paint(painter,QRect(0,0,300,120),restored,store,now,2200,false);
    painter.end();
    check(!canvas.isNull(),"offscreen painter");
    QJsonArray broken{QJsonObject{{"ElementType",0},
         {"Data",QJsonArray{QJsonObject{{"Type",99}}}}}};
    skipped=0;
    check(ElementLayout::fromLegacy(broken,&skipped).isEmpty()&&skipped==1,
          "unknown legacy item transparently skipped");
    const QJsonArray weatherLegacy{QJsonObject{{"ElementType",0},{"Data",QJsonArray{
        QJsonObject{{"Type",9},{"Data",QJsonObject{{"APIKey","local-test-key"},
            {"Adcode",110000},{"City","北京"}}}}
    }}}};
    const auto weather=ElementLayout::fromLegacy(weatherLegacy);
    check(weather.size()==1&&weather.first().items.first().type=="weather",
          "old AMap weather item recognized");
    check(weather.first().items.first().weatherAdcode=="110000",
          "numeric AMap adcode migrated to text");
    const auto weatherSaved=ElementLayout::fromJson(ElementLayout::toJson(weather));
    check(weatherSaved.size()==1 && weatherSaved.first().items.first().weatherCity==QStringLiteral("北京"),
          "weather settings survive roundtrip");
    check(restored.first().cycleAfterScroll && restored.first().intervalSeconds==5,
          "old long-text cycles finish scrolling and short text waits five seconds");
    const auto fastLegacy=ElementLayout::fromLegacy(items,nullptr,8);
    check(qAbs(fastLegacy.first().scrollPixelsPerSecond-125)<.001,
          "legacy milliseconds-per-pixel speed is converted");
    const QJsonArray oldTypes{QJsonObject{{"ElementType",2},{"Data",QJsonArray{
        QJsonObject{{"Type",7},{"TextFormat","Preserved headline"}},
        QJsonObject{{"Type",8},{"TextFormat","Preserved daily quote"}},
        QJsonObject{{"Type",6}},QJsonObject{{"Type",10}},
        QJsonObject{{"Type",11}},QJsonObject{{"Type",12}},QJsonObject{{"Type",13}}
    }}}};
    skipped=0;
    const auto oldBlocks=ElementLayout::fromLegacy(oldTypes,&skipped);
    check(oldBlocks.size()==1 && oldBlocks.first().items.size()==7 && skipped==0,
          "all previously placeholder legacy content types are recognized");
    check(ElementLayout::evaluate(oldBlocks.first().items[0],store,now)=="Preserved headline" &&
          ElementLayout::evaluate(oldBlocks.first().items[1],store,now)=="Preserved daily quote",
          "news and daily-word legacy literal formats are retained without a source");
    check(oldBlocks.first().items[2].rotateDuty && oldBlocks.first().autoSize,
          "legacy daily rotation and adaptive course size restored");
    check(oldBlocks.first().items[4].type=="progressBar" &&
          oldBlocks.first().items[5].type=="weatherBlock" &&
          oldBlocks.first().items[6].type=="analogClock",
          "legacy block enums map to distinct graphical renderers");

    ElementDefinition configured;
    configured.rect=QRect(13,17,250,90);configured.mode="all";
    configured.allLayout="grid";configured.columns=3;configured.groupId="group-test";
    configured.autoSize=true;configured.cycleAfterScroll=true;configured.scrollPixelsPerSecond=97.5;
    TextElement configuredItem;configuredItem.type="weatherBlock";
    configuredItem.weatherAddress="Address retained";configuredItem.weatherForecast=true;
    configuredItem.sourceUrl="https://example.org/feed.xml";configuredItem.sourceFile="quote.txt";
    configuredItem.rotateDuty=true;configured.items={configuredItem};
    const auto configuredRoundtrip=ElementDefinition::fromJson(configured.toJson());
    check(configuredRoundtrip.allLayout=="grid" && configuredRoundtrip.columns==3 &&
          configuredRoundtrip.groupId=="group-test" && configuredRoundtrip.autoSize &&
          configuredRoundtrip.cycleAfterScroll && configuredRoundtrip.scrollPixelsPerSecond==97.5,
          "new element layout and animation settings persist");
    const auto configuredText=configuredRoundtrip.items.first();
    check(configuredText.weatherAddress=="Address retained" && configuredText.weatherForecast &&
          configuredText.sourceUrl=="https://example.org/feed.xml" &&
          configuredText.sourceFile=="quote.txt" && configuredText.rotateDuty,
          "weather address, forecast and external sources persist");
    const auto compatibility=ElementDefinition::fromJson(QJsonObject{{"items",QJsonArray{shortText.toJson()}}});
    check(!compatibility.cycleAfterScroll && !compatibility.autoSize &&
          !compatibility.items.first().rotateDuty && compatibility.allLayout=="vertical",
          "pre-existing modern layouts keep their established defaults");
    const auto invalid=ElementDefinition::fromJson(QJsonObject{{"allLayout","unknown"},
          {"columns",-4},{"scrollPixelsPerSecond",-10}});
    check(invalid.allLayout=="vertical" && invalid.columns==1 && invalid.scrollPixelsPerSecond>0,
          "invalid grid and speed settings are normalized safely");

    ElementDefinition afterScroll;afterScroll.rect=QRect(0,0,120,30);
    afterScroll.mode="cycle";afterScroll.cycleAfterScroll=true;
    afterScroll.intervalSeconds=1;afterScroll.scrollPixelsPerSecond=20;
    afterScroll.items={shortText,longText};
    check(ElementLayout::nextChangeDelay(available,{afterScroll},store,now,0)==1000 &&
          ElementLayout::nextChangeDelay(available,{afterScroll},store,now,700)==300,
          "short item schedules an exact transition without frame animation");
    const int longDelay=ElementLayout::nextChangeDelay(available,{afterScroll},store,now,1000);
    check(longDelay>1000 && ElementLayout::hasScrollingText(available,{afterScroll},store,now,2000),
          "long text remains visible beyond the short-item interval");
    check(!ElementLayout::hasScrollingText(available,{afterScroll},store,now,1000LL+longDelay) &&
          ElementLayout::nextChangeDelay(available,{afterScroll},store,now,1000LL+longDelay)==1000,
          "cycle returns to the short item after a complete scrolling pass");
    afterScroll.scrollPixelsPerSecond=40;
    const int fasterDelay=ElementLayout::nextChangeDelay(available,{afterScroll},store,now,1000);
    check(fasterDelay<longDelay && qAbs(fasterDelay*2-longDelay)<=1,
          "changing the speed changes the scrolling cycle duration");
    afterScroll.marquee=false;
    check(ElementLayout::nextChangeDelay(available,{afterScroll},store,now,1000)==1000,
          "a disabled marquee uses the regular cycle interval");

    store.dayInfos[now.date().dayOfWeek()]=QStringList{"First notice","Second notice"};
    TextElement duty;duty.type="duty";duty.rotateDuty=true;duty.format="Info: {value}";
    check(ElementLayout::evaluate(duty,store,now,0)=="Info: First notice" &&
          ElementLayout::evaluate(duty,store,now,4999)=="Info: First notice" &&
          ElementLayout::evaluate(duty,store,now,5000)=="Info: Second notice" &&
          ElementLayout::evaluate(duty,store,now,10000)=="Info: First notice",
          "daily information rotates each five seconds and wraps");
    ElementDefinition dutyElement;dutyElement.rect=QRect(0,0,290,30);dutyElement.items={duty};
    check(ElementLayout::nextChangeDelay(available,{dutyElement},store,now,1234)==3766,
          "daily information wakes even when text does not scroll");
    duty.rotateDuty=false;
    check(ElementLayout::evaluate(duty,store,now,5000).contains("First notice · Second notice"),
          "joined daily information remains available");
    dutyElement.rect.moveLeft(400);
    check(ElementLayout::nextChangeDelay(available,{dutyElement},store,now,0)==0,
          "off-canvas cycles and daily messages do not request a timer");

    const auto render=[&](const QList<ElementDefinition>& elements,qint64 elapsed=0,
                          const QDateTime& date=QDateTime()) {
        QImage image(400,260,QImage::Format_ARGB32_Premultiplied);image.fill(Qt::transparent);
        QPainter drawing(&image);
        ElementLayout::paint(drawing,image.rect(),elements,store,date.isValid()?date:now,elapsed,false);
        return image;
    };
    const auto inkPixels=[](const QImage& image,const QRect& region) {
        int count=0;
        const QRect clipped=region.intersected(image.rect());
        for(int y=clipped.top();y<=clipped.bottom();++y)
            for(int x=clipped.left();x<=clipped.right();++x)
                if(qAlpha(image.pixel(x,y)))++count;
        return count;
    };
    ElementDefinition simultaneous;simultaneous.rect=QRect(10,10,240,120);
    simultaneous.mode="all";simultaneous.marquee=false;
    TextElement a=shortText,b=shortText,c=shortText,d=shortText;
    a.format="Alpha";b.format="Bravo";c.format="Charlie";d.format="Delta";
    simultaneous.items={a,b,c,d};simultaneous.allLayout="grid";simultaneous.columns=2;
    const auto gridImage=render({simultaneous});
    check(inkPixels(gridImage,QRect(10,10,120,60))>0 && inkPixels(gridImage,QRect(130,10,120,60))>0 &&
          inkPixels(gridImage,QRect(10,70,120,60))>0 && inkPixels(gridImage,QRect(130,70,120,60))>0,
          "all-display grid draws a separate item in every cell");
    check(inkPixels(gridImage,QRect(0,140,400,100))==0,
          "grid output is clipped to the element rectangle");
    simultaneous.allLayout="horizontal";
    const auto horizontalImage=render({simultaneous});
    check(inkPixels(horizontalImage,QRect(10,10,60,120))>0 &&
          inkPixels(horizontalImage,QRect(190,10,60,120))>0 && horizontalImage!=gridImage,
          "horizontal simultaneous display creates distinct columns");
    simultaneous.allLayout="vertical";
    const auto verticalImage=render({simultaneous});
    check(inkPixels(verticalImage,QRect(10,10,240,30))>0 &&
          inkPixels(verticalImage,QRect(10,100,240,30))>0 && verticalImage!=horizontalImage,
          "vertical simultaneous display creates distinct rows");
    // Width must be measured against each horizontal cell, not the full element.
    TextElement medium=shortText;medium.format="Medium width phrase";
    const int mediumWidth=QFontMetrics(QFont(medium.font,medium.pointSize)).horizontalAdvance(medium.format);
    simultaneous.rect=QRect(0,0,mediumWidth+20,60);simultaneous.items={medium,medium};
    simultaneous.marquee=true;simultaneous.allLayout="vertical";
    check(!ElementLayout::hasScrollingText(QRect(0,0,400,260),{simultaneous},store,now,0),
          "a phrase fitting a vertical row remains static");
    simultaneous.allLayout="horizontal";
    check(ElementLayout::hasScrollingText(QRect(0,0,400,260),{simultaneous},store,now,0),
          "horizontal cell overflow activates animation");

    TextElement today;today.type="today";
    ElementDefinition adaptive;adaptive.rect=QRect(7,9,20,20);adaptive.items={today};adaptive.autoSize=true;
    const QRect oneCourse=ElementLayout::contentBounds({adaptive},store,now);
    check(oneCourse.topLeft()==QPoint(7,9) && oneCourse.width()>20,
          "adaptive course bounds use course names and preserve their position");
    Course secondCourse=course;secondCourse.name="A much longer course title";
    secondCourse.start=QTime(7,0);secondCourse.end=QTime(8,0);store.courses<<secondCourse;
    const QRect twoCourses=ElementLayout::contentBounds({adaptive},store,now);
    check(twoCourses.width()>oneCourse.width() && twoCourses.height()>oneCourse.height(),
          "adaptive course bounds grow with course count and title length");
    const auto adaptiveImage=render({adaptive});
    check(inkPixels(adaptiveImage,twoCourses)>0 &&
          inkPixels(adaptiveImage,QRect(7,twoCourses.bottom()+1,300,100))==0,
          "adaptive paint uses the same rectangle as content bounds");
    adaptive.items={longText};
    check(!ElementLayout::hasScrollingText(QRect(0,0,3000,260),{adaptive},store,now,0),
          "adaptive text geometry is also respected by animation detection");
    check(ElementLayout::hasScrollingText(QRect(0,0,120,260),{adaptive},store,now,0),
          "a window clipping adaptive text still activates scrolling");

    TextElement progress;progress.type="progressBar";progress.color=QColor("#D02020");
    ElementDefinition block;block.rect=QRect(5,5,180,45);block.items={progress};
    const auto progressStart=render({block},0,now);
    const auto progressHalf=render({block},0,now.addSecs(1800));
    check(progressStart!=progressHalf && qAlpha(progressHalf.pixel(35,42))>
          qAlpha(progressHalf.pixel(150,42)),
          "course progress bar displays the elapsed course fraction");
    TextElement analog;analog.type="analogClock";analog.format="%H:%M:%S";
    block.rect=QRect(5,5,140,165);block.items={analog};
    check(render({block},0,now)!=render({block},0,now.addSecs(1)),
          "analog clock hands and caption advance with the supplied clock");
    TextElement weatherBlock;weatherBlock.type="weatherBlock";
    block.rect=QRect(5,5,300,90);block.items={weatherBlock};
    check(inkPixels(render({block}),block.rect)>500 &&
          !ElementLayout::hasScrollingText(available,{block},store,now,0),
          "weather block renders its card without requesting marquee frames");

    QTemporaryDir sourceDirectory;
    QFile headlines(sourceDirectory.filePath("headlines.txt"));
    check(headlines.open(QIODevice::WriteOnly),"create local headline fixture");
    headlines.write("Headline One\nHeadline Two\n");headlines.close();
    TextElement news;news.type="news";news.sourceFile=headlines.fileName();news.format="News: {value}";
    const auto firstHeadline=ElementLayout::evaluate(news,store,now);
    check(firstHeadline.startsWith("News: Headline ") &&
          firstHeadline!=ElementLayout::evaluate(news,store,now.addSecs(15)),
          "news source is integrated with explicit templates and deterministic rotation");
    block.items={news};
    check(ElementLayout::nextChangeDelay(QRect(0,0,400,260),{block},store,now,0)==15000,
          "news schedules the next headline boundary without scrolling");
    TextElement daily=news;daily.type="dailyWord";daily.format="Quote: {value}";
    check(ElementLayout::evaluate(daily,store,now)==ElementLayout::evaluate(daily,store,now.addSecs(15)) &&
          ElementLayout::evaluate(daily,store,now)!=ElementLayout::evaluate(daily,store,now.addDays(1)),
          "daily words retain a whole-day selection and change on the next date");
    block.items={daily};
    check(ElementLayout::nextChangeDelay(QRect(0,0,400,260),{block},store,now,0)==18*3600*1000,
          "daily word schedules the next local midnight");
    if(!failures)qInfo()<<"Element migration tests passed";
    return failures?1:0;
}
