#pragma once
#include <QDateTime>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
namespace tt {
// Opt-in, read-only AMap weather provider. Never synchronously blocks GUI rendering.
class WeatherClient final : public QObject {
public:
    static WeatherClient& instance();
    QString text(const QString& apiKey,const QString& adcode,const QString& city);
private:
    WeatherClient();
    struct Result {
        QString display;
        QDateTime retryAfter;
        bool requested = false;
    };
    QNetworkAccessManager network_;
    QHash<QString,Result> cache_;
};
}
