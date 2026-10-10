#include "ScheduleStore.h"
#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>
#include <QDebug>
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
    if(!failed) qInfo()<<"All schedule tests passed";
    return failed?1:0;
}
