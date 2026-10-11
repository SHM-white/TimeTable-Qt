#include "ScheduleStore.h"
#include "Translation.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSet>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUuid>
#include <algorithm>

namespace tt {
namespace {
QString makeId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }
QTime parseTime(const QString& text) {
    QTime result = QTime::fromString(text.trimmed(), "HH:mm");
    if (result.isValid()) return result;
    const QString digits = text.trimmed();
    bool ok = false;
    int hhmm = digits.toInt(&ok);
    if (!ok || digits.size() > 4 || hhmm < 0) return {};
    return QTime(hhmm / 100, hhmm % 100);
}
QString timeText(QTime t) { return t.toString("HH:mm"); }
QJsonArray integerArray(const QVector<int>& v) {
    QJsonArray a; for (int w : v) a.append(w); return a;
}
QVector<int> readWeeks(const QJsonValue& v) {
    QVector<int> result;
    for (const auto& w : v.toArray()) if (w.isDouble()) {
        int n = w.toInt(); if (n > 0 && !result.contains(n)) result.append(n);
    }
    std::sort(result.begin(), result.end());
    return result;
}
QString quoteCsv(QString v) {
    v.replace('"', "\"\"");
    return (v.contains(',') || v.contains('\n') || v.contains('\r') || v.contains('"'))
        ? "\"" + v + "\"" : v;
}
bool csvRecords(const QString& raw, QList<QStringList>* records, QString* error) {
    QStringList cells; QString current; bool quoted=false, closedQuote=false;
    for(int i=0;i<raw.size();++i) {
        const QChar c=raw.at(i);
        if(quoted) {
            if(c=='"') {
                if(i+1<raw.size()&&raw.at(i+1)=='"') { current+='"'; ++i; }
                else { quoted=false; closedQuote=true; }
            } else if(c=='\r') {
                current+='\n';
                if(i+1<raw.size()&&raw.at(i+1)=='\n') ++i;
            } else current+=c;
        } else if(c==',' || c=='\r' || c=='\n') {
            cells<<current; current.clear(); closedQuote=false;
            if(c!=',') {
                if(c=='\r'&&i+1<raw.size()&&raw.at(i+1)=='\n') ++i;
                if(cells.size()>1 || !cells.first().trimmed().isEmpty()) records->append(cells);
                cells.clear();
            }
        } else if(c=='"' && current.isEmpty() && !closedQuote) quoted=true;
        else if(c=='"' || (closedQuote&&!c.isSpace())) {
            if(error) *error=tt::translate("CSV 引号格式无效");
            return false;
        } else if(!closedQuote) current+=c;
    }
    if(quoted) { if(error) *error=tt::translate("CSV 引号未闭合"); return false; }
    if(!current.isEmpty()||!cells.isEmpty()||closedQuote) { cells<<current; records->append(cells); }
    return true;
}
bool sameCourse(const Course& a,const Course& b) {
    auto aw=a.weeks,bw=b.weeks;
    std::sort(aw.begin(),aw.end()); std::sort(bw.begin(),bw.end());
    aw.erase(std::unique(aw.begin(),aw.end()),aw.end());
    bw.erase(std::unique(bw.begin(),bw.end()),bw.end());
    return a.name==b.name&&a.teacher==b.teacher&&a.room==b.room&&a.weekday==b.weekday&&
        a.start==b.start&&a.end==b.end&&aw==bw;
}
bool writeBytes(const QString& path, const QByteArray& content, QString* error) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = file.errorString();
        return false;
    }
    if (file.write(content) != content.size() || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}
QString icsEscape(QString s) {
    s.replace("\\", "\\\\").replace(";", "\\;").replace(",", "\\,");
    s.replace("\r\n", "\n").replace("\n", "\\n");
    return s;
}
QString icsStamp(const QDateTime& dt) { return dt.toUTC().toString("yyyyMMdd'T'HHmmss'Z'"); }
QString icsLocal(const QDateTime& dt) { return dt.toString("yyyyMMdd'T'HHmmss"); }
} // namespace

