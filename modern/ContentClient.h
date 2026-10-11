#pragma once
#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>
#include <QStringList>
namespace tt {
// Configured HTTPS feeds or local UTF-8 files; no default background networking.
class ContentClient final : public QObject {
public:
    static ContentClient& instance();
    // kind is "news" or "dailyWord". A configured local file takes precedence.
    QString text(const QString& kind,const QString& sourceUrl,const QString& sourceFile,
                 const QDateTime& displayNow=QDateTime::currentDateTime());
    static QStringList parseItems(const QByteArray& bytes,const QString& kind,QString* error=nullptr);
    static QString selectText(const QStringList& items,const QString& kind,
                              const QDate& date=QDate::currentDate(),int rotation=0);
private:
    ContentClient();
    struct Result {
        QStringList items;
        QString error;
        QDateTime retryAfter;
        QDateTime fileModified;
        qint64 fileSize=-1;
        bool requested=false;
    };
    QNetworkAccessManager network_;
    QHash<QString,Result> cache_;
};
}
