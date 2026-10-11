#include "DesktopLayoutStore.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QTemporaryDir>
#include <cmath>
using namespace tt;
namespace {
int failures=0;
void check(bool value,const char* name) { if(!value){qCritical()<<"FAILED:"<<name;++failures;} }
QByteArray bytes(const QString& path) { QFile file(path);if(!file.open(QIODevice::ReadOnly))return {};return file.readAll(); }
void put(const QString& path,const QByteArray& data) { QFile file(path);if(file.open(QIODevice::WriteOnly))file.write(data); }
}
int main(int argc,char** argv) {
    QApplication application(argc,argv);
    QTemporaryDir temporary;
    if(!temporary.isValid())return 1;
    const QString source=temporary.filePath("source/layout.json");
    QDir().mkpath(temporary.filePath("source"));
    QImage background(8,8,QImage::Format_ARGB32);background.fill(QColor(14,40,100,210));
    const QString image=temporary.filePath("source/background.png");
    check(background.save(image),"create background fixture");
    OverlayConfig config;config.id="window";config.title=QStringLiteral("迁移小窗口");config.type="elements";
    config.collapsed=false;config.geometry=QRect(43,56,80,24);config.compactGeometry=config.geometry;config.expandedGeometry=config.geometry;
    config.backgroundImage=image;config.imageBackground=true;config.elementInset=0;config.cornerRadius=0;config.opacity=100;
    config.fps=0;config.debug=true;config.showFPS=true;config.acrylic=true;config.visible=false;
    ElementDefinition definition;definition.rect=QRect(0,0,80,24);definition.mode="all";
    definition.groupId="group";definition.allLayout="horizontal";definition.scrollPixelsPerSecond=42;definition.cycleAfterScroll=true;
    TextElement text;text.type="note";text.format="Portable layout";definition.items<<text;
    config.elements<<definition;
    QString error;QVector<OverlayConfig> restored;
    check(DesktopLayoutStore::write(source,{config},&error),"atomic custom-path persistence");
    check(DesktopLayoutStore::read(source,&restored,&error)&&restored.size()==1,"read custom layout");
    if(!restored.isEmpty()) {
        const auto c=restored.first();
        check(c.geometry==config.geometry&&c.elementInset==0&&c.cornerRadius==0&&c.opacity==100,"exact small geometry and pixel style retained");
        check(c.fps==0&&c.debug&&c.showFPS&&c.acrylic&&!c.visible,"appearance and startup preferences retained");
        check(c.elements.first().groupId=="group"&&c.elements.first().cycleAfterScroll,"complete grouped element settings retained");
    }
    const QByteArray original=bytes(source);
    OverlayConfig bad=config;bad.fps=121;
    check(!DesktopLayoutStore::write(source,{bad},&error)&&bytes(source)==original,"invalid write preserves original file");
    QImage oversized(17000,1,QImage::Format_ARGB32);oversized.fill(Qt::black);
    const QString largeImage=temporary.filePath("too-wide.png");oversized.save(largeImage);
    bad=config;bad.backgroundImage=largeImage;
    check(!DesktopLayoutStore::write(source,{bad},&error,true)&&bytes(source)==original,
        "oversized decoded image is rejected before replacing the layout");
    QVector<OverlayConfig> previous{config};
    check(!DesktopLayoutStore::parse("{ broken",&previous,&error)&&previous.size()==1&&previous.first().id=="window",
        "invalid document preserves caller state");
    const auto duplicate=QJsonDocument(QJsonArray{DesktopLayoutStore::toJson(config),DesktopLayoutStore::toJson(config)}).toJson();
    check(!DesktopLayoutStore::parse(duplicate,&previous,&error)&&previous.size()==1,"duplicate IDs reject entire document");
    auto malformed=DesktopLayoutStore::toJson(config);malformed.insert("fps","broken");
    check(!DesktopLayoutStore::parse(QJsonDocument(QJsonArray{malformed}).toJson(),&previous,&error),"malformed integer fields rejected");
    const QString backup=temporary.filePath("backup.json");
    check(DesktopLayoutStore::write(backup,{config},&error,true),"portable backup embeds image");
    QFile::remove(image);
    check(DesktopLayoutStore::read(backup,&restored,&error)&&!restored.first().backgroundAsset.isEmpty(),"portable image restores after source removal");
    if(!restored.isEmpty())check(QImage::fromData(restored.first().backgroundAsset)==background,"portable background retains pixels and alpha");
    check(DesktopLayoutStore::write(source,restored,&error)&&DesktopLayoutStore::read(source,&restored,&error)&&!restored.first().backgroundAsset.isEmpty(),
        "restored embedded image survives ordinary future saves");
    const QString exe=temporary.filePath("exe");QDir().mkpath(exe);
    put(exe+"/relative.png","exe");put(temporary.filePath("source/relative.png"),"json");
    check(DesktopLayoutStore::resolveBackground("relative.png",source,exe)==temporary.filePath("source/relative.png"),"relative image uses JSON directory first");
    QFile::remove(temporary.filePath("source/relative.png"));
    check(DesktopLayoutStore::resolveBackground("relative.png",source,exe)==exe+"/relative.png","relative image falls back to executable directory");
    const QJsonArray legacyItems{
        QJsonObject{{"ElementType",1},{"Location",QJsonArray{0,0}},{"Size",QJsonArray{80,24}},
            {"Data",QJsonArray{QJsonObject{{"Type",0},{"TextFormat","Small"}},QJsonObject{{"Type",999}}}}}};
    const QJsonObject legacyWindow{{"UIElements",legacyItems},{"Name","Legacy"},{"WindowLocation",QJsonArray{43,56}},
        {"WindowSize",QJsonArray{80,24}},{"FPS",0},{"Debug",true},{"ShowFPS",true},{"AutoOpen",false},
        {"BackGroundColor",QJsonArray{100,120,130,145}},{"ImgAsBackGround",false}};
    const QJsonObject legacy{{"ItemScrollSpeed",20},{"MainWindow",legacyWindow}};
    int skipped=0;
    check(DesktopLayoutStore::fromLegacy(legacy,source,&restored,&skipped,&error)&&restored.size()==1,"legacy main window migrates independently");
    check(skipped==1,"unsupported legacy entry counted once");
    if(!restored.isEmpty()) {
        const auto c=restored.first();
        check(c.geometry==QRect(43,56,80,24)&&c.elementInset==0&&c.cornerRadius==0&&c.opacity==100,"legacy dimensions are not expanded to modern minimum");
        check(c.backgroundColor.alpha()==145&&c.fps==0&&c.debug&&c.showFPS&&!c.visible,"legacy alpha FPS diagnostics and startup settings migrate");
        check(std::abs(c.elements.first().scrollPixelsPerSecond-50.0)<0.001,"legacy milliseconds per pixel converts to scrolling speed");
    }
    if(!failures)qInfo()<<"Desktop layout storage tests passed";
    return failures?1:0;
}