ScheduleStore::ScheduleStore() {
    semesterMonday = mondayOf(QDate::currentDate());
    const QVector<QPair<QString,QString>> times = {
        {"08:00","08:45"},{"08:55","09:40"},{"10:00","10:45"},
        {"10:55","11:40"},{"14:00","14:45"},{"14:55","15:40"},
        {"16:00","16:45"},{"16:55","17:40"},{"19:00","19:45"},
        {"19:55","20:40"},{"20:50","21:35"},{"21:45","22:30"}
    };
    for (int i=0;i<times.size();++i)
        periods.insert(i+1, {parseTime(times[i].first),parseTime(times[i].second)});
}

QString ScheduleStore::defaultPath() {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/schedule.json";
}
QDate ScheduleStore::mondayOf(QDate d) { return d.addDays(1-d.dayOfWeek()); }

int ScheduleStore::weekNumber(QDate d) const {
    if (!d.isValid() || !semesterMonday.isValid()) return 0;
    const int delta = semesterMonday.daysTo(d);
    if (delta < 0) return 0;
    const int n = delta/7 + 1;
    return n <= totalWeeks ? n : 0;
}

QString ScheduleStore::formatWeeks(const QVector<int>& source) {
    if (source.isEmpty()) return tt::translate("无");
    auto v = source;
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    QStringList parts;
    int start=v.first(), prev=start;
    for (int i=1;i<=v.size();++i) {
        if (i < v.size() && v.at(i) == prev+1) { prev=v.at(i); continue; }
        parts << (start==prev ? QString::number(start) : QString("%1-%2").arg(start).arg(prev));
        if (i < v.size()) start=prev=v.at(i);
    }
    return parts.join(",");
}

QVector<int> ScheduleStore::parseWeeks(const QString& text, int maxWeek, QString* error) {
    QVector<int> result;
    const QString normalized = text.trimmed().replace(QChar(0xFF0C), ',').replace(QChar(0x3001), ',');
    if (normalized == QStringLiteral("全部") || normalized == QStringLiteral("全") || normalized == "*" ||
        normalized.compare("all",Qt::CaseInsensitive)==0) {
        for(int w=1;w<=maxWeek;++w) result << w;
        return result;
    }
    if (normalized.isEmpty()) { if(error) *error=tt::translate("请输入周数，例如 1-16单、2-18双、1-8,10,12"); return {}; }
    for (QString part : normalized.split(',', Qt::SkipEmptyParts)) {
        part=part.trimmed();
        int parity=0;
        if (part.endsWith(QStringLiteral("单"))) { parity=1; part.chop(1); }
        else if (part.endsWith(QStringLiteral("双"))) { parity=2; part.chop(1); }
        else if(part.endsWith("odd",Qt::CaseInsensitive)) { parity=1; part.chop(3); }
        else if(part.endsWith("even",Qt::CaseInsensitive)) { parity=2; part.chop(4); }
        part=part.trimmed();
        if(part.isEmpty()&&parity) part=QString("1-%1").arg(maxWeek);
        const QStringList bounds = part.split('-');
        bool ok1=false,ok2=true;
        const int first=bounds.value(0).toInt(&ok1);
        const int last=bounds.size()==2 ? bounds.value(1).toInt(&ok2) : first;
        if (!ok1 || !ok2 || bounds.size()>2 || first<1 || last>maxWeek || first>last) {
            if (error) *error=tt::translate("周数格式无效：%1（有效范围 1-%2）").arg(part).arg(maxWeek);
            return {};
        }
        for (int w=first;w<=last;++w)
            if ((!parity || (parity==1 && w%2) || (parity==2 && w%2==0)) && !result.contains(w)) result << w;
    }
    std::sort(result.begin(),result.end());
    if (result.isEmpty() && error) *error=tt::translate("没有选中任何教学周");
    return result;
}

