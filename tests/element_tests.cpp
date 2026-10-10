#include "ElementLayout.h"
#include <QApplication>
#include <QImage>
#include <QJsonDocument>
#include <QPainter>
#include <QDebug>
using namespace tt;
static int failures=0;
void check(bool condition,const char* name) {
    if(!condition){qCritical()<<"FAILED:"<<name;++failures;}
}
int main(int argc,char** argv) {
    QApplication app(argc,argv);
    QJsonArray items{
        QJsonObject{{"ElementType",1},{"Location",QJsonArray{22,9}},
            {"Size",QJsonArray{230,33}},
            {"Data",QJsonArray{
                QJsonObject{{"Type",1},{"TextFormat","%H:%M:%S"},{"Font","SimSun"},
                    {"TextColor",QJsonArray{128,64,32,200}},{"TextSize",18}},
                QJsonObject{{"Type",5},{"TextFormat","距离考试 %03d天 %02d小时"},
                    {"TextSize",12},{"Data",QJsonObject{{"TargetTime",QJsonArray{2026,12,21,8,0,0}}}}}
            }}}};
    int skipped=0;
    const auto restored=ElementLayout::fromLegacy(items,&skipped);
    check(restored.size()==1&&skipped==0,"legacy element restored");
    check(restored.first().mode=="cycle","legacy cycle preserved");
    check(restored.first().rect==QRect(22,9,230,33),"legacy precise rectangle");
    check(restored.first().items.size()==2,"legacy cycling options");
    check(restored.first().items.first().font=="SimSun","legacy font");
    check(restored.first().items.first().color==QColor(128,64,32,200),"legacy RGBA");
    check(restored.first().items.first().pointSize==18,"legacy point size");
    check(restored.first().items.last().target.date()==QDate(2026,12,21),"legacy countdown date");
    const auto loaded=ElementLayout::fromJson(ElementLayout::toJson(restored));
    check(loaded.size()==1&&loaded.first().items.size()==2,"layout JSON roundtrip");
    check(loaded.first().items.first().color.alpha()==200,"alpha preserved");
    check(loaded.first().items.last().target.isValid(),"target preserved");
    const QDateTime now(QDate(2026,12,20),QTime(6,0));
    const QDateTime target(QDate(2026,12,21),QTime(8,0));
    check(ElementLayout::countdown(target,now,"距离考试%03d天%02d小时")==
          QStringLiteral("距离考试001天02小时"),"countdown placeholders are safe");
    check(ElementLayout::countdown(target,now,"%s %n %03d")==
          QStringLiteral("%s %n 001"),"unsupported printf controls are printed literally");
    ScheduleStore store;
    store.semesterMonday=QDate(2026,12,14);
    store.totalWeeks=4;
    store.courses.clear();
    Course course;course.name="Algorithms";course.weekday=7;
    course.start=QTime(6,0);course.end=QTime(7,0);course.weeks={1};
    store.courses<<course;
    TextElement text;text.type="current";text.format="正在：{value}";
    check(ElementLayout::evaluate(text,store,now)==QStringLiteral("正在：Algorithms"),
          "explicit lesson template");
    QImage canvas(300,120,QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    ElementLayout::paint(painter,QRect(0,0,300,120),restored,store,now,2200,false);
    painter.end();
    check(!canvas.isNull(),"offscreen painter");
    QJsonArray broken{QJsonObject{{"ElementType",0},
         {"Data",QJsonArray{QJsonObject{{"Type",9}}}}}};
    skipped=0;
    check(ElementLayout::fromLegacy(broken,&skipped).isEmpty()&&skipped==1,
          "unsupported weather item transparently skipped");
    if(!failures)qInfo()<<"Element migration tests passed";
    return failures?1:0;
}
