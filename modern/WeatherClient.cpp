#include "WeatherClient.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
namespace tt {
WeatherClient& WeatherClient::instance() {
    static WeatherClient singleton;
    return singleton;
}
WeatherClient::WeatherClient():QObject(nullptr) {}
QString WeatherClient::text(const QString& apiKey,const QString& adcode,const QString& city) {
    if(apiKey.trimmed().isEmpty()||adcode.trimmed().isEmpty())
        return QStringLiteral("天气未配置：请填写高德 API Key 和行政区划代码");
    // Cache is keyed by the secret, but keys never appear in logs or the UI.
    const QString key=apiKey+"|"+adcode;
    auto& item=cache_[key];
    const auto now=QDateTime::currentDateTimeUtc();
    if(item.requested || (item.retryAfter.isValid()&&item.retryAfter>now))
        return item.display.isEmpty()?QStringLiteral("正在获取天气…"):item.display;
    if(cache_.size()>32)cache_.clear();  // Prevent unbounded cache growth.
    auto& entry=cache_[key];
    entry.requested=true;
    QUrl url("https://restapi.amap.com/v3/weather/weatherInfo");
    QUrlQuery query;
    query.addQueryItem("key",apiKey);
    query.addQueryItem("city",adcode);
    query.addQueryItem("extensions","base");
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("Accept","application/json");
    auto* reply=network_.get(request);
    QObject::connect(reply,&QNetworkReply::finished,this,[this,key,city,reply] {
        auto& result=cache_[key];
        result.requested=false;
        result.retryAfter=QDateTime::currentDateTimeUtc().addSecs(5*60);
        if(reply->error()==QNetworkReply::NoError) {
            const auto doc=QJsonDocument::fromJson(reply->readAll());
            const auto root=doc.object();
            const auto rows=root.value("lives").toArray();
            if(root.value("status").toString()=="1" && !rows.isEmpty()) {
                const auto info=rows.first().toObject();
                const QString weather=info.value("weather").toString();
                const QString temperature=info.value("temperature").toString();
                const QString label=info.value("city").toString(city);
                if(!weather.isEmpty()) {
                    result.display=label+"  "+weather+"  "+temperature+QStringLiteral("℃");
                    result.retryAfter=QDateTime::currentDateTimeUtc().addSecs(60*60);
                }
            }
        }
        if(result.display.isEmpty()) result.display=QStringLiteral("天气获取失败：检查网络和高德 API Key");
        reply->deleteLater();
    });
    return entry.display.isEmpty()?QStringLiteral("正在获取天气…"):entry.display;
}
}