bool ScheduleStore::validCourse(const Course& c, QString* error) {
    if (c.name.trimmed().isEmpty() || c.weekday<1 || c.weekday>7 ||
        !c.start.isValid() || !c.end.isValid() || c.start>=c.end || c.weeks.isEmpty()) {
        if (error) *error=tt::translate("课程名称、星期、时间或教学周无效");
        return false;
    }
    for(int week:c.weeks) if(week<1) {
        if(error) *error=tt::translate("课程教学周必须大于零");
        return false;
    }
    return true;
}

QList<Course> ScheduleStore::coursesForDate(QDate date) const {
    QDate source = date;
    for (const auto& change: changes) {
        if (change.date != date) continue;
        if (change.holiday) return {};
        if (change.sourceDate.isValid()) source=change.sourceDate;
        break;
    }
    const int week=weekNumber(source);
    if (!week) return {};
    QList<Course> out;
    for (const auto& c:courses)
        if (c.weekday==source.dayOfWeek() && c.weeks.contains(week)) out << c;
    std::sort(out.begin(),out.end(),[](const Course& a,const Course& b){
        return a.start==b.start ? a.name<b.name : a.start<b.start;
    });
    return out;
}
QList<Course> ScheduleStore::coursesForWeek(int week) const {
    QList<Course> out;
    if (week<1 || week>totalWeeks) return out;
    const QDate start=semesterMonday.addDays((week-1)*7);
    for (int d=0;d<7;++d) out.append(coursesForDate(start.addDays(d)));
    return out;
}
const Course* ScheduleStore::currentCourse(const QDateTime& now) const {
    // Return a stable pointer into the owned list, rather than a temporary daily snapshot.
    QDate source=now.date();
    for (const auto& change:changes) if (change.date==now.date()) {
        if (change.holiday) return nullptr;
        if (change.sourceDate.isValid()) source=change.sourceDate;
        break;
    }
    const int week=weekNumber(source);
    for (const auto& c:courses)
        if (week && c.weekday==source.dayOfWeek() && c.weeks.contains(week) &&
            c.start<=now.time() && now.time()<c.end) return &c;
    return nullptr;
}
bool ScheduleStore::nextCourse(const QDateTime& now, Course* found, QDateTime* start) const {
    // Search through the semester, including sparse courses that meet once a month.
    // Return immediately outside the term to avoid rescanning hundreds of days per tick.
    const QDate lastDay=semesterMonday.addDays(totalWeeks*7-1);
    if (now.date()>lastDay) return false;
    const QDate firstDay=now.date()<semesterMonday ? semesterMonday : now.date();
    for (QDate d=firstDay; d<=lastDay; d=d.addDays(1)) {
        for (const auto& c:coursesForDate(d)) {
            const QDateTime when(d,c.start);
            if (when<=now) continue;
            if (found) *found=c;
            if (start) *start=when;
            return true;
        }
    }
    return false;
}

QList<CourseConflict> ScheduleStore::conflicts() const {
    QList<CourseConflict> result;
    for(int a=0;a<courses.size();++a) for(int b=a+1;b<courses.size();++b) {
        const auto& first=courses.at(a); const auto& second=courses.at(b);
        if(first.weekday!=second.weekday || first.end<=second.start || second.end<=first.start) continue;
        CourseConflict conflict; conflict.firstIndex=a; conflict.secondIndex=b;
        for(int week:first.weeks)
            if(second.weeks.contains(week)&&!conflict.weeks.contains(week)) conflict.weeks<<week;
        if(!conflict.weeks.isEmpty()) {
            std::sort(conflict.weeks.begin(),conflict.weeks.end());
            result<<conflict;
        }
    }
    return result;
}

