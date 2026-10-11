#include "WeatherClient.h"
#include "Translation.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>
#include <QUrlQuery>
#include <functional>

namespace tt {
namespace {
constexpr int maxReplyBytes=256*1024;
QString jsonText(const QJsonValue& value) {
    if(value.isString())return value.toString().trimmed();
    if(value.isDouble())return QString::number(value.toInt());
    return {};
}
void requestJson(QNetworkAccessManager& network,QObject* context,const QUrl& url,
                 const std::function<void(const QByteArray&,const QString&)>& complete) {
    QNetworkRequest request(url);
    request.setRawHeader("Accept","application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::SameOriginRedirectPolicy);
    auto* reply=network.get(request);
    QObject::connect(reply,&QNetworkReply::readyRead,context,[reply] {
        QByteArray bytes=reply->property("weatherBytes").toByteArray();
        bytes+=reply->readAll();
        if(bytes.size()>maxReplyBytes) {
            reply->setProperty("weatherFailure",tt::translate("天气响应过大"));
            reply->abort();
        } else reply->setProperty("weatherBytes",bytes);
    });
    QTimer::singleShot(10000,reply,[reply] {
        if(!reply->isFinished()) {
            reply->setProperty("weatherFailure",tt::translate("天气请求超时"));
            reply->abort();
        }
    });
    QObject::connect(reply,&QNetworkReply::finished,context,[reply,complete] {
        QByteArray bytes=reply->property("weatherBytes").toByteArray();
        bytes+=reply->readAll();
        QString error=reply->property("weatherFailure").toString();
        if(bytes.size()>maxReplyBytes)error=tt::translate("天气响应过大");
        if(error.isEmpty()&&reply->error()!=QNetworkReply::NoError)
            error=tt::translate("网络连接失败，请检查网络和高德 API Key");
        complete(error.isEmpty()?bytes:QByteArray(),error);
        reply->deleteLater();
    });
}
QJsonObject successfulRoot(const QByteArray& bytes,QString* error) {
    if(bytes.size()>maxReplyBytes) {
        if(error)*error=tt::translate("天气响应过大");
        return {};
    }
    QJsonParseError failure;
    const auto doc=QJsonDocument::fromJson(bytes,&failure);
    if(failure.error!=QJsonParseError::NoError||!doc.isObject()) {
        if(error)*error=tt::translate("天气服务返回了无效 JSON");
        return {};
    }
    const auto root=doc.object();
    if(jsonText(root.value("status"))!="1") {
        if(error)*error=tt::translate("高德服务拒绝请求，请检查 API Key 和地区设置");
        return {};
    }
    return root;
}
QString displayResult(const QString& text,bool stale,const QString& loading) {
    if(text.isEmpty())return loading;
    return stale?text+tt::translate("（更新失败，显示缓存）"):text;
}
}
WeatherClient& WeatherClient::instance() {
    static WeatherClient singleton;
    return singleton;
}
WeatherClient::WeatherClient():QObject(nullptr) {}
QString WeatherClient::parseAdcode(const QByteArray& bytes,QString* error) {
    if(error)error->clear();
    const auto root=successfulRoot(bytes,error);
    if(root.isEmpty())return {};
    const auto rows=root.value("geocodes").toArray();
    static const QRegularExpression validCode(QStringLiteral("^\\d{6}$"));
    for(const auto& row:rows) {
        const auto code=jsonText(row.toObject().value("adcode"));
        if(validCode.match(code).hasMatch())return code;
    }
    if(error)*error=tt::translate("无法识别地址，请填写更完整的地址或行政区划代码");
    return {};
}
QString WeatherClient::parseWeather(const QByteArray& bytes,const QString& city,bool forecast,
                                    const QDate& today,QString* error) {
    if(error)error->clear();
    const auto root=successfulRoot(bytes,error);
    if(root.isEmpty())return {};
    if(!forecast) {
        const auto rows=root.value("lives").toArray();
        if(!rows.isEmpty()) {
            const auto info=rows.first().toObject();
            const auto weather=jsonText(info.value("weather"));
            const auto temperature=jsonText(info.value("temperature"));
            QString label=jsonText(info.value("city"));
            if(label.isEmpty())label=city.trimmed();
            if(!weather.isEmpty()&&!temperature.isEmpty())
                return (label+"  "+weather+"  "+temperature+QStringLiteral("℃")).trimmed();
        }
    } else {
        const auto rows=root.value("forecasts").toArray();
        if(!rows.isEmpty()) {
            const auto info=rows.first().toObject();
            QString label=jsonText(info.value("city"));
            if(label.isEmpty())label=city.trimmed();
            const auto casts=info.value("casts").toArray();
            QStringList days;
            for(const auto& value:casts) {
                const auto cast=value.toObject();
                const auto date=QDate::fromString(jsonText(cast.value("date")),Qt::ISODate);
                if(date!=today&&date!=today.addDays(1))continue;
                const auto day=jsonText(cast.value("dayweather"));
                const auto night=jsonText(cast.value("nightweather"));
                const auto low=jsonText(cast.value("nighttemp"));
                const auto high=jsonText(cast.value("daytemp"));
                if(day.isEmpty()||low.isEmpty()||high.isEmpty())continue;
                const QString weather=night.isEmpty()||day==night?day:day+" / "+night;
                const QString prefix=date==today?tt::translate("今天"):tt::translate("明天");
                days<<prefix+" "+weather+" "+low+QStringLiteral("～")+high+QStringLiteral("℃");
            }
            if(!days.isEmpty())return (label+"  "+days.join(QStringLiteral("；"))).trimmed();
        }
    }
    if(error)*error=forecast?tt::translate("天气服务未返回今日或明日预报"):
                            tt::translate("天气服务未返回有效实况");
    return {};
}
QString WeatherClient::text(const QString& apiKey,const QString& adcode,const QString& city,
                           const QString& address,bool forecast) {
    const auto secret=apiKey.trimmed();
    if(secret.isEmpty())return tt::translate("天气未配置：请填写高德 API Key");
    QString code=adcode.trimmed();
    const auto now=QDateTime::currentDateTimeUtc();
    if(code.isEmpty()) {
        const auto place=address.trimmed().isEmpty()?city.trimmed():address.trimmed();
        if(place.isEmpty())return tt::translate("天气未配置：请填写地区代码、城市或地址");
        const QString locationKey=secret+"|"+place+"|"+city.trimmed();
        if(!locations_.contains(locationKey)&&locations_.size()>=32) {
            for(auto it=locations_.begin();it!=locations_.end();) {
                if(!it->requested)it=locations_.erase(it);else ++it;
            }
        }
        auto& location=locations_[locationKey];
        code=location.adcode;
        if(code.isEmpty()) {
            if(!location.requested&&(!location.retryAfter.isValid()||location.retryAfter<=now)) {
                location.requested=true;
                location.error.clear();
                QUrl url(QStringLiteral("https://restapi.amap.com/v3/geocode/geo"));
                QUrlQuery query;
                query.addQueryItem("key",secret);
                query.addQueryItem("address",place);
                if(!city.trimmed().isEmpty())query.addQueryItem("city",city.trimmed());
                url.setQuery(query);
                requestJson(network_,this,url,[this,locationKey](const QByteArray& bytes,const QString& error) {
                    auto& result=locations_[locationKey];
                    result.requested=false;
                    result.error=error;
                    if(error.isEmpty())result.adcode=parseAdcode(bytes,&result.error);
                    result.retryAfter=QDateTime::currentDateTimeUtc().addSecs(result.adcode.isEmpty()?300:86400);
                });
            }
            return location.error.isEmpty()?tt::translate("正在识别天气地区…"):
                   tt::translate("天气地区识别失败：")+location.error;
        }
    }
    static const QRegularExpression validCode(QStringLiteral("^\\d{6}$"));
    if(!validCode.match(code).hasMatch())return tt::translate("行政区划代码应为 6 位数字");
    // Secrets are used only in private cache keys and requests, never in diagnostics.
    const QString key=secret+"|"+code+(forecast?"|forecast|"+QDate::currentDate().toString(Qt::ISODate):"|live");
    if(!cache_.contains(key)&&cache_.size()>=32) {
        for(auto it=cache_.begin();it!=cache_.end();) {
            if(!it->requested)it=cache_.erase(it);else ++it;
        }
    }
    auto& item=cache_[key];
    if(item.requested||(item.retryAfter.isValid()&&item.retryAfter>now))
        return displayResult(item.display,item.stale,tt::translate("正在获取天气…"));
    item.requested=true;
    QUrl url(QStringLiteral("https://restapi.amap.com/v3/weather/weatherInfo"));
    QUrlQuery query;
    query.addQueryItem("key",secret);
    query.addQueryItem("city",code);
    query.addQueryItem("extensions",forecast?"all":"base");
    url.setQuery(query);
    requestJson(network_,this,url,[this,key,city,forecast](const QByteArray& bytes,const QString& networkError) {
        auto& result=cache_[key];
        result.requested=false;
        QString error=networkError;
        QString display;
        if(error.isEmpty())display=parseWeather(bytes,city,forecast,QDate::currentDate(),&error);
        const bool success=!display.isEmpty();
        result.retryAfter=QDateTime::currentDateTimeUtc().addSecs(success?3600:300);
        if(success) { result.display=display;result.stale=false; }
        else if(!result.display.isEmpty()&&!result.display.startsWith(tt::translate("天气获取失败：")))
            result.stale=true;
        else { result.display=tt::translate("天气获取失败：")+error;result.stale=false; }
    });
    return displayResult(item.display,item.stale,tt::translate("正在获取天气…"));
}
}
