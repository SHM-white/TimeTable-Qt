#include "DesktopLayoutStore.h"
#include "Translation.h"
#include <QCoreApplication>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonParseError>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>

namespace tt {
namespace {
constexpr qint64 MaximumBytes = 64LL * 1024 * 1024;
const QStringList Types={"clock","current","next","today","week","countdown","note","duty","combined","elements"};
bool fail(QString* error,const QString& message) { if(error)*error=message;return false; }
bool validImage(const QByteArray& bytes) {
    QBuffer buffer;buffer.setData(bytes);if(!buffer.open(QIODevice::ReadOnly))return false;
    QImageReader reader(&buffer);
    const QSize dimensions=reader.size();
    if(!dimensions.isValid() || dimensions.width()>16384 || dimensions.height()>16384 ||
        qint64(dimensions.width())*dimensions.height()>40000000)return false;
    return !reader.read().isNull();
}
QRect rectangle(const QJsonObject& object,const QString& prefix,const QRect& fallback) {
    const QString x=prefix.isEmpty()?"x":prefix+"X";
    const QString y=prefix.isEmpty()?"y":prefix+"Y";
    const QString w=prefix.isEmpty()?"width":prefix+"Width";
    const QString h=prefix.isEmpty()?"height":prefix+"Height";
    return QRect(object.value(x).toInt(fallback.x()),object.value(y).toInt(fallback.y()),
        object.value(w).toInt(fallback.width()),object.value(h).toInt(fallback.height()));
}
bool validRectangle(const QRect& geometry) {
    return geometry.width()>0 && geometry.height()>0 && geometry.width()<=16384 && geometry.height()<=16384 &&
        qAbs(qint64(geometry.x()))<=1000000 && qAbs(qint64(geometry.y()))<=1000000;
}
bool decode(const QJsonObject& j,const QString& sourcePath,OverlayConfig* output,QString* error) {
    const QStringList numberKeys={"x","y","width","height","compactX","compactY","compactWidth","compactHeight",
        "expandedX","expandedY","expandedWidth","expandedHeight","fontSize","opacity","fps","FPS",
        "elementInset","cornerRadius","cycleSeconds"};
    for(const auto& key:numberKeys)if(j.contains(key) && (!j.value(key).isDouble() ||
        j.value(key).toDouble()!=j.value(key).toInt()))return fail(error,tt::translate("字段 %1 必须是整数。").arg(key));
    const QStringList boolKeys={"imageBackground","autoSize","collapsed","compactShowTitle","cycleItems","visible",
        "topmost","locked","showDetailsOnClick","debug","Debug","showFPS","ShowFPS","acrylic"};
    for(const auto& key:boolKeys)if(j.contains(key)&&!j.value(key).isBool())return fail(error,tt::translate("字段 %1 必须是布尔值。").arg(key));
    OverlayConfig c;
    c.id=j.value("id").toString();c.title=j.value("title").toString();
    c.type=j.value("type").toString("current");c.theme=j.value("theme").toString("light");
    if(c.id.isEmpty() || !Types.contains(c.type))return fail(error,tt::translate("窗口 ID 或类型无效。"));
    if(c.theme!="dark" && c.theme!="light")return fail(error,tt::translate("窗口主题无效。"));
    c.note=j.value("note").toString();
    if(j.contains("elements")&&!j.value("elements").isArray())return fail(error,tt::translate("元素列表格式无效。"));
    c.elements=ElementLayout::fromJson(j.value("elements").toArray());
    const auto rawElements=j.value("elements").toArray();
    if(c.elements.size()!=rawElements.size())return fail(error,tt::translate("包含无效或过多的布局元素。"));
    for(int index=0;index<rawElements.size();++index) {
        const auto object=rawElements[index].toObject();
        if(!rawElements[index].isObject() || !object.value("items").isArray() ||
            c.elements[index].items.size()!=object.value("items").toArray().size())
            return fail(error,tt::translate("包含无效或不支持的内容项。"));
    }
    c.backgroundImage=DesktopLayoutStore::resolveBackground(j.value("backgroundImage").toString(),sourcePath);
    const auto asset=j.value("backgroundAsset").toString().toLatin1();
    if(!asset.isEmpty()) {
        static const QRegularExpression base64(QStringLiteral("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$"));
        if(asset.size()>32*1024*1024 || !base64.match(QString::fromLatin1(asset)).hasMatch())
            return fail(error,tt::translate("内嵌背景图片编码无效或过大。"));
        c.backgroundAsset=QByteArray::fromBase64(asset);
        if(!validImage(c.backgroundAsset))return fail(error,tt::translate("内嵌背景图片无效或解码尺寸过大。"));
    }
    c.backgroundColor=QColor(j.value("backgroundColor").toString("#FFFFFF"));
    if(!c.backgroundColor.isValid())return fail(error,tt::translate("背景颜色无效。"));
    c.imageBackground=j.value("imageBackground").toBool(false);c.autoSize=j.value("autoSize").toBool(false);
    c.geometry=rectangle(j,{},QRect(20,20,300,160));
    c.collapsed=j.value("collapsed").toBool(true);
    c.compactGeometry=rectangle(j,"compact",QRect(c.geometry.topLeft(),c.collapsed?c.geometry.size():QSize(195,54)));
    c.expandedGeometry=rectangle(j,"expanded",QRect(c.geometry.topLeft(),c.collapsed?QSize(300,160):c.geometry.size()));
    if(!validRectangle(c.geometry)||!validRectangle(c.compactGeometry)||!validRectangle(c.expandedGeometry))
        return fail(error,tt::translate("窗口尺寸或位置无效。"));
    c.geometry=c.collapsed?c.compactGeometry:c.expandedGeometry;
    c.compactShowTitle=j.value("compactShowTitle").toBool(false);
    c.combinedItems=j.value("combinedItems").toString("clock,current,next");
    c.cycleItems=j.value("cycleItems").toBool(false);c.cycleSeconds=j.value("cycleSeconds").toInt(5);
    c.visible=j.value("visible").toBool(true);c.topmost=j.value("topmost").toBool(true);
    c.locked=j.value("locked").toBool(false);c.showDetailsOnClick=j.value("showDetailsOnClick").toBool(true);
    c.fontSize=j.value("fontSize").toInt(13);c.opacity=j.value("opacity").toInt(94);
    c.fps=j.value("fps").toInt(j.value("FPS").toInt(60));
    c.debug=j.value("debug").toBool(j.value("Debug").toBool(false));
    c.showFPS=j.value("showFPS").toBool(j.value("ShowFPS").toBool(false));
    c.elementInset=j.value("elementInset").toInt(5);c.cornerRadius=j.value("cornerRadius").toInt(17);
    c.acrylic=j.value("acrylic").toBool(false);
    if(c.fontSize<8||c.fontSize>35||c.opacity<0||c.opacity>100||c.fps<0||c.fps>120||
        c.elementInset<0||c.elementInset>100||c.cornerRadius<0||c.cornerRadius>100||c.cycleSeconds<1||c.cycleSeconds>300)
        return fail(error,tt::translate("窗口外观或动画参数超出允许范围。"));
    c.target=QDateTime::fromString(j.value("target").toString(),Qt::ISODate);
    *output=c;return true;
}
}

QString DesktopLayoutStore::defaultPath() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("desktop-layout.json");
}
QString DesktopLayoutStore::resolveBackground(const QString& image,const QString& configPath,const QString& executableDirectory) {
    if(image.isEmpty() || QFileInfo(image).isAbsolute() || image.startsWith("embedded:"))return image;
    if(!configPath.isEmpty()) {
        const QString local=QFileInfo(configPath).dir().absoluteFilePath(image);
        if(QFileInfo::exists(local))return QDir::cleanPath(local);
    }
    const QString executable=QDir(executableDirectory.isEmpty()?QCoreApplication::applicationDirPath():executableDirectory).absoluteFilePath(image);
    if(QFileInfo::exists(executable))return QDir::cleanPath(executable);
    return configPath.isEmpty()?image:QFileInfo(configPath).dir().absoluteFilePath(image);
}
QJsonObject DesktopLayoutStore::toJson(const OverlayConfig& c) {
    QJsonObject object{
        {"id",c.id},{"title",c.title},{"type",c.type},{"theme",c.theme},{"note",c.note},
        {"elements",ElementLayout::toJson(c.elements)},{"backgroundImage",c.backgroundImage},
        {"backgroundColor",c.backgroundColor.name(QColor::HexArgb)},
        {"imageBackground",c.imageBackground},{"autoSize",c.autoSize},
        {"x",c.geometry.x()},{"y",c.geometry.y()},{"width",c.geometry.width()},{"height",c.geometry.height()},
        {"visible",c.visible},{"topmost",c.topmost},{"locked",c.locked},{"showDetailsOnClick",c.showDetailsOnClick},
        {"collapsed",c.collapsed},{"compactShowTitle",c.compactShowTitle},
        {"combinedItems",c.combinedItems},{"cycleItems",c.cycleItems},{"cycleSeconds",c.cycleSeconds},
        {"compactX",c.compactGeometry.x()},{"compactY",c.compactGeometry.y()},
        {"compactWidth",c.compactGeometry.width()},{"compactHeight",c.compactGeometry.height()},
        {"expandedX",c.expandedGeometry.x()},{"expandedY",c.expandedGeometry.y()},
        {"expandedWidth",c.expandedGeometry.width()},{"expandedHeight",c.expandedGeometry.height()},
        {"fontSize",c.fontSize},{"opacity",c.opacity},{"target",c.target.toString(Qt::ISODate)},
        {"fps",c.fps},{"debug",c.debug},{"showFPS",c.showFPS},{"elementInset",c.elementInset},
        {"cornerRadius",c.cornerRadius},{"acrylic",c.acrylic}
    };
    if(!c.backgroundAsset.isEmpty())object.insert("backgroundAsset",QString::fromLatin1(c.backgroundAsset.toBase64()));
    return object;
}
bool DesktopLayoutStore::parse(const QByteArray& raw,QVector<OverlayConfig>* configs,QString* error,const QString& sourcePath) {
    if(!configs)return fail(error,tt::translate("没有指定配置输出。"));
    if(raw.size()>MaximumBytes)return fail(error,tt::translate("布局文件超过 64 MiB。"));
    QByteArray bytes=raw;if(bytes.startsWith(QByteArray::fromHex("EFBBBF")))bytes.remove(0,3);
    QJsonParseError jsonError;
    const auto document=QJsonDocument::fromJson(bytes,&jsonError);
    if(jsonError.error!=QJsonParseError::NoError)return fail(error,jsonError.errorString());
    QJsonArray windows;
    if(document.isArray())windows=document.array(); // Previous QSettings persistence.
    else if(document.isObject()) {
        const auto root=document.object();
        if(root.value("format").toString()!="TimeTableNext/DesktopLayout" || root.value("version").toInt()!=1 || !root.value("windows").isArray())
            return fail(error,tt::translate("无法识别桌面布局格式或版本。"));
        windows=root.value("windows").toArray();
    } else return fail(error,tt::translate("桌面布局必须是 JSON 对象或窗口数组。"));
    if(windows.size()>500)return fail(error,tt::translate("桌面窗口数量超过 500。"));
    QVector<OverlayConfig> candidate;QSet<QString> ids;
    for(const auto& rawWindow:windows) {
        if(!rawWindow.isObject())return fail(error,tt::translate("窗口配置必须是 JSON 对象。"));
        OverlayConfig config;
        if(!decode(rawWindow.toObject(),sourcePath,&config,error))return false;
        if(ids.contains(config.id))return fail(error,tt::translate("桌面窗口 ID 重复。"));
        ids.insert(config.id);candidate<<config;
    }
    *configs=candidate;if(error)error->clear();return true;
}
bool DesktopLayoutStore::read(const QString& path,QVector<OverlayConfig>* configs,QString* error) {
    QFile file(path);if(!file.open(QIODevice::ReadOnly))return fail(error,file.errorString());
    if(file.size()>MaximumBytes)return fail(error,tt::translate("布局文件超过 64 MiB。"));
    return parse(file.readAll(),configs,error,path);
}
bool DesktopLayoutStore::write(const QString& path,const QVector<OverlayConfig>& configs,QString* error,bool embedImages) {
    if(path.trimmed().isEmpty())return fail(error,tt::translate("布局路径不能为空。"));
    QJsonArray windows;
    for(auto config:configs) {
        if(embedImages && config.backgroundAsset.isEmpty() && !config.backgroundImage.isEmpty() && !config.backgroundImage.startsWith("embedded:")) {
            QFile image(config.backgroundImage);
            if(!image.open(QIODevice::ReadOnly))return fail(error,tt::translate("无法备份背景图片：%1").arg(config.backgroundImage));
            if(image.size()>24*1024*1024)return fail(error,tt::translate("背景图片超过 24 MiB。"));
            config.backgroundAsset=image.readAll();
            if(!validImage(config.backgroundAsset))return fail(error,tt::translate("背景图片无效或解码尺寸过大：%1").arg(config.backgroundImage));
            config.backgroundImage="embedded:"+QFileInfo(config.backgroundImage).fileName();
        }
        windows.append(toJson(config));
    }
    const auto bytes=QJsonDocument(QJsonObject{{"format","TimeTableNext/DesktopLayout"},{"version",1},{"windows",windows}}).toJson(QJsonDocument::Indented);
    QVector<OverlayConfig> validated;if(!parse(bytes,&validated,error,path))return false;
    const QString directory=QFileInfo(path).absolutePath();
    if(!QDir().mkpath(directory))return fail(error,tt::translate("无法创建布局目录：%1").arg(directory));
    QSaveFile file(path);if(!file.open(QIODevice::WriteOnly))return fail(error,file.errorString());
    if(file.write(bytes)!=bytes.size() || !file.commit())return fail(error,file.errorString());
    if(error)error->clear();return true;
}
bool DesktopLayoutStore::fromLegacy(const QJsonObject& root,const QString& sourcePath,QVector<OverlayConfig>* configs,int* skipped,QString* error) {
    if(!configs)return fail(error,tt::translate("没有指定配置输出。"));
    auto windows=root.value("Windows").toArray();
    if(root.value("MainWindow").isObject())windows.prepend(root.value("MainWindow"));
    if(windows.isEmpty())return fail(error,tt::translate("找不到旧版 Windows 或 MainWindow。"));
    QVector<OverlayConfig> result;int rejected=0;
    const qreal scrollSpeed=std::max(1.0,root.value("ItemScrollSpeed").toDouble(35.0));
    for(const auto& raw:windows) {
        if(!raw.isObject()) {++rejected;continue;}
        const auto old=raw.toObject();const auto definitions=old.value("UIElements").toArray();
        if(definitions.isEmpty()) {++rejected;continue;}
        int invalid=0;
        const auto elements=ElementLayout::fromLegacy(definitions,&invalid,scrollSpeed);
        rejected+=invalid;
        if(elements.isEmpty())continue; // Count unsupported entries only once.
        OverlayConfig c;c.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
        c.type="elements";c.elements=elements;c.collapsed=false;c.opacity=100;c.elementInset=0;c.cornerRadius=0;
        c.title=old.value("Name").toString(old.value("name").toString()).trimmed();
        if(c.title.isEmpty())c.title=tt::translate("旧版窗口 %1").arg(result.size()+1);
        c.autoSize=old.value("AutoResize").toBool(false);
        c.fps=std::clamp(old.value("FPS").toInt(root.value("FPS").toInt(30)),0,120);
        c.debug=old.value("Debug").toBool(root.value("Debug").toBool(false));
        c.showFPS=old.value("ShowFPS").toBool(root.value("ShowFPS").toBool(false));
        c.acrylic=old.value("AcrylicEffect").toBool(false);
        const auto rgba=old.value("BackGroundColor").toArray();
        if(rgba.size()>=3)c.backgroundColor=QColor(std::clamp(rgba[0].toInt(),0,255),std::clamp(rgba[1].toInt(),0,255),
            std::clamp(rgba[2].toInt(),0,255),rgba.size()>3?std::clamp(rgba[3].toInt(),0,255):255);
        c.backgroundImage=resolveBackground(old.value("BackGroundImg").toString(),sourcePath);
        c.imageBackground=old.value("ImgAsBackGround").toBool(false);
        const auto location=old.value("WindowLocation").toArray();const auto dimensions=old.value("WindowSize").toArray();
        const QPoint origin(location.size()>=2?location[0].toInt():20,location.size()>=2?location[1].toInt():20);
        const auto bounds=ElementLayout::contentBounds(c.elements);
        QSize size(std::max(1,bounds.right()+1),std::max(1,bounds.bottom()+1));
        if(dimensions.size()>=2&&dimensions[0].toInt()>0&&dimensions[1].toInt()>0)
            size=QSize(std::clamp(dimensions[0].toInt(),1,16384),std::clamp(dimensions[1].toInt(),1,16384));
        c.geometry=QRect(origin,size);c.compactGeometry=c.geometry;c.expandedGeometry=c.geometry;
        c.visible=old.value("AutoOpen").toBool(true);c.topmost=old.value("TopMost").toBool(true);
        c.locked=!old.value("Moveable").toBool(true);result<<c;
    }
    if(result.isEmpty())return fail(error,tt::translate("旧配置中没有可识别的信息元素。"));
    *configs=result;if(skipped)*skipped=rejected;if(error)error->clear();return true;
}
}