QString CourseReminder::key() const {
    return course.id+"/"+startsAt.toString(Qt::ISODate);
}
QList<CourseReminder> ScheduleStore::remindersDue(const QDateTime& now,int minutesBefore,
                                                 int graceSeconds) const {
    QList<CourseReminder> result;
    if(!now.isValid()||minutesBefore<0||minutesBefore>1440||graceSeconds<1) return result;
    // Include tomorrow: a lesson shortly after midnight may be reminded today.
    for(int offset=0;offset<=1;++offset) {
        const QDate date=now.date().addDays(offset);
        for(const auto& course:coursesForDate(date)) {
            QDateTime start=now; start.setDate(date); start.setTime(course.start);
            const QDateTime reminder=start.addSecs(-minutesBefore*60);
            if(start.isValid()&&now>=reminder&&now<reminder.addSecs(graceSeconds)&&
               (minutesBefore==0||now<=start))
                result<<CourseReminder{course,start};
        }
    }
    return result;
}

QJsonObject ScheduleStore::toJson() const {
    QJsonArray classes;
    for(const auto& c:courses) {
        classes.append(QJsonObject{
            {"id",c.id},{"name",c.name},{"teacher",c.teacher},{"room",c.room},
            {"weekday",c.weekday},{"start",timeText(c.start)},{"end",timeText(c.end)},
            {"weeks",integerArray(c.weeks)},{"color",c.color}
        });
    }
    QJsonArray exceptions;
    for(const auto& d:changes)
        exceptions.append(QJsonObject{{"date",d.date.toString(Qt::ISODate)},
            {"type",d.holiday?"holiday":"use"},{"sourceDate",d.sourceDate.toString(Qt::ISODate)}});
    QJsonArray periodItems;
    for(auto i=periods.cbegin();i!=periods.cend();++i)
        periodItems.append(QJsonObject{{"section",i.key()},{"start",timeText(i.value().first)},
                {"end",timeText(i.value().second)}});
    QJsonArray infoDays;
    for(auto it=dayInfos.cbegin();it!=dayInfos.cend();++it) {
        QJsonArray messages;
        for(const auto& message:it.value()) messages.append(message);
        infoDays.append(QJsonObject{{"weekday",it.key()},{"messages",messages}});
    }
    return QJsonObject{{"schema","timetable-next/v1"},{"semesterStart",semesterMonday.toString(Qt::ISODate)},
        {"totalWeeks",totalWeeks},{"courses",classes},{"dayChanges",exceptions},
        {"periods",periodItems},{"dayInfos",infoDays}};
}

