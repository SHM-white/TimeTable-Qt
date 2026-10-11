#pragma once
#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
namespace tt {
// Opt-in, read-only AMap provider. Rendering only starts asynchronous requests.
class WeatherClient final : public QObject {
public:
    static WeatherClient& instance();
    QString text(const QString& apiKey,const QString& adcode,const QString& city,
                 const QString& address=QString(),bool forecast=false);
    // These parsers do not access the network and can be used with saved fixtures.
    static QString parseAdcode(const QByteArray& bytes,QString* error=nullptr);
    static QString parseWeather(const QByteArray& bytes,const QString& city,bool forecast,
                                const QDate& today=QDate::currentDate(),QString* error=nullptr);
private:
    WeatherClient();
    struct Result {
        QString display;
        QDateTime retryAfter;
        bool requested=false;
        bool stale=false;
    };
    struct Location {
        QString adcode;
        QString error;
        QDateTime retryAfter;
        bool requested=false;
    };
    QNetworkAccessManager network_;
    QHash<QString,Result> cache_;
    QHash<QString,Location> locations_;
};
}
