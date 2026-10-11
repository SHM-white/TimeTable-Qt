#pragma once

#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QTime>
#include <QVector>

namespace tt {

struct Course {
    QString id;
    QString name;
    QString teacher;
    QString room;
    int weekday = 1;                // Qt: Monday = 1 ... Sunday = 7
    QTime start;
    QTime end;
    QVector<int> weeks;              // Explicit active weeks, no parity ambiguity
    QString color = "#6C6AF6";
};

struct DayChange {
    QDate date;
    bool holiday = true;            // true: no courses; false: use another date's lessons
    QDate sourceDate;
};

struct CourseConflict {
    int firstIndex = -1;
    int secondIndex = -1;
    QVector<int> weeks;
};

struct CourseReminder {
    Course course;
    QDateTime startsAt;
    QString key() const;
};

class ScheduleStore {
public:
    enum class ImportMode { Replace, Append };
    QDate semesterMonday;
    int totalWeeks = 20;
    QList<Course> courses;
    QList<DayChange> changes;
    QMap<int, QStringList> dayInfos; // Legacy Infos, Monday=1..Sunday=7
    QMap<int, QPair<QTime,QTime>> periods;

    ScheduleStore();

    static QString defaultPath();
    static QDate mondayOf(QDate date);
    static QString formatWeeks(const QVector<int>& weeks);
    static QVector<int> parseWeeks(const QString& text, int maxWeek, QString* error = nullptr);
    static bool validCourse(const Course& course, QString* error = nullptr);
    int weekNumber(QDate date) const;
    QList<Course> coursesForDate(QDate date) const;
    QList<Course> coursesForWeek(int week) const;
    const Course* currentCourse(const QDateTime& now) const;
    bool nextCourse(const QDateTime& now, Course* course, QDateTime* start) const;
    QList<CourseConflict> conflicts() const;
    QList<CourseReminder> remindersDue(const QDateTime& now, int minutesBefore,
                                      int graceSeconds = 60) const;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& root, QString* error = nullptr);
    bool load(const QString& path, QString* error = nullptr);
    bool save(const QString& path, QString* error = nullptr) const;
    bool importFile(const QString& path, QString* error = nullptr);
    bool importFile(const QString& path, ImportMode mode, QString* error = nullptr);
    bool exportCsv(const QString& path, QString* error = nullptr) const;
    bool exportIcs(const QString& path, QString* error = nullptr) const;
    bool exportXiaoAiJson(const QString& path, QString* error = nullptr) const;

private:
    bool importCsv(const QByteArray& bytes, QString* error);
    bool importLegacy(const QJsonObject& root, QString* error);
    bool importXiaoAi(const QJsonObject& root, QString* error);
};

} // namespace tt