bool ScheduleStore::fromJson(const QJsonObject& root, QString* error) {
    if (root.value("schema").toString()!="timetable-next/v1") {
        if(error) *error=tt::translate("不是 TimeTable Next 的课表文件");
        return false;
    }
    const auto invalidField=[error](const QString& name) {
        if(error) *error=tt::translate("课表字段类型无效：%1").arg(name);
        return false;
    };
    if(!root.value("courses").isArray()) return invalidField("courses");
    for(const QString& name:QStringList{"dayInfos","dayChanges","periods"})
        if(root.contains(name)&&!root.value(name).isArray()) return invalidField(name);
    if(!root.value("semesterStart").isString()||!root.value("totalWeeks").isDouble())
        return invalidField("semesterStart / totalWeeks");
    ScheduleStore copy;
    const QDate start=QDate::fromString(root.value("semesterStart").toString(),Qt::ISODate);
    const int weeks=root.value("totalWeeks").toInt(0);
    if(!start.isValid() || weeks<1 || weeks>60) {
        if(error) *error=tt::translate("学期开始日期或总周数无效");
        return false;
    }
    copy.semesterMonday=mondayOf(start);
    copy.totalWeeks=weeks;
    copy.courses.clear(); copy.changes.clear(); copy.dayInfos.clear();
    QSet<QString> ids;
    for(const auto& info:root.value("dayInfos").toArray()) {
        if(!info.isObject()) return invalidField("dayInfos[]");
        const auto o=info.toObject();
        if(!o.value("weekday").isDouble()||!o.value("messages").isArray()) return invalidField("dayInfos[].weekday / messages");
        const int day=o.value("weekday").toInt(0);
        if(day<1||day>7) {
            if(error) *error=tt::translate("每日附加信息中的星期无效");
            return false;
        }
        QStringList messages;
        for(const auto& item:o.value("messages").toArray()) {
            if(!item.isString()) return invalidField("dayInfos[].messages[]");
            const QString message=item.toString().trimmed();
            if(!message.isEmpty()) messages<<message;
        }
        copy.dayInfos[day]=messages;
    }
    for(const auto& item:root.value("courses").toArray()) {
        if(!item.isObject()) return invalidField("courses[]");
        const QJsonObject o=item.toObject();
        if(!o.value("name").isString()||!o.value("weekday").isDouble()||
           !o.value("start").isString()||!o.value("end").isString()||!o.value("weeks").isArray())
            return invalidField("courses[].name / weekday / start / end / weeks");
        for(const QString& field:QStringList{"id","teacher","room","color"})
            if(o.contains(field)&&!o.value(field).isString()) return invalidField("courses[]."+field);
        for(const auto& week:o.value("weeks").toArray())
            if(!week.isDouble()||week.toDouble()<1||week.toDouble()!=week.toInt()) return invalidField("courses[].weeks[]");
        Course c;
        c.id=o.value("id").toString(makeId());
        if(c.id.isEmpty()||ids.contains(c.id)) c.id=makeId();
        ids.insert(c.id);
        c.name=o.value("name").toString().trimmed();
        c.teacher=o.value("teacher").toString();
        c.room=o.value("room").toString();
        c.weekday=o.value("weekday").toInt();
        c.start=parseTime(o.value("start").toString());
        c.end=parseTime(o.value("end").toString());
        c.weeks=readWeeks(o.value("weeks"));
        c.color=o.value("color").toString("#6C6AF6");
        if(!validCourse(c,error)) return false;
        for(int w:c.weeks) if(w>weeks) {
            if(error) *error=tt::translate("课程周数超出学期范围"); return false;
        }
        copy.courses << c;
    }
    for(const auto& item:root.value("dayChanges").toArray()) {
        if(!item.isObject()) return invalidField("dayChanges[]");
        const QJsonObject o=item.toObject();
        if(!o.value("date").isString()||!o.value("type").isString()||
           (o.value("type").toString()!="holiday"&&o.value("type").toString()!="use"))
            return invalidField("dayChanges[].date / type");
        DayChange d;
        d.date=QDate::fromString(o.value("date").toString(),Qt::ISODate);
        d.holiday=o.value("type").toString()=="holiday";
        d.sourceDate=QDate::fromString(o.value("sourceDate").toString(),Qt::ISODate);
        if(!d.date.isValid() || (!d.holiday && !d.sourceDate.isValid())) {
            if(error) *error=tt::translate("调休日期无效"); return false;
        }
        copy.changes << d;
    }
    const QJsonArray periodArray=root.value("periods").toArray();
    if(!periodArray.isEmpty()) {
        copy.periods.clear();
        for(const auto& item:periodArray) {
            if(!item.isObject()) return invalidField("periods[]");
            const QJsonObject o=item.toObject();
            if(!o.value("section").isDouble()||!o.value("start").isString()||!o.value("end").isString())
                return invalidField("periods[].section / start / end");
            const int index=o.value("section").toInt();
            const QTime from=parseTime(o.value("start").toString());
            const QTime to=parseTime(o.value("end").toString());
            if(index<1 || index>40 || !from.isValid() || !to.isValid() || from>=to) {
                if(error) *error=tt::translate("节次时间表无效"); return false;
            }
            copy.periods[index]={from,to};
        }
    }
    *this=copy;
    return true;
}

