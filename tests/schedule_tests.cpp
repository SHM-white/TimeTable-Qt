#include "ScheduleStore.h"
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QDebug>
#include <QJsonArray>
#include <cstdio>

using namespace tt;
static int failed=0;
static void check(bool condition,const char* message) {
    if(!condition) {
        std::fprintf(stderr,"FAIL: %s\n",message);
        std::fflush(stderr);
        qCritical()<<"FAIL:"<<message;
        ++failed;
    }
}
int main(int argc,char** argv) {
    QCoreApplication app(argc,argv);
    QString error;
    auto odd=ScheduleStore::parseWeeks("1-8单",20,&error);
    check(odd==QVector<int>({1,3,5,7}),"odd weeks");
    check(ScheduleStore::parseWeeks("2-8双,9",20)==QVector<int>({2,4,6,8,9}),"even and explicit weeks");
    check(ScheduleStore::parseWeeks("All",4)==QVector<int>({1,2,3,4}),"English all alias");
    check(ScheduleStore::parseWeeks("1-8odd",20)==QVector<int>({1,3,5,7})&&
          ScheduleStore::parseWeeks("2-8 EVEN,9",20)==QVector<int>({2,4,6,8,9}),"English odd/even aliases with ranges");
    check(ScheduleStore::parseWeeks("odd",5)==QVector<int>({1,3,5})&&ScheduleStore::parseWeeks("even",5)==QVector<int>({2,4}),
          "English parity aliases cover whole term");
    check(ScheduleStore::parseWeeks("1-99",20,&error).isEmpty()&&!error.isEmpty(),"invalid range");
    ScheduleStore s;
    s.semesterMonday=QDate(2026,9,7);s.totalWeeks=20;s.courses.clear();
    Course course;
    course.id="demo";course.name="Algorithms";course.weekday=1;
    course.start=QTime(8,0);course.end=QTime(9,40);
    course.weeks={1,3,5,7};
    s.courses<<course;
    s.dayInfos[1]=QStringList{QStringLiteral("早班值日"),QStringLiteral("值班 A 组")};
    check(s.weekNumber(QDate(2026,9,7))==1,"week 1 start");
    check(s.weekNumber(QDate(2026,9,14))==2,"week 2 start");
    check(s.coursesForDate(QDate(2026,9,7)).size()==1,"odd week active");
    check(s.coursesForDate(QDate(2026,9,14)).isEmpty(),"even week inactive");
    check(s.currentCourse(QDateTime(QDate(2026,9,7),QTime(8,0)))!=nullptr,"start inclusive");
    check(s.currentCourse(QDateTime(QDate(2026,9,7),QTime(9,40)))==nullptr,"end exclusive");
    DayChange holiday;holiday.date=QDate(2026,9,7);
    s.changes<<holiday;
    check(s.coursesForDate(holiday.date).isEmpty(),"holiday suppression");
    s.changes.clear();
    DayChange makeup;makeup.date=QDate(2026,9,12);makeup.holiday=false;makeup.sourceDate=QDate(2026,9,7);
    s.changes<<makeup;
    check(s.coursesForDate(makeup.date).size()==1,"makeup day source");
    Course next;QDateTime nextTime;
    check(s.nextCourse(QDateTime(QDate(2026,9,7),QTime(7,0)),&next,&nextTime),"next course exists");
    check(nextTime==QDateTime(QDate(2026,9,7),QTime(8,0)),"next start");
    QTemporaryDir temp;
    check(temp.isValid(),"temp directory");
    const QString json=temp.path()+"/schedule.json";
    check(s.save(json,&error),"save JSON");
    ScheduleStore loaded;
    check(loaded.load(json,&error),"load JSON");
    check(loaded.courses.size()==1&&loaded.changes.size()==1&&loaded.totalWeeks==20,"round trip");
    check(loaded.dayInfos.value(1)==s.dayInfos.value(1),"daily Infos survive round-trip");
    const QJsonObject beforeMalformed=loaded.toJson();
    for(const QString& field:QStringList{"courses","dayInfos","dayChanges","periods"}) {
        QJsonObject damagedRoot=beforeMalformed; damagedRoot[field]="damaged";
        check(!loaded.fromJson(damagedRoot,&error)&&loaded.toJson()==beforeMalformed,"invalid array type rejected without mutation");
    }
    QJsonObject missingCourses=beforeMalformed; missingCourses.remove("courses");
    check(!loaded.fromJson(missingCourses,&error)&&loaded.toJson()==beforeMalformed,"missing required courses rejected without mutation");
    QJsonObject malformedWeek=beforeMalformed;
    QJsonArray badCourses=malformedWeek.value("courses").toArray();
    QJsonObject badCourse=badCourses.first().toObject(); badCourse["weeks"]=QJsonArray{1,"bad"}; badCourses[0]=badCourse; malformedWeek["courses"]=badCourses;
    check(!loaded.fromJson(malformedWeek,&error)&&loaded.toJson()==beforeMalformed,"mixed malformed weeks do not silently drop data");
    const QString csv=temp.path()+"/table.csv";
    check(s.exportCsv(csv,&error),"CSV export");
    ScheduleStore fromCsv;fromCsv.totalWeeks=20;
    check(fromCsv.importFile(csv,&error)&&fromCsv.courses.size()==1,"CSV reimport");
    check(fromCsv.courses.first().weeks==course.weeks,"CSV weeks");
    const QString ics=temp.path()+"/table.ics";
    check(s.exportIcs(ics,&error),"ICS export");
    QFile file(ics);check(file.open(QIODevice::ReadOnly),"ICS readable");
    check(file.readAll().contains("BEGIN:VEVENT"),"ICS has events");
    QFile legacy(temp.path()+"/legacy.json");
    check(legacy.open(QIODevice::WriteOnly),"legacy writable");
    legacy.write(R"({"Mon":{"Lessons":[["Math","0800","0940"],["未录入","1000","1100"]],"Infos":["值日 A 组","请录入"]}})");
    legacy.close();
    ScheduleStore old;
    check(old.importFile(legacy.fileName(),&error),"legacy import");
    check(old.courses.size()==1&&old.courses.first().name=="Math","placeholder skipped");
    check(old.dayInfos.value(1)==QStringList({QStringLiteral("值日 A 组")}),"legacy Infos preserved");
    QFile xiaomi(temp.path()+"/xiaomi.json");
    check(xiaomi.open(QIODevice::WriteOnly),"XiaoAi sample writable");
    xiaomi.write(R"({"data":{"courseInfos":[{"name":"AI","day":1,"sections":[{"section":1},{"section":2}],"weeks":[1,3,5],"teacher":"Professor","position":"Room"}]}})");
    xiaomi.close();
    ScheduleStore xs;xs.totalWeeks=20;
    check(xs.importFile(xiaomi.fileName(),&error),"XiaoAi courseInfos import");
    check(xs.courses.size()==1&&xs.courses.first().end==QTime(9,40),"section mapping");
    check(xs.exportXiaoAiJson(temp.path()+"/xiaomi-out.json",&error),"XiaoAi export");

    // CSV quoting is record based: embedded commas, quotes and newlines round-trip.
    ScheduleStore multiline=s; multiline.changes.clear();
    multiline.courses[0].name="Lab, \"advanced\"\nProject";
    multiline.courses[0].teacher="Teacher\nAssistant";
    check(multiline.exportCsv(temp.path()+"/multiline.csv",&error),"multiline CSV export");
    ScheduleStore multilineLoaded=s;
    check(multilineLoaded.importFile(temp.path()+"/multiline.csv",&error),"multiline CSV import");
    check(multilineLoaded.courses.first().name==multiline.courses.first().name&&
          multilineLoaded.courses.first().teacher==multiline.courses.first().teacher,"CSV embedded quoted newlines preserved");
    QFile badCsv(temp.path()+"/bad.csv");
    check(badCsv.open(QIODevice::WriteOnly),"invalid CSV fixture writable");
    badCsv.write("name,weekday,start,end\n\"unterminated,1,08:00,09:00\n"); badCsv.close();
    const QJsonObject beforeBadCsv=multilineLoaded.toJson();
    check(!multilineLoaded.importFile(badCsv.fileName(),&error)&&multilineLoaded.toJson()==beforeBadCsv,
          "malformed CSV leaves active model unchanged");

    ScheduleStore merge=s; merge.changes.clear();
    DayChange existingHoliday; existingHoliday.date=QDate(2026,9,10); merge.changes<<existingHoliday;
    ScheduleStore addition=merge;
    addition.courses.first().id="different-id"; // Same content still deduplicates.
    Course extra=course; extra.id=course.id; extra.name="New seminar"; extra.start=QTime(10,0); extra.end=QTime(11,0);
    addition.courses<<extra;
    addition.dayInfos[1]<<QStringLiteral("新增附加信息");
    addition.changes[0].holiday=false; addition.changes[0].sourceDate=QDate(2026,9,7);
    DayChange newHoliday; newHoliday.date=QDate(2026,9,11); addition.changes<<newHoliday;
    check(addition.save(temp.path()+"/addition.json",&error),"append fixture saved");
    check(merge.importFile(temp.path()+"/addition.json",ScheduleStore::ImportMode::Append,&error),"native JSON append");
    check(merge.courses.size()==2&&merge.courses.at(0).id!=merge.courses.at(1).id,"append deduplicates contents and resolves ID collision");
    check(merge.changes.size()==2&&merge.changes.first().holiday,"append preserves existing day changes");
    check(merge.dayInfos.value(1).size()==3&&merge.dayInfos.value(1).contains(QStringLiteral("新增附加信息")),
          "append preserves and deduplicates daily Infos");
    check(merge.importFile(temp.path()+"/addition.json",ScheduleStore::ImportMode::Append,&error)&&merge.courses.size()==2,
          "repeated append is idempotent");
    check(merge.importFile(csv,ScheduleStore::ImportMode::Append,&error)&&merge.courses.size()==2&&merge.changes.size()==2&&merge.dayInfos.value(1).size()==3,
          "CSV append keeps courses Infos and holidays");
    ScheduleStore anotherTerm=addition; anotherTerm.semesterMonday=QDate(2027,2,22);
    check(anotherTerm.save(temp.path()+"/other-term.json",&error),"other term fixture saved");
    const QJsonObject beforeRejectedMerge=merge.toJson();
    check(!merge.importFile(temp.path()+"/other-term.json",ScheduleStore::ImportMode::Append,&error)&&merge.toJson()==beforeRejectedMerge,
          "append with incompatible semester fails transactionally");

    ScheduleStore overlap=s; overlap.changes.clear(); overlap.courses.clear();
    Course first=course; first.weeks={1,3}; overlap.courses<<first;
    Course second=first; second.id="second"; second.name="Overlapping"; second.start=QTime(9,0); second.end=QTime(10,0); second.weeks={2,3};
    overlap.courses<<second;
    Course touching=first; touching.id="touching"; touching.start=QTime(10,0); touching.end=QTime(11,0); overlap.courses<<touching;
    Course otherWeek=first; otherWeek.id="other-week"; otherWeek.weeks={4}; overlap.courses<<otherWeek;
    check(overlap.conflicts().size()==1&&overlap.conflicts().first().weeks==QVector<int>{3},
          "conflicts require intersecting weeks and strict time overlap; touching edges allowed");

    ScheduleStore reminder=s; reminder.changes.clear();
    const QDateTime trigger(QDate(2026,9,7),QTime(7,50));
    check(reminder.remindersDue(trigger.addSecs(-1),10).isEmpty(),"reminder not early");
    check(reminder.remindersDue(trigger,10).size()==1,"reminder due at chosen lead time");
    check(reminder.remindersDue(trigger.addSecs(59),10).size()==1&&reminder.remindersDue(trigger.addSecs(60),10).isEmpty(),
          "reminder has bounded catch-up window");
    check(reminder.remindersDue(trigger,10).first().key()==reminder.remindersDue(trigger.addSecs(1),10).first().key(),
          "reminder has stable deduplication key");
    check(reminder.remindersDue(QDateTime(QDate(2026,9,14),QTime(7,50)),10).isEmpty(),"inactive teaching week has no reminder");
    reminder.changes<<holiday;
    check(reminder.remindersDue(trigger,10).isEmpty(),"holiday suppresses reminder");
    reminder.changes.clear(); reminder.changes<<makeup;
    const auto makeupReminders=reminder.remindersDue(QDateTime(QDate(2026,9,12),QTime(7,50)),10);
    check(makeupReminders.size()==1&&makeupReminders.first().startsAt.date()==QDate(2026,9,12),"makeup reminder uses actual date");
    reminder.changes.clear(); reminder.courses.first().start=QTime(0,5); reminder.courses.first().end=QTime(0,45);
    check(reminder.remindersDue(QDateTime(QDate(2026,9,6),QTime(23,55)),10).size()==1,"midnight lead reminder checks tomorrow");
    check(reminder.remindersDue(QDateTime(QDate(2026,9,7),QTime(0,5)),0).size()==1,"zero lead reminder at course start");
    if(!failed) qInfo()<<"All schedule tests passed";
    return failed?1:0;
}
