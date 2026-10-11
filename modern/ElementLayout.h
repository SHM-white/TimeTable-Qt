#pragma once
#include "ScheduleStore.h"
#include <QColor>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QPainter>
#include <QRect>
#include <QString>
namespace tt {
struct TextElement {
    QString type = "clock";           // text types plus progressBar/analogClock/weatherBlock
    QString format;                   // time strftime / countdown positional placeholders / textual prefix
    QString font = "Microsoft YaHei UI";
    QColor color = QColor("#233044");
    int pointSize = 11;
    QDateTime target;
    bool bold = false;
    bool vertical = false;          // today: vertical list or horizontal strip
    bool abbreviate = false;        // today: legacy first-character display
    QString weatherApiKey;
    QString weatherAdcode;
    QString weatherCity;
    QString weatherAddress;
    bool weatherForecast = false;
    QString sourceUrl;
    QString sourceFile;
    bool rotateDuty = false;         // false preserves the joined daily-information view
    QJsonObject toJson() const;
    static TextElement fromJson(const QJsonObject& object);
};
struct ElementDefinition {
    QRect rect{8,8,170,27};          // coordinates local to the floating card
    QString mode = "single";          // single, cycle, all
    QList<TextElement> items;
    int intervalSeconds = 6;
    bool marquee = true;
    bool cycleAfterScroll = false;
    qreal scrollPixelsPerSecond = 1000.0 / 35.0;
    QString allLayout = "vertical";   // vertical, horizontal, grid
    int columns = 2;
    QString groupId;
    bool autoSize = false;
    QJsonObject toJson() const;
    static ElementDefinition fromJson(const QJsonObject& object);
};
class ElementLayout {
public:
    static constexpr int AnimationIntervalMs = 16;
    static QJsonArray toJson(const QList<ElementDefinition>& elements);
    static QList<ElementDefinition> fromJson(const QJsonArray& array);
    static QList<ElementDefinition> fromLegacy(const QJsonArray& oldUiElements,int* skipped = nullptr,
                                               qreal legacyScrollSpeed = 35.0);
    static QString evaluate(const TextElement& item,const ScheduleStore& store,
                            const QDateTime& now,qint64 elapsedMs = 0);
    static QString countdown(const QDateTime& target,const QDateTime& now,
                             const QString& pattern);
    static void paint(QPainter& painter,const QRect& available,
                      const QList<ElementDefinition>& elements,const ScheduleStore& store,
                      const QDateTime& now,qint64 elapsedMs,bool dark);
    static bool hasScrollingText(const QRect& available,
                                 const QList<ElementDefinition>& elements,
                                 const ScheduleStore& store,const QDateTime& now,
                                 qint64 elapsedMs);
    static QRect contentBounds(const QList<ElementDefinition>& elements);
    static QRect contentBounds(const QList<ElementDefinition>& elements,const ScheduleStore& store,
                               const QDateTime& now);
    // 0 means no timed item change. Geometry and cycle durations match paint().
    static int nextChangeDelay(const QRect& available,const QList<ElementDefinition>& elements,
                               const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs);
};
}
