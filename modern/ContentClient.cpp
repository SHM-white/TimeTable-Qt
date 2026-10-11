#include "ContentClient.h"
#include "Translation.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>
#include <QXmlStreamReader>

namespace tt {
namespace {
constexpr int maxSourceBytes=1024*1024;
constexpr int maxItems=256;
constexpr int maxTextLength=800;
QString plainText(QString text) {
    static const QRegularExpression tags(QStringLiteral("<[^>]*>"));
    text.remove(tags);
    text.replace(QStringLiteral("&nbsp;"),QStringLiteral(" "));
    text.replace(QStringLiteral("&amp;"),QStringLiteral("&"));
    text.replace(QStringLiteral("&lt;"),QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"),QStringLiteral(">"));
    text.replace(QStringLiteral("&quot;"),QStringLiteral("\""));
    text.replace(QStringLiteral("&#39;"),QStringLiteral("'"));
    text=text.simplified();
    if(text.size()>maxTextLength)text=text.left(maxTextLength-1)+QChar(0x2026);
    return text;
}
void appendItem(QStringList& items,const QString& text) {
    if(items.size()>=maxItems)return;
    const auto clean=plainText(text);
    if(!clean.isEmpty()&&!items.contains(clean))items<<clean;
}
QString stringValue(const QJsonObject& object,const QStringList& keys) {
    for(const auto& key:keys) {
        const auto value=object.value(key);
        if(value.isString()&&!value.toString().trimmed().isEmpty())return value.toString();
    }
    return {};
}
void jsonItems(const QJsonValue& value,const QString& kind,QStringList& items,int depth=0) {
    if(depth>6||items.size()>=maxItems)return;
    if(value.isString()) { appendItem(items,value.toString());return; }
    if(value.isArray()) {
        for(const auto& item:value.toArray()) {
            jsonItems(item,kind,items,depth+1);
            if(items.size()>=maxItems)break;
        }
        return;
    }
    if(!value.isObject())return;
    const auto object=value.toObject();
    const auto content=kind=="news"?
        stringValue(object,{"title","headline","text"}):
        stringValue(object,{"content","text","sentence","hitokoto","quote","title"});
    bool collection=false;
    for(const QString& key:{QStringLiteral("articles"),QStringLiteral("items"),
            QStringLiteral("news"),QStringLiteral("results"),QStringLiteral("data"),
            QStringLiteral("quotes"),QStringLiteral("sentences")})
        collection=collection||object.value(key).isArray()||object.value(key).isObject();
    if(!content.isEmpty()&&(kind!="news"||!collection)) {
        QString text=content;
        if(kind=="dailyWord") {
            auto author=stringValue(object,{"author","from_who","creator"});
            if(author.isEmpty()&&object.value("author").isObject())
                author=stringValue(object.value("author").toObject(),{"name"});
            if(!author.isEmpty())text+=QStringLiteral(" —— ")+author;
        }
        appendItem(items,text);
        // A feed-level title must not hide its articles; daily quotes can also
        // accompany an explicitly supplied collection.
    }
    for(const QString& key:{QStringLiteral("articles"),QStringLiteral("items"),
            QStringLiteral("news"),QStringLiteral("results"),QStringLiteral("data"),
            QStringLiteral("quotes"),QStringLiteral("sentences")}) {
        if(object.contains(key))jsonItems(object.value(key),kind,items,depth+1);
    }
}
QStringList xmlItems(const QByteArray& bytes,const QString& kind,QString* error) {
    QXmlStreamReader xml(bytes);
    QStringList items;
    int depth=0;
    int entryDepth=-1;
    QString title,content,author;
    while(!xml.atEnd()) {
        xml.readNext();
        if(xml.isDTD()) {
            if(error)*error=tt::translate("内容源不支持 XML DTD");
            return {};
        }
        if(xml.isStartElement()) {
            ++depth;
            const auto name=xml.name().toString().toLower();
            if(entryDepth<0&&(name=="item"||name=="entry")) {
                entryDepth=depth;title.clear();content.clear();author.clear();
            } else if(entryDepth>=0&&(name=="title"||name=="content"||
                       name=="description"||name=="summary"||name=="creator"||
                       name=="name")) {
                const auto text=xml.readElementText(QXmlStreamReader::IncludeChildElements);
                --depth; // readElementText consumes the matching end element.
                if(name=="title")title=text;
                else if(name=="creator"||name=="name")author=text;
                else if(content.isEmpty())content=text;
            }
        } else if(xml.isEndElement()) {
            if(depth==entryDepth) {
                QString text=kind=="news"?title:(content.isEmpty()?title:content);
                if(kind=="dailyWord"&&!author.trimmed().isEmpty())text+=QStringLiteral(" —— ")+author;
                appendItem(items,text);
                entryDepth=-1;
            }
            --depth;
        }
    }
    if(xml.hasError()) {
        if(error)*error=tt::translate("内容源不是有效的 RSS / Atom XML");
        return {};
    }
    return items;
}
QString prefix(const QString& kind) {
    return kind=="news"?tt::translate("新闻"):tt::translate("每日一句");
}
}
ContentClient& ContentClient::instance() {
    static ContentClient singleton;
    return singleton;
}
ContentClient::ContentClient():QObject(nullptr) {}
QStringList ContentClient::parseItems(const QByteArray& bytes,const QString& kind,QString* error) {
    if(error)error->clear();
    if(kind!="news"&&kind!="dailyWord") {
        if(error)*error=tt::translate("未知内容类型");
        return {};
    }
    if(bytes.size()>maxSourceBytes) {
        if(error)*error=tt::translate("内容源超过 1 MiB 上限");
        return {};
    }
    QByteArray input=bytes;
    if(input.startsWith("\xEF\xBB\xBF"))input.remove(0,3);
    input=input.trimmed();
    QStringList items;
    if(input.startsWith('<'))items=xmlItems(input,kind,error);
    else if(input.startsWith('{')||input.startsWith('[')) {
        QJsonParseError failure;
        const auto doc=QJsonDocument::fromJson(input,&failure);
        if(failure.error!=QJsonParseError::NoError) {
            if(error)*error=tt::translate("内容源不是有效的 JSON");
            return {};
        }
        jsonItems(doc.isArray()?QJsonValue(doc.array()):QJsonValue(doc.object()),kind,items);
    } else {
        const auto text=QString::fromUtf8(input);
        if(text.toUtf8()!=input) {
            if(error)*error=tt::translate("本地文本须使用 UTF-8 编码");
            return {};
        }
        for(const auto& line:text.split('\n'))appendItem(items,line);
    }
    if(items.isEmpty()&&error&&error->isEmpty())*error=tt::translate("内容源没有可显示的条目");
    return items;
}
QString ContentClient::selectText(const QStringList& items,const QString& kind,
                                 const QDate& date,int rotation) {
    if(items.isEmpty())return {};
    qint64 index=kind=="dailyWord"?date.toJulianDay():rotation;
    index=((index%items.size())+items.size())%items.size();
    return items.at(int(index));
}
QString ContentClient::text(const QString& kind,const QString& sourceUrl,const QString& sourceFile,
                            const QDateTime& displayNow) {
    if(kind!="news"&&kind!="dailyWord")return tt::translate("未知内容类型");
    const QString label=prefix(kind);
    const QString filePath=sourceFile.trimmed();
    const QString urlText=sourceUrl.trimmed();
    if(filePath.isEmpty()&&urlText.isEmpty())
        return label+tt::translate("未配置：请选择 UTF-8 文件或填写 HTTPS RSS / JSON 地址");
    QUrl url;
    if(filePath.isEmpty()) {
        url=QUrl(urlText,QUrl::StrictMode);
        if(!url.isValid()||url.scheme().toLower()!="https"||url.host().isEmpty()||!url.userInfo().isEmpty())
            return label+tt::translate("内容源须为不含登录凭据的 HTTPS 地址");
    }
    const QString source=filePath.isEmpty()?url.toString(QUrl::FullyEncoded):QFileInfo(filePath).absoluteFilePath();
    const QString key=kind+(filePath.isEmpty()?"|url|":"|file|")+source;
    if(!cache_.contains(key)&&cache_.size()>=32) {
        for(auto it=cache_.begin();it!=cache_.end();) {
            if(!it->requested)it=cache_.erase(it);else ++it;
        }
    }
    auto& item=cache_[key];
    const auto now=QDateTime::currentDateTimeUtc();
    if(!item.requested&&(!item.retryAfter.isValid()||item.retryAfter<=now)) {
        if(!filePath.isEmpty()) {
            item.retryAfter=now.addSecs(2); // Rendering may call text() every frame.
            const QFileInfo info(source);
            if(!info.exists()||!info.isFile())item.error=tt::translate("本地文件不存在");
            else if(info.size()>maxSourceBytes)item.error=tt::translate("内容源超过 1 MiB 上限");
            else if(item.items.isEmpty()||info.lastModified()!=item.fileModified||info.size()!=item.fileSize) {
                QFile file(source);
                if(!file.open(QIODevice::ReadOnly))item.error=tt::translate("无法读取本地文件");
                else {
                    QString error;
                    const auto items=parseItems(file.read(maxSourceBytes+1),kind,&error);
                    item.error=error;
                    if(!items.isEmpty()) {
                        item.items=items;
                        item.fileModified=info.lastModified();item.fileSize=info.size();
                    }
                }
            } else item.error.clear();
        } else {
            item.requested=true;
            QNetworkRequest request(url);
            request.setRawHeader("Accept","application/rss+xml, application/atom+xml, application/json, text/plain");
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::NoLessSafeRedirectPolicy);
            auto* reply=network_.get(request);
            QObject::connect(reply,&QNetworkReply::readyRead,this,[reply] {
                QByteArray bytes=reply->property("contentBytes").toByteArray();
                bytes+=reply->readAll();
                if(bytes.size()>maxSourceBytes) {
                    reply->setProperty("contentFailure",tt::translate("内容源超过 1 MiB 上限"));
                    reply->abort();
                } else reply->setProperty("contentBytes",bytes);
            });
            QTimer::singleShot(15000,reply,[reply] {
                if(!reply->isFinished()) {
                    reply->setProperty("contentFailure",tt::translate("内容请求超时"));
                    reply->abort();
                }
            });
            QObject::connect(reply,&QNetworkReply::finished,this,[this,key,kind,reply] {
                auto& result=cache_[key];
                result.requested=false;
                QString error=reply->property("contentFailure").toString();
                QByteArray bytes=reply->property("contentBytes").toByteArray();
                bytes+=reply->readAll();
                if(bytes.size()>maxSourceBytes)error=tt::translate("内容源超过 1 MiB 上限");
                if(error.isEmpty()&&reply->error()!=QNetworkReply::NoError)error=tt::translate("网络连接失败");
                if(error.isEmpty()) {
                    const auto items=parseItems(bytes,kind,&error);
                    if(!items.isEmpty())result.items=items;
                }
                result.error=error;
                result.retryAfter=QDateTime::currentDateTimeUtc().addSecs(error.isEmpty()?900:300);
                reply->deleteLater();
            });
        }
    }
    if(!item.items.isEmpty()) {
        // Headlines change every 15 seconds; daily words use a stable date index.
        const int rotation=int((displayNow.toMSecsSinceEpoch()/15000)%item.items.size());
        const QString selected=selectText(item.items,kind,displayNow.date(),rotation);
        return item.error.isEmpty()?selected:selected+tt::translate("（更新失败，显示缓存）");
    }
    if(!item.error.isEmpty())return label+tt::translate("获取失败：")+item.error;
    return tt::translate("正在获取")+label+QStringLiteral("…");
}
}
