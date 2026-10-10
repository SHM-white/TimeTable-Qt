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
    QString type = "clock";           // clock/current/next/nextTime/countdown/duty/note/today/progress/week
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
    QJsonObject toJson() const;
    static TextElement fromJson(const QJsonObject& object);
};
struct ElementDefinition {
    QRect rect{8,8,170,27};          // coordinates local to the floating card
    QString mode = "single";          // single, cycle, all
    QList<TextElement> items;
    int intervalSeconds = 6;
    bool marquee = true;
    QJsonObject toJson() const;
    static ElementDefinition fromJson(const QJsonObject& object);
};
class ElementLayout {
public:
    static QJsonArray toJson(const QList<ElementDefinition>& elements);
    static QList<ElementDefinition> fromJson(const QJsonArray& array);
    static QList<ElementDefinition> fromLegacy(const QJsonArray& oldUiElements,int* skipped = nullptr);
    static QString evaluate(const TextElement& item,const ScheduleStore& store,
                            const QDateTime& now);
    static QString countdown(const QDateTime& target,const QDateTime& now,
                             const QString& pattern);
    static void paint(QPainter& painter,const QRect& available,
                      const QList<ElementDefinition>& elements,const ScheduleStore& store,
                      const QDateTime& now,qint64 elapsedMs,bool dark);
    static QRect contentBounds(const QList<ElementDefinition>& elements);
};
}