bool ScheduleStore::load(const QString& path, QString* error) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) { if(error) *error=file.errorString(); return false; }
    QJsonParseError parseError;
    const auto doc=QJsonDocument::fromJson(file.readAll(),&parseError);
    if(!doc.isObject()) {
        if(error) *error=parseError.errorString();
        return false;
    }
    return fromJson(doc.object(),error);
}
bool ScheduleStore::save(const QString& path, QString* error) const {
    return writeBytes(path,QJsonDocument(toJson()).toJson(QJsonDocument::Indented),error);
}

bool ScheduleStore::importLegacy(const QJsonObject& root, QString* error) {
    const QStringList days={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    QList<Course> imported;
    QMap<int,QStringList> oldInfo;
    bool hasValidDay=false;
    for(int day=0;day<days.size();++day) {
        const QJsonObject dayObject=root.value(days.at(day)).toObject();
        if(dayObject.isEmpty()) continue;
        hasValidDay=true;
        const QJsonArray messages=dayObject.value("Infos").toArray();
        for(const auto& item:messages) if(item.isString()) {
            const QString value=item.toString().trimmed();
            if(!value.isEmpty()&&value!=QStringLiteral("请录入")) oldInfo[day+1]<<value;
        }
        const auto lessons=dayObject.value("Lessons").toArray();
        for(const auto& item:lessons) {
            const QJsonArray data=item.toArray();
            if(data.size()<3) continue;
            Course c;
            c.id=makeId();
            c.name=data.at(0).toString().trimmed();
            if(c.name.isEmpty() || c.name==QStringLiteral("未录入")) continue;
            c.weekday=day+1;
            c.start=parseTime(data.at(1).toVariant().toString());
            c.end=parseTime(data.at(2).toVariant().toString());
            for(int w=1;w<=totalWeeks;++w) c.weeks<<w;
            if(!validCourse(c,error)) return false;
            imported<<c;
        }
    }
    if(!hasValidDay || (imported.isEmpty()&&oldInfo.isEmpty())) {
        if(error) *error=tt::translate("未找到有效旧版课程或每日附加信息；占位项已忽略");
        return false;
    }
    courses=imported;
    dayInfos=oldInfo;
    changes.clear();
    return true;
}
bool ScheduleStore::importXiaoAi(const QJsonObject& root, QString* error) {
    const QJsonArray data=root.value("courseInfos").toArray().isEmpty()
        ? root.value("data").toObject().value("courseInfos").toArray()
        : root.value("courseInfos").toArray();
    if(data.isEmpty()) { if(error) *error=tt::translate("找不到 courseInfos 课程数组"); return false; }
    QList<Course> imported;
    for(const auto& item:data) {
        const QJsonObject o=item.toObject();
        Course c; c.id=makeId();
        c.name=o.value("name").toString().trimmed();
        c.teacher=o.value("teacher").toString();
        c.room=o.value("position").toString();
        c.weekday=o.value("day").toInt();
        c.weeks=readWeeks(o.value("weeks"));
        const QJsonArray sections=o.value("sections").toArray();
        if(sections.isEmpty()) { if(error) *error=tt::translate("小爱课程缺少节次信息"); return false; }
        const int first=sections.first().toObject().value("section").toInt();
        const int last=sections.last().toObject().value("section").toInt();
        if(!periods.contains(first) || !periods.contains(last)) {
            if(error) *error=tt::translate("第 %1-%2 节尚未配置上课时间，请先到设置页调整").arg(first).arg(last);
            return false;
        }
        c.start=periods.value(first).first;
        c.end=periods.value(last).second;
        if(!validCourse(c,error)) return false;
        for(int w:c.weeks) if(w>totalWeeks) {
            if(error) *error=tt::translate("小爱课程周数大于学期总周数，请先调整设置");
            return false;
        }
        imported<<c;
    }
    courses=imported; changes.clear(); dayInfos.clear();
    return true;
}
bool ScheduleStore::importCsv(const QByteArray& bytes, QString* error) {
    QString raw=QString::fromUtf8(bytes);
    if(raw.startsWith(QChar(0xFEFF))) raw.remove(0,1);
    QList<QStringList> records;
    if(!csvRecords(raw,&records,error)) return false;
    QList<Course> imported;
    for(int i=0;i<records.size();++i) {
        const QStringList cells=records.at(i);
        if(cells.size()<4) {
            if(error) *error=tt::translate("CSV 第 %1 条记录少于 4 列").arg(i+1);
            return false;
        }
        if(i==0 && (cells[0].contains(QStringLiteral("星期")) ||
                     cells[0].contains(QStringLiteral("课程")) ||
                     cells[0].compare("day",Qt::CaseInsensitive)==0 ||
                     cells[0].compare("name",Qt::CaseInsensitive)==0)) continue;
        Course c; c.id=makeId();
        // Modern CSV: name,weekday,start,end,weeks,teacher,room
        // Old CSV: weekday,name,start,end (Mon/Tue/...).
        const QString first=cells[0].trimmed();
        const QStringList days={"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
        const int legacyDay=days.indexOf(first,0);
        if(legacyDay>=0) { c.weekday=legacyDay+1; c.name=cells[1]; }
        else { c.name=first; c.weekday=cells[1].trimmed().toInt(); }
        c.start=parseTime(cells[2]); c.end=parseTime(cells[3]);
        if(cells.size()>4 && !cells[4].trimmed().isEmpty()) {
            c.weeks=parseWeeks(cells[4],totalWeeks,error);
        } else for(int w=1;w<=totalWeeks;++w) c.weeks<<w;
        if(cells.size()>5) c.teacher=cells[5];
        if(cells.size()>6) c.room=cells[6];
        if(!validCourse(c,error)) {
            if(error) *error=tt::translate("CSV 第 %1 行：%2").arg(i+1).arg(*error);
            return false;
        }
        imported<<c;
    }
    if(imported.isEmpty()) { if(error) *error=tt::translate("CSV 中没有有效课程"); return false; }
    courses=imported; changes.clear(); dayInfos.clear(); return true;
}
bool ScheduleStore::importFile(const QString& path, QString* error) {
    return importFile(path,ImportMode::Replace,error);
}
bool ScheduleStore::importFile(const QString& path, ImportMode mode, QString* error) {
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) { if(error) *error=file.errorString(); return false; }
    const QByteArray bytes=file.readAll();
    ScheduleStore copy=*this;
    bool ok=false,native=false;
    if(path.endsWith(".csv",Qt::CaseInsensitive)) ok=copy.importCsv(bytes,error);
    else {
        QJsonParseError pe;
        const auto doc=QJsonDocument::fromJson(bytes,&pe);
        if(!doc.isObject()) { if(error) *error=pe.errorString(); return false; }
        const auto obj=doc.object();
        if(obj.value("schema").toString()=="timetable-next/v1") { native=true; ok=copy.fromJson(obj,error); }
        else if(obj.contains("data") || obj.contains("courseInfos")) ok=copy.importXiaoAi(obj,error);
        else ok=copy.importLegacy(obj,error);
    }
    if(!ok) return false;
    if(mode==ImportMode::Replace) { *this=copy; return true; }
    if(native&&semesterMonday!=copy.semesterMonday) {
        if(error) *error=tt::translate("追加的课表开学日期与当前学期不同，请改用替换或切换学期档案");
        return false;
    }
    ScheduleStore merged=*this;
    QSet<QString> ids;
    for(const auto& course:merged.courses) ids.insert(course.id);
    for(auto course:copy.courses) {
        for(int week:course.weeks) if(week>merged.totalWeeks) {
            if(error) *error=tt::translate("追加课程周数超出当前学期范围");
            return false;
        }
        bool duplicate=false;
        for(const auto& existing:merged.courses) if(sameCourse(existing,course)) { duplicate=true; break; }
        if(duplicate) continue;
        if(course.id.isEmpty()||ids.contains(course.id)) course.id=makeId();
        ids.insert(course.id); merged.courses<<course;
    }
    for(auto it=copy.dayInfos.cbegin();it!=copy.dayInfos.cend();++it)
        for(const auto& message:it.value())
            if(!merged.dayInfos[it.key()].contains(message)) merged.dayInfos[it.key()]<<message;
    for(const auto& change:copy.changes) {
        bool exists=false;
        for(const auto& existing:merged.changes) if(existing.date==change.date) { exists=true; break; }
        if(!exists) merged.changes<<change;
    }
    std::sort(merged.changes.begin(),merged.changes.end(),[](const DayChange& a,const DayChange& b){return a.date<b.date;});
    *this=merged;
    return true;
}
bool ScheduleStore::exportCsv(const QString& path, QString* error) const {
    QByteArray out=QByteArray::fromHex("EFBBBF");
    out+="name,weekday,start,end,weeks,teacher,room\n";
    for(const auto& c:courses)
        out += (QStringList{quoteCsv(c.name),QString::number(c.weekday),timeText(c.start),
                 timeText(c.end),quoteCsv(formatWeeks(c.weeks)),quoteCsv(c.teacher),
                 quoteCsv(c.room)}.join(",")+"\n").toUtf8();
    return writeBytes(path,out,error);
}
bool ScheduleStore::exportIcs(const QString& path, QString* error) const {
    QString data="BEGIN:VCALENDAR\r\nVERSION:2.0\r\nPRODID:-//TimeTableNext//Schedule//ZH\r\nCALSCALE:GREGORIAN\r\n";
    const QDateTime now=QDateTime::currentDateTimeUtc();
    for(int week=1;week<=totalWeeks;++week) {
        const QDate monday=semesterMonday.addDays((week-1)*7);
        for(int d=0;d<7;++d) {
            const QDate date=monday.addDays(d);
            for(const auto& c:coursesForDate(date)) {
                const QString dateStart=icsLocal(QDateTime(date,c.start));
                const QString dateEnd=icsLocal(QDateTime(date,c.end));
                data += QString("BEGIN:VEVENT\r\nUID:%1-%2@timetable-next\r\nDTSTAMP:%3\r\nDTSTART:%4\r\nDTEND:%5\r\nSUMMARY:%6\r\nLOCATION:%7\r\nDESCRIPTION:%8\r\nEND:VEVENT\r\n")
                    .arg(c.id,date.toString("yyyyMMdd"),icsStamp(now),dateStart,dateEnd,
                         icsEscape(c.name),icsEscape(c.room),icsEscape(c.teacher));
            }
        }
    }
    data+="END:VCALENDAR\r\n";
    return writeBytes(path,data.toUtf8(),error);
}
bool ScheduleStore::exportXiaoAiJson(const QString& path, QString* error) const {
    QJsonArray result;
    for(const auto& c:courses) {
        int first=0,last=0;
        for(auto it=periods.cbegin();it!=periods.cend();++it) {
            if(it.value().first==c.start) first=it.key();
            if(it.value().second==c.end) last=it.key();
        }
        if(!first || !last || first>last) {
            if(error) *error=tt::translate("课程「%1」时间与节次表不匹配，无法无损导出为小爱结构；请先调整节次时间").arg(c.name);
            return false;
        }
        QJsonArray sections;
        for(int n=first;n<=last;++n) sections.append(QJsonObject{{"section",n}});
        result.append(QJsonObject{{"name",c.name},{"day",c.weekday},{"weeks",integerArray(c.weeks)},
            {"sections",sections},{"teacher",c.teacher},{"position",c.room}});
    }
    return writeBytes(path,QJsonDocument(QJsonObject{{"data",QJsonObject{{"courseInfos",result}}}})
        .toJson(QJsonDocument::Indented),error);
}

} // namespace tt
