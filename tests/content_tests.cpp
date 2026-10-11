#include "ContentClient.h"
#include "WeatherClient.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTemporaryDir>
using namespace tt;
namespace {
int failures=0;
void check(bool condition,const char* name) {
    if(!condition) {qCritical()<<"FAILED:"<<name;++failures;}
}
}
int main(int argc,char** argv) {
    QCoreApplication app(argc,argv);
    QString error;
    check(WeatherClient::parseAdcode(R"({"status":"1","geocodes":[{"adcode":"110105"}]})",&error)=="110105"&&error.isEmpty(),
          "AMap geocoding selects valid administrative code");
    check(WeatherClient::parseAdcode(R"({"status":"1","geocodes":[]})",&error).isEmpty()&&!error.isEmpty(),
          "unmatched geocoding returns actionable failure");
    check(WeatherClient::parseAdcode(R"({"status":"0","info":"INVALID_USER_KEY"})",&error).isEmpty()&&!error.isEmpty(),
          "provider failures are not interpreted as locations");
    check(WeatherClient::parseAdcode(R"({"status":"1","geocodes":[{"adcode":"not-a-code"}]})",&error).isEmpty(),
          "invalid geocoding fields rejected");
    const auto live=WeatherClient::parseWeather(R"({"status":"1","lives":[{"city":"北京","weather":"晴","temperature":"18"}]})",{},false,QDate(2026,10,11),&error);
    check(live==QStringLiteral("北京  晴  18℃")&&error.isEmpty(),"AMap live weather parsing");
    const QByteArray forecast=R"({"status":"1","forecasts":[{"city":"北京","casts":[
        {"date":"2026-10-10","dayweather":"旧数据","daytemp":"99","nighttemp":"98"},
        {"date":"2026-10-11","dayweather":"晴","nightweather":"多云","daytemp":"20","nighttemp":"10"},
        {"date":"2026-10-12","dayweather":"阴","nightweather":"阴","daytemp":"18","nighttemp":"9"},
        {"date":"2026-10-13","dayweather":"后天","daytemp":"17","nighttemp":"8"}]}]})";
    const auto prediction=WeatherClient::parseWeather(forecast,{},true,QDate(2026,10,11),&error);
    check(prediction==QStringLiteral("北京  今天 晴 / 多云 10～20℃；明天 阴 9～18℃"),
          "forecast uses today and tomorrow and preserves night/day ranges");
    check(WeatherClient::parseWeather(forecast,{},true,QDate(2026,11,1),&error).isEmpty()&&!error.isEmpty(),
          "expired forecasts never masquerade as current weather");
    const auto rss=ContentClient::parseItems(R"(<?xml version="1.0" encoding="UTF-8"?><rss version="2.0"><channel>
        <title>Feed title should not be a headline</title>
        <item><title>First &amp; second</title></item><item><title>中文新闻</title></item>
        <item><title>First &amp; second</title></item></channel></rss>)","news",&error);
    check(rss==QStringList({"First & second",QStringLiteral("中文新闻")})&&error.isEmpty(),
          "RSS headlines omit feed title and deduplicate decoded titles");
    const auto atom=ContentClient::parseItems(R"(<feed xmlns="http://www.w3.org/2005/Atom"><title>Feed</title>
        <entry><title>Quote</title><content type="html">&lt;p&gt;Read slowly.&lt;/p&gt;</content>
        <author><name>Alice</name></author></entry></feed>)","dailyWord",&error);
    check(atom==QStringList({QStringLiteral("Read slowly. —— Alice")}),"Atom daily quote with HTML and author");
    check(ContentClient::parseItems(R"(<rss><channel><item><title>broken</rss>)","news",&error).isEmpty()&&!error.isEmpty(),
          "malformed feed rejected instead of partial silent success");
    check(ContentClient::parseItems(R"(<!DOCTYPE rss [<!ENTITY sample "expanded">]><rss><item><title>&sample;</title></item></rss>)","news",&error).isEmpty()&&!error.isEmpty(),
          "feed DTD rejected");
    const auto news=ContentClient::parseItems(R"({"title":"News feed","data":{"articles":[{"title":"One"},{"headline":"Two"}]}})","news",&error);
    check(news==QStringList({"One","Two"}),"nested JSON feed extracts actual headlines");
    const auto quote=ContentClient::parseItems(R"({"data":{"content":"Keep moving.","author":"Writer"}})","dailyWord",&error);
    check(quote==QStringList({QStringLiteral("Keep moving. —— Writer")}),"nested daily JSON content and author");
    check(ContentClient::parseItems(R"({"hitokoto":"一句话","from_who":"作者"})","dailyWord",&error)==
          QStringList{QStringLiteral("一句话 —— 作者")},"hitokoto JSON compatible");
    check(ContentClient::parseItems("{broken","news",&error).isEmpty()&&!error.isEmpty(),"malformed JSON is not treated as plain text");
    const auto lines=ContentClient::parseItems(QByteArray("\xEF\xBB\xBF")+QStringLiteral("第一句\r\n\n第二句\n").toUtf8(),"dailyWord",&error);
    check(lines==QStringList({QStringLiteral("第一句"),QStringLiteral("第二句")}),"UTF-8 BOM and CRLF local lines parsed");
    check(ContentClient::parseItems(QByteArray("\xFF\xFEinvalid"),"dailyWord",&error).isEmpty()&&!error.isEmpty(),
          "non-UTF-8 text reports encoding requirement");
    check(ContentClient::parseItems(QByteArray(1024*1024+1,'x'),"news",&error).isEmpty()&&!error.isEmpty(),
          "oversized content refused");
    const auto longItem=ContentClient::parseItems(QByteArray(2000,'a'),"dailyWord",&error);
    check(longItem.size()==1&&longItem.first().size()==800,"display text length bounded");
    const QStringList choices{"one","two","three"};
    check(ContentClient::selectText(choices,"dailyWord",QDate(2026,10,11),0)==
          ContentClient::selectText(choices,"dailyWord",QDate(2026,10,11),999),"daily quote stable throughout same date");
    check(ContentClient::selectText(choices,"dailyWord",QDate(2026,10,11))!=
          ContentClient::selectText(choices,"dailyWord",QDate(2026,10,12)),"daily quote advances by date");
    check(ContentClient::selectText(choices,"news",QDate(),4)=="two"&&
          ContentClient::selectText(choices,"news",QDate(),-1)=="three","news rotation safely wraps");
    auto& content=ContentClient::instance();
    check(content.text("news",{},{}).contains(QStringLiteral("未配置")),"unconfigured news remains offline");
    check(content.text("news","http://example.invalid/feed",{}).contains("HTTPS"),"insecure URL rejected before networking");
    QTemporaryDir directory;
    check(directory.isValid(),"local content test directory");
    const QString path=directory.path()+"/quotes.txt";
    QFile file(path);
    check(file.open(QIODevice::WriteOnly),"local quote fixture writable");
    file.write("A local daily quotation.");file.close();
    check(content.text("dailyWord",{},path)=="A local daily quotation.","local daily source works without network");
    check(content.text("dailyWord","https://example.invalid/feed",path)=="A local daily quotation.",
          "configured local file has precedence and suppresses URL requests");
    const QString newsPath=directory.path()+"/news.txt";
    QFile newsFile(newsPath);
    check(newsFile.open(QIODevice::WriteOnly),"local news fixture writable");
    newsFile.write("one\ntwo\nthree\n");newsFile.close();
    const QDateTime displayNow(QDate(2026,10,11),QTime(12,0),Qt::UTC);
    const int displayIndex=int((displayNow.toMSecsSinceEpoch()/15000)%choices.size());
    check(content.text("news",{},newsPath,displayNow)==choices.at(displayIndex),
          "content rendering respects supplied model time");
    check(content.text("news",{},newsPath,displayNow.addSecs(15))==choices.at((displayIndex+1)%choices.size()),
          "content news clock advances at supplied 15 second boundary");
    check(WeatherClient::instance().text({}, {}, {}).contains("API Key"),"unconfigured weather remains offline");
    if(!failures)qInfo()<<"All offline content and weather fixture tests passed";
    return failures?1:0;
}
