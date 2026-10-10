#include "ElementLayout.h"
#include <QFontMetrics>
#include <QJsonValue>
#include <QRegularExpression>
#include <algorithm>
#include <ctime>
namespace tt {
namespace {
QString dateFormat(const QDateTime& date,const QString& fmt) {
    const QByteArray pattern=(fmt.isEmpty()?"%a %H:%M:%S":fmt.toUtf8());
    if(pattern.size()>500) return QStringLiteral("时间格式过长");
    const std::time_t stamp=static_cast<std::time_t>(date.toSecsSinceEpoch());
    std::tm local{};
#ifdef Q_OS_WIN
    localtime_s(&local,&stamp);
#else
    localtime_r(&stamp,&local);
#endif
    char buffer[2048]{};
    if(std::strftime(buffer,sizeof(buffer),pattern.constData(),&local)==0) return {};
    return QString::fromUtf8(buffer);
}
QString readType(int number) {
    switch(number) {
    case 0:return "note"; case 1:return "clock"; case 2:return "current";
    case 3:return "next"; case 4:return "nextTime"; case 5:return "countdown";
    case 6:return "duty"; case 10:return "today"; case 11:return "progress";
    default:return {};
    }
}
QDateTime readTarget(const QJsonValue& value) {
    const QJsonArray a=value.toObject().value("TargetTime").toArray();
    if(a.size()<3) return {};
    const QDate d(a[0].toInt(),a[1].toInt(),a[2].toInt());
    const QTime t(a.size()>3?a[3].toInt():0,a.size()>4?a[4].toInt():0,a.size()>5?a[5].toInt():0);
    return (d.isValid()&&t.isValid())?QDateTime(d,t):QDateTime{};
}
QString clampString(QString value,int limit=2000) { if(value.size()>limit)value.truncate(limit);return value; }
} // namespace
QJsonObject TextElement::toJson() const {
    return {{"type",type},{"format",format},{"font",font},{"color",color.name(QColor::HexArgb)},
        {"pointSize",pointSize},{"bold",bold},{"target",target.toString(Qt::ISODate)}};
}
TextElement TextElement::fromJson(const QJsonObject& o) {
    TextElement item;
    item.type=o.value("type").toString("clock");
    item.format=o.value("format").toString();
    item.font=o.value("font").toString("Microsoft YaHei UI");
    const QColor color(o.value("color").toString("#233044"));
    if(color.isValid()) item.color=color;
    item.pointSize=std::clamp(o.value("pointSize").toInt(11),7,64);
    item.bold=o.value("bold").toBool(false);
    item.target=QDateTime::fromString(o.value("target").toString(),Qt::ISODate);
    return item;
}
QJsonObject ElementDefinition::toJson() const {
    QJsonArray array;
    for(const auto& e:items) array.append(e.toJson());
    return {{"x",rect.x()},{"y",rect.y()},{"width",rect.width()},{"height",rect.height()},
        {"mode",mode},{"intervalSeconds",intervalSeconds},{"marquee",marquee},{"items",array}};
}
ElementDefinition ElementDefinition::fromJson(const QJsonObject& o) {
    ElementDefinition e;
    e.rect=QRect(std::clamp(o.value("x").toInt(8),-2048,4096),
                 std::clamp(o.value("y").toInt(8),-2048,4096),
                 std::clamp(o.value("width").toInt(170),16,3000),
                 std::clamp(o.value("height").toInt(27),16,3000));
    e.mode=o.value("mode").toString("single");
    if(e.mode!="single"&&e.mode!="cycle"&&e.mode!="all") e.mode="single";
    e.intervalSeconds=std::clamp(o.value("intervalSeconds").toInt(6),1,600);
    e.marquee=o.value("marquee").toBool(true);
    for(const auto& v:o.value("items").toArray())
        if(v.isObject()&&e.items.size()<32) e.items << TextElement::fromJson(v.toObject());
    return e;
}
QJsonArray ElementLayout::toJson(const QList<ElementDefinition>& elements) {
    QJsonArray out;for(const auto& e:elements)out.append(e.toJson());return out;
}
QList<ElementDefinition> ElementLayout::fromJson(const QJsonArray& items) {
    QList<ElementDefinition> out;
    for(const auto& item:items) {
        if(!item.isObject()||out.size()>=64) break;
        ElementDefinition e=ElementDefinition::fromJson(item.toObject());
        if(!e.items.isEmpty())out<<e;
    }
    return out;
}
QList<ElementDefinition> ElementLayout::fromLegacy(const QJsonArray& array,int* skipped) {
    QList<ElementDefinition> out;
    for(const auto& old:array) {
        if(!old.isObject()||out.size()>=64){if(skipped)++*skipped;continue;}
        const auto obj=old.toObject();
        ElementDefinition e;
        const QJsonArray position=obj.value("Location").toArray();
        const QJsonArray size=obj.value("Size").toArray();
        if(position.size()>=2)e.rect.moveTopLeft(QPoint(position[0].toInt(),position[1].toInt()));
        if(size.size()>=2&&size[0].toInt()>0&&size[1].toInt()>0)
            e.rect.setSize(QSize(std::clamp(size[0].toInt(),16,3000),
                                 std::clamp(size[1].toInt(),16,3000)));
        const int mode=obj.value("ElementType").toInt();
        e.mode=mode==1?"cycle":mode==2?"all":"single";
        for(const auto& raw:obj.value("Data").toArray()) {
            if(!raw.isObject())continue;
            const auto value=raw.toObject();
            const QString type=readType(value.value("Type").toInt(-1));
            if(type.isEmpty()) {if(skipped)++*skipped;continue;}
            TextElement text;
            text.type=type;
            text.format=value.value("TextFormat").toString();
            text.font=value.value("Font").toString("Microsoft YaHei UI");
            text.pointSize=std::clamp(value.value("TextSize").toInt(11),7,64);
            const QJsonArray colors=value.value("TextColor").toArray();
            if(colors.size()>=3)text.color=QColor(
                std::clamp(colors[0].toInt(),0,255),std::clamp(colors[1].toInt(),0,255),
                std::clamp(colors[2].toInt(),0,255),colors.size()>3?std::clamp(colors[3].toInt(),0,255):255);
            text.target=readTarget(value.value("Data"));
            e.items<<text;
        }
        if(!e.items.isEmpty())out<<e;
    }
    return out;
}
QString ElementLayout::countdown(const QDateTime& target,const QDateTime& now,const QString& pattern) {
    if(!target.isValid())return QStringLiteral("请设置目标日期");
    const qint64 secs=std::max(qint64(0),now.secsTo(target));
    const qint64 nums[]={secs/86400,secs%86400/3600,secs%3600/60,secs%60};
    if(pattern.trimmed().isEmpty())
        return QStringLiteral("%1天 %2小时 %3分 %4秒").arg(nums[0]).arg(nums[1]).arg(nums[2]).arg(nums[3]);
    // Legacy printf-like %03d countdown format, without executing untrusted printf format.
    const QRegularExpression rx("%(0?)([1-9]?[0-9]?)d");
    QString out;int cursor=0,index=0;
    auto it=rx.globalMatch(pattern);
    while(it.hasNext()) {
        const auto m=it.next();
        out+=pattern.mid(cursor,m.capturedStart()-cursor);
        const int width=std::min(m.captured(2).toInt(),32);
        const auto number=nums[std::min(index++,3)];
        out+=width?QString("%1").arg(number,width,10,m.captured(1)=="0"?QLatin1Char('0'):QLatin1Char(' ')):QString::number(number);
        cursor=m.capturedEnd();
    }
    out+=pattern.mid(cursor);
    return clampString(out);
}
QString ElementLayout::evaluate(const TextElement& item,const ScheduleStore& store,const QDateTime& now) {
    const auto current=store.currentCourse(now);
    Course next;QDateTime start;
    const bool hasNext=store.nextCourse(now,&next,&start);
    QString result;
    if(item.type=="clock")return dateFormat(now,item.format);
    if(item.type=="note")return clampString(item.format);
    if(item.type=="countdown")return countdown(item.target,now,item.format);
    if(item.type=="current")result=current?current->name:QStringLiteral("无课程");
    else if(item.type=="next")result=hasNext?next.name:QStringLiteral("暂无课程");
    else if(item.type=="nextTime")result=hasNext?countdown(start,now,QString()):QStringLiteral("暂无课程");
    else if(item.type=="duty")result=store.dayInfos.value(now.date().dayOfWeek()).join(QStringLiteral(" · "));
    else if(item.type=="week")result=QStringLiteral("第 %1 周").arg(store.weekNumber(now.date()));
    else if(item.type=="progress") {
        if(current) {
            const int total=current->start.secsTo(current->end);
            const int elapsed=current->start.secsTo(now.time());
            result=total>0?QStringLiteral("%1%").arg(std::clamp(elapsed*100/total,0,100)):QString();
        } else result=QStringLiteral("未在上课");
    } else if(item.type=="today") {
        QStringList parts;
        for(const auto& course:store.coursesForDate(now.date()))
            parts<<course.start.toString("HH:mm")+" "+course.name;
        result=parts.join(QStringLiteral(" / "));
    }
    if(item.format.isEmpty())return result;
    // Prefix is the legacy behavior for lesson types. {value} offers an explicit template.
    return clampString(item.format.contains("{value}")?
        QString(item.format).replace("{value}",result):item.format+result);
}
QRect ElementLayout::contentBounds(const QList<ElementDefinition>& elements) {
    QRect bounds;
    for(const auto& e:elements)bounds=bounds.isNull()?e.rect:bounds.united(e.rect);
    return bounds;
}
void ElementLayout::paint(QPainter& p,const QRect& area,const QList<ElementDefinition>& elements,
                          const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs,bool dark) {
    for(const auto& e:elements) {
        if(e.items.isEmpty())continue;
        const QRect target=QRect(area.topLeft()+e.rect.topLeft(),e.rect.size()).intersected(area);
        if(target.isEmpty())continue;
        p.save();p.setClipRect(target);
        QList<TextElement> visible;
        if(e.mode=="all")visible=e.items;
        else if(e.mode=="cycle")visible<<e.items.at(int((elapsedMs/(1000LL*e.intervalSeconds))%e.items.size()));
        else visible<<e.items.first();
        int offset=0;
        for(const auto& item:visible) {
            QRect row=target;
            if(e.mode=="all") {
                const int rowHeight=std::max(1,int(target.height()/visible.size()));
                row=QRect(target.left(),target.top()+offset*rowHeight,target.width(),rowHeight);
                ++offset;
            }
            QFont f(item.font,item.pointSize);
            f.setBold(item.bold);
            p.setFont(f);
            QColor ink=item.color;
            if(!ink.isValid())ink=dark?QColor("#F5F7FF"):QColor("#242A35");
            p.setPen(ink);
            const QString text=evaluate(item,store,now).replace('\n',' ');
            const QFontMetrics metrics(f);
            const int textWidth=metrics.horizontalAdvance(text);
            if(e.marquee&&textWidth>row.width()) {
                const int gap=std::max(35,row.width()/3);
                const qint64 width=std::max(1,textWidth+gap);
                const int dx=int((elapsedMs/35)%width);
                const int baseline=row.top()+(row.height()+metrics.ascent()-metrics.descent())/2;
                p.drawText(row.left()-dx,baseline,text);
                p.drawText(row.left()-dx+textWidth+gap,baseline,text);
            } else {
                p.drawText(row,Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine,
                           metrics.elidedText(text,Qt::ElideRight,std::max(1,row.width())));
            }
        }
        p.restore();
    }
}
}
