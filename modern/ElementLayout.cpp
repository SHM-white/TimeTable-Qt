#include "ElementLayout.h"
#include "Translation.h"
#include "ContentClient.h"
#include "WeatherClient.h"
#include <QFontMetrics>
#include <QJsonValue>
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>
namespace tt {
namespace {
QString dateFormat(const QDateTime& date,const QString& fmt) {
    const QByteArray pattern=fmt.isEmpty()?QByteArray("%a %H:%M:%S"):fmt.toUtf8();
    if(pattern.size()>500) return tt::translate("时间格式过长");
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
    case 6:return "duty"; case 7:return "news"; case 8:return "dailyWord";
    case 9:return "weather"; case 10:return "today"; case 11:return "progressBar";
    case 12:return "weatherBlock"; case 13:return "analogClock";
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
QString clampString(QString value,int limit=2000) {
    if(value.size()>limit)value.truncate(limit);
    return value;
}
QFont itemFont(const TextElement& item) {
    QFont font(item.font,item.pointSize);
    font.setBold(item.bold);
    return font;
}
bool isText(const TextElement& item) {
    return item.type!="today" && item.type!="progressBar" &&
        item.type!="analogClock" && item.type!="weatherBlock";
}
qreal scrollSpeed(const ElementDefinition& element) {
    return std::isfinite(element.scrollPixelsPerSecond)
        ?std::clamp(element.scrollPixelsPerSecond,qreal(1),qreal(2000)):1000.0/35.0;
}
QString lineText(const TextElement& item,const ScheduleStore& store,const QDateTime& now,
                 qint64 elapsedMs) {
    return ElementLayout::evaluate(item,store,now,elapsedMs).replace('\n',' ');
}
// A rotating daily message must not change the duration of an outer cycle.
int cycleTextWidth(const TextElement& item,const ScheduleStore& store,const QDateTime& now) {
    const QFontMetrics metrics(itemFont(item));
    int width=metrics.horizontalAdvance(lineText(item,store,now,0));
    if(item.type=="duty" && item.rotateDuty) {
        const auto infos=store.dayInfos.value(now.date().dayOfWeek());
        for(int n=1;n<infos.size();++n)
            width=std::max(width,metrics.horizontalAdvance(lineText(item,store,now,n*5000LL)));
    }
    return width;
}
QSize naturalSize(const TextElement& item,const ScheduleStore& store,const QDateTime& now) {
    const QFontMetrics metrics(itemFont(item));
    if(item.type=="today") {
        const auto courses=store.coursesForDate(now.date());
        if(courses.isEmpty())return QSize(metrics.horizontalAdvance(tt::translate("今天没有课程"))+8,metrics.height()+6);
        int width=0,height=0;
        for(const auto& course:courses.mid(0,40)) {
            const QString name=item.abbreviate?course.name.left(1):course.name;
            if(item.vertical) {
                int columnWidth=0;
                for(const auto& character:name)
                    columnWidth=std::max(columnWidth,metrics.horizontalAdvance(QString(character)));
                width=std::max(width,std::max(12,columnWidth)+8);
                height=std::max(height,metrics.height()*std::max(1,int(name.size()))+8);
            } else {
                width=std::max(width,metrics.horizontalAdvance(course.start.toString("HH:mm")+" "+name)+8);
                height+=metrics.height()+6;
            }
        }
        if(item.vertical)width*=std::min(40,int(courses.size()));
        return QSize(width,height);
    }
    if(item.type=="analogClock")return QSize(120,140+metrics.height());
    if(item.type=="progressBar")return QSize(std::max(180,cycleTextWidth(item,store,now)+8),metrics.height()+22);
    if(item.type=="weatherBlock") {
        const auto lines=ElementLayout::evaluate(item,store,now).split('\n');
        int width=180;
        for(const auto& line:lines)width=std::max(width,metrics.horizontalAdvance(line)+16);
        return QSize(width,std::max(2,int(lines.size()))*metrics.height()+16);
    }
    return QSize(cycleTextWidth(item,store,now)+8,metrics.height()+6);
}
QRect effectiveRect(const ElementDefinition& element,const ScheduleStore& store,const QDateTime& now) {
    if(!element.autoSize || element.items.isEmpty())return element.rect;
    int width=0,height=0;
    if(element.mode=="all" && element.allLayout=="grid") {
        const int columns=std::clamp(element.columns,1,int(element.items.size()));
        int cellWidth=0,cellHeight=0;
        for(const auto& item:element.items) {
            const QSize size=naturalSize(item,store,now);
            cellWidth=std::max(cellWidth,size.width());cellHeight=std::max(cellHeight,size.height());
        }
        width=cellWidth*columns;
        height=cellHeight*((element.items.size()+columns-1)/columns);
    } else for(const auto& item:element.items) {
        const QSize size=naturalSize(item,store,now);
        if(element.mode=="all" && element.allLayout=="horizontal") {
            width=std::max(width,size.width());height=std::max(height,size.height());
        } else if(element.mode=="all") {
            width=std::max(width,size.width());height=std::max(height,size.height());
        } else {
            width=std::max(width,size.width());height=std::max(height,size.height());
        }
    }
    if(element.mode=="all" && element.allLayout=="horizontal")width*=element.items.size();
    else if(element.mode=="all" && element.allLayout!="grid")height*=element.items.size();
    return QRect(element.rect.topLeft(),QSize(std::clamp(width,16,3000),std::clamp(height,16,3000)));
}
struct CycleState { int index=0; qint64 elapsed=0; qint64 remaining=0; bool scrollPass=false; };
CycleState cycleState(const ElementDefinition& element,int width,const ScheduleStore& store,
                      const QDateTime& now,qint64 elapsedMs) {
    CycleState state;
    state.elapsed=std::max(qint64(0),elapsedMs);
    if(element.mode!="cycle" || element.items.size()<2)return state;
    if(!element.cycleAfterScroll) {
        const qint64 duration=1000LL*std::clamp(element.intervalSeconds,1,600);
        state.index=int((state.elapsed/duration)%element.items.size());
        state.elapsed%=duration;state.remaining=duration-state.elapsed;
        return state;
    }
    QList<qint64> durations;
    QList<bool> passes;
    qint64 total=0;
    for(const auto& item:element.items) {
        const int textWidth=isText(item)?cycleTextWidth(item,store,now):0;
        const bool pass=element.cycleAfterScroll && element.marquee && textWidth>width;
        const qint64 duration=pass?qint64(std::ceil((textWidth+width)*1000.0/scrollSpeed(element)))
            :1000LL*std::clamp(element.intervalSeconds,1,600);
        durations<<std::max(qint64(1),duration);passes<<pass;total+=durations.last();
    }
    qint64 phase=state.elapsed%total;
    for(int n=0;n<durations.size();++n) {
        if(phase<durations[n]) {
            state.index=n;state.elapsed=phase;state.remaining=durations[n]-phase;state.scrollPass=passes[n];
            break;
        }
        phase-=durations[n];
    }
    return state;
}
struct ItemView { const TextElement* item=nullptr; QRect row; qint64 elapsed=0; bool scrollPass=false; };
QList<ItemView> itemViews(const ElementDefinition& element,const QRect& target,
                          const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs) {
    if(element.items.isEmpty())return {};
    if(element.mode!="all") {
        const auto state=cycleState(element,target.width(),store,now,elapsedMs);
        return {{&element.items.at(state.index),target,state.elapsed,state.scrollPass}};
    }
    QList<ItemView> result;
    const int count=element.items.size();
    const int columns=element.allLayout=="grid"?std::clamp(element.columns,1,count)
        :element.allLayout=="horizontal"?count:1;
    const int rows=(count+columns-1)/columns;
    for(int n=0;n<count;++n) {
        const int left=target.left()+target.width()*(n%columns)/columns;
        const int right=target.left()+target.width()*((n%columns)+1)/columns;
        const int top=target.top()+target.height()*(n/columns)/rows;
        const int bottom=target.top()+target.height()*((n/columns)+1)/rows;
        result<<ItemView{&element.items.at(n),QRect(left,top,right-left,bottom-top),
                        std::max(qint64(0),elapsedMs),false};
    }
    return result;
}
qreal courseProgress(const ScheduleStore& store,const QDateTime& now) {
    const auto current=store.currentCourse(now);
    if(!current)return 0;
    const int total=current->start.secsTo(current->end);
    return total>0?std::clamp(qreal(current->start.secsTo(now.time()))/total,qreal(0),qreal(1)):0;
}
void paintClock(QPainter& painter,const QRect& row,const TextElement& item,const QDateTime& now,
                const QColor& ink,const QFontMetrics& metrics) {
    const int diameter=std::min(row.width()-8,row.height()-metrics.height()-10);
    const QString label=dateFormat(now,item.format.isEmpty()?QStringLiteral("%H:%M:%S"):item.format);
    if(diameter<12) {
        painter.drawText(row,Qt::AlignCenter,metrics.elidedText(label,Qt::ElideRight,row.width()));
        return;
    }
    const QPointF center(row.center().x(),row.top()+4+diameter/2.0);
    const qreal radius=diameter/2.0;
    painter.save();
    QColor fill=ink;fill.setAlpha(18);
    painter.setBrush(fill);painter.setPen(QPen(ink,1.5));
    painter.drawEllipse(center,radius,radius);painter.translate(center);
    for(int n=0;n<12;++n) {
        painter.drawLine(QPointF(0,-radius*.82),QPointF(0,-radius*.94));painter.rotate(30);
    }
    const QTime time=now.time();
    const auto hand=[&](qreal degrees,qreal length,qreal weight,const QColor& color) {
        painter.save();painter.rotate(degrees);painter.setPen(QPen(color,weight,Qt::SolidLine,Qt::RoundCap));
        painter.drawLine(QPointF(0,0),QPointF(0,-radius*length));painter.restore();
    };
    hand((time.hour()%12+time.minute()/60.0)*30,.5,3,ink);
    hand((time.minute()+time.second()/60.0)*6,.73,2,ink);
    hand(time.second()*6,.82,1,QColor("#6F64E8"));
    painter.setBrush(ink);painter.drawEllipse(QPointF(0,0),2,2);painter.restore();
    painter.drawText(QRect(row.left(),row.bottom()-metrics.height()+1,row.width(),metrics.height()),
                     Qt::AlignCenter,metrics.elidedText(label,Qt::ElideRight,row.width()));
}
} // namespace
QJsonObject TextElement::toJson() const {
    return {{"type",type},{"format",format},{"font",font},{"color",color.name(QColor::HexArgb)},
        {"pointSize",pointSize},{"bold",bold},{"vertical",vertical},{"abbreviate",abbreviate},
        {"target",target.toString(Qt::ISODate)},
        {"weatherApiKey",weatherApiKey},{"weatherAdcode",weatherAdcode},{"weatherCity",weatherCity},
        {"weatherAddress",weatherAddress},{"weatherForecast",weatherForecast},
        {"sourceUrl",sourceUrl},{"sourceFile",sourceFile},{"rotateDuty",rotateDuty}};
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
    item.vertical=o.value("vertical").toBool(false);
    item.abbreviate=o.value("abbreviate").toBool(false);
    item.target=QDateTime::fromString(o.value("target").toString(),Qt::ISODate);
    item.weatherApiKey=o.value("weatherApiKey").toString();
    item.weatherAdcode=o.value("weatherAdcode").toString();
    item.weatherCity=o.value("weatherCity").toString();
    item.weatherAddress=o.value("weatherAddress").toString();
    item.weatherForecast=o.value("weatherForecast").toBool(false);
    item.sourceUrl=o.value("sourceUrl").toString();
    item.sourceFile=o.value("sourceFile").toString();
    item.rotateDuty=o.value("rotateDuty").toBool(false);
    return item;
}
QJsonObject ElementDefinition::toJson() const {
    QJsonArray array;
    for(const auto& e:items) array.append(e.toJson());
    return {{"x",rect.x()},{"y",rect.y()},{"width",rect.width()},{"height",rect.height()},
        {"mode",mode},{"intervalSeconds",intervalSeconds},{"marquee",marquee},{"items",array},
        {"cycleAfterScroll",cycleAfterScroll},{"scrollPixelsPerSecond",scrollPixelsPerSecond},
        {"allLayout",allLayout},{"columns",columns},{"groupId",groupId},{"autoSize",autoSize}};
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
    e.cycleAfterScroll=o.value("cycleAfterScroll").toBool(false);
    e.scrollPixelsPerSecond=o.value("scrollPixelsPerSecond").toDouble(1000.0/35.0);
    e.scrollPixelsPerSecond=scrollSpeed(e);
    e.allLayout=o.value("allLayout").toString("vertical");
    if(e.allLayout!="vertical" && e.allLayout!="horizontal" && e.allLayout!="grid")e.allLayout="vertical";
    e.columns=std::clamp(o.value("columns").toInt(2),1,32);
    e.groupId=o.value("groupId").toString();
    e.autoSize=o.value("autoSize").toBool(false);
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
QList<ElementDefinition> ElementLayout::fromLegacy(const QJsonArray& array,int* skipped,qreal legacyScrollSpeed) {
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
        e.intervalSeconds=5;e.cycleAfterScroll=true;
        e.scrollPixelsPerSecond=std::isfinite(legacyScrollSpeed)&&legacyScrollSpeed>0
            ?std::clamp(1000.0/legacyScrollSpeed,qreal(1),qreal(2000)):1000.0/35.0;
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
            const auto extra=value.value("Data").toObject();
            if(type=="weather" || type=="weatherBlock") {
                text.weatherApiKey=extra.value("APIKey").toString();
                text.weatherAdcode=extra.value("Adcode").toVariant().toString();
                text.weatherCity=extra.value("City").toString();
                text.weatherAddress=extra.value("Address").toString();
                text.weatherForecast=extra.value("Forecast").toBool(true);
            }
            if(type=="news" || type=="dailyWord") {
                text.sourceUrl=extra.value("SourceUrl").toString(extra.value("URL").toString());
                text.sourceFile=extra.value("SourceFile").toString();
            }
            if(type=="duty")text.rotateDuty=true;
            if(type=="today") {
                text.vertical=extra.value("Direction").toInt()!=0;
                text.abbreviate=!extra.value("FullName").toBool(true);
                e.autoSize=true;
            }
            e.items<<text;
        }
        if(!e.items.isEmpty())out<<e;
    }
    return out;
}
QString ElementLayout::countdown(const QDateTime& target,const QDateTime& now,const QString& pattern) {
    if(!target.isValid())return tt::translate("请设置目标日期");
    const qint64 secs=std::max(qint64(0),now.secsTo(target));
    const qint64 nums[]={secs/86400,secs%86400/3600,secs%3600/60,secs%60};
    if(pattern.trimmed().isEmpty())
        return tt::translate("%1天 %2小时 %3分 %4秒").arg(nums[0]).arg(nums[1]).arg(nums[2]).arg(nums[3]);
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
QString ElementLayout::evaluate(const TextElement& item,const ScheduleStore& store,const QDateTime& now,
                               qint64 elapsedMs) {
    QString result;
    if(item.type=="clock" || item.type=="analogClock")return dateFormat(now,item.format);
    if(item.type=="weather" || item.type=="weatherBlock") {
        result=WeatherClient::instance().text(item.weatherApiKey,item.weatherAdcode,item.weatherCity,
            item.weatherAddress,item.weatherForecast);
    } else if(item.type=="news" || item.type=="dailyWord") {
        // The old implementation treated these types as literal TextFormat.
        if(item.sourceUrl.isEmpty() && item.sourceFile.isEmpty() && !item.format.isEmpty())
            return clampString(item.format);
        result=ContentClient::instance().text(item.type,item.sourceUrl,item.sourceFile,now);
    } else if(item.type=="note")return clampString(item.format);
    else if(item.type=="countdown")return countdown(item.target,now,item.format);
    else if(item.type=="current") {
        const auto current=store.currentCourse(now);
        result=current?current->name:tt::translate("无课程");
    } else if(item.type=="next"||item.type=="nextTime") {
        Course next;QDateTime start;
        const bool hasNext=store.nextCourse(now,&next,&start);
        result=hasNext?(item.type=="next"?next.name:countdown(start,now,QString())):
            tt::translate("暂无课程");
    } else if(item.type=="duty") {
        const auto infos=store.dayInfos.value(now.date().dayOfWeek());
        result=item.rotateDuty && !infos.isEmpty()
            ?infos.at(int((std::max(qint64(0),elapsedMs)/5000)%infos.size()))
            :infos.join(QStringLiteral(" · "));
    } else if(item.type=="week")result=tt::translate("第 %1 周").arg(store.weekNumber(now.date()));
    else if(item.type=="progress" || item.type=="progressBar") {
        result=store.currentCourse(now)?QStringLiteral("%1%").arg(int(courseProgress(store,now)*100))
            :tt::translate("未在上课");
    } else if(item.type=="today") {
        QStringList parts;
        for(const auto& course:store.coursesForDate(now.date()))
            parts<<course.start.toString("HH:mm")+" "+course.name;
        result=parts.join(QStringLiteral(" / "));
    }
    if(item.format.isEmpty())return clampString(result);
    return clampString(item.format.contains("{value}")?
        QString(item.format).replace("{value}",result):item.format+result);
}
QRect ElementLayout::contentBounds(const QList<ElementDefinition>& elements) {
    QRect bounds;
    for(const auto& e:elements)bounds=bounds.isNull()?e.rect:bounds.united(e.rect);
    return bounds;
}
QRect ElementLayout::contentBounds(const QList<ElementDefinition>& elements,const ScheduleStore& store,
                                 const QDateTime& now) {
    QRect bounds;
    for(const auto& element:elements) {
        const QRect rect=effectiveRect(element,store,now);
        bounds=bounds.isNull()?rect:bounds.united(rect);
    }
    return bounds;
}
bool ElementLayout::hasScrollingText(const QRect& area,const QList<ElementDefinition>& elements,
                                    const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs) {
    for(const auto& e:elements) {
        if(!e.marquee)continue;
        const QRect target=effectiveRect(e,store,now).translated(area.topLeft()).intersected(area);
        if(!target.intersects(area))continue;
        for(const auto& view:itemViews(e,target,store,now,elapsedMs)) {
            if(!isText(*view.item) || !view.row.intersects(area))continue;
            if(QFontMetrics(itemFont(*view.item)).horizontalAdvance(lineText(*view.item,store,now,elapsedMs))
                >view.row.width())return true;
        }
    }
    return false;
}
int ElementLayout::nextChangeDelay(const QRect& area,const QList<ElementDefinition>& elements,
                                  const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs) {
    qint64 next=0;
    elapsedMs=std::max(qint64(0),elapsedMs);
    const auto consider=[&](qint64 remaining) {
        if(remaining>0 && (!next || remaining<next))next=remaining;
    };
    for(const auto& element:elements) {
        const QRect target=effectiveRect(element,store,now).translated(area.topLeft()).intersected(area);
        if(element.items.isEmpty() || !target.intersects(area))continue;
        const auto cycle=cycleState(element,target.width(),store,now,elapsedMs);
        consider(cycle.remaining);
        const auto views=element.mode=="all"?itemViews(element,target,store,now,elapsedMs)
            :QList<ItemView>{{&element.items.at(cycle.index),target,cycle.elapsed,cycle.scrollPass}};
        for(const auto& view:views) {
            if(!view.row.intersects(area))continue;
            if(view.item->type=="duty" && view.item->rotateDuty &&
                store.dayInfos.value(now.date().dayOfWeek()).size()>1)
                consider(5000-elapsedMs%5000);
            if(!view.item->sourceUrl.isEmpty() || !view.item->sourceFile.isEmpty()) {
                if(view.item->type=="news")
                    consider(15000-((now.toMSecsSinceEpoch()%15000)+15000)%15000);
                else if(view.item->type=="dailyWord") {
                    QDateTime midnight=now;midnight.setDate(now.date().addDays(1));midnight.setTime(QTime(0,0));
                    consider(now.msecsTo(midnight));
                }
            }
        }
    }
    return int(std::min(next,qint64(std::numeric_limits<int>::max())));
}
void ElementLayout::paint(QPainter& painter,const QRect& area,const QList<ElementDefinition>& elements,
                          const ScheduleStore& store,const QDateTime& now,qint64 elapsedMs,bool dark) {
    for(const auto& element:elements) {
        const QRect target=effectiveRect(element,store,now).translated(area.topLeft()).intersected(area);
        if(!target.intersects(area))continue;
        for(const auto& view:itemViews(element,target,store,now,elapsedMs)) {
            const TextElement& item=*view.item;
            const QRect row=view.row;
            if(!row.intersects(area))continue;
            painter.save();painter.setClipRect(row.intersected(area),Qt::IntersectClip);
            const QFont font=itemFont(item);
            painter.setFont(font);
            QColor ink=item.color;
            if(!ink.isValid())ink=dark?QColor("#F5F7FF"):QColor("#242A35");
            painter.setPen(ink);
            const QFontMetrics metrics(font);
            if(item.type=="today") {
                const auto today=store.coursesForDate(now.date());
                if(today.isEmpty())painter.drawText(row,Qt::AlignCenter,tt::translate("今天没有课程"));
                const int count=std::min(40,int(today.size()));
                for(int n=0;n<count;++n) {
                    const auto& course=today.at(n);
                    const bool active=course.start<=now.time() && now.time()<course.end;
                    QRect courseArea;
                    if(item.vertical) {
                        const int left=row.left()+row.width()*n/count;
                        const int right=row.left()+row.width()*(n+1)/count;
                        courseArea=QRect(left,row.top(),right-left,row.height());
                    } else {
                        const int top=row.top()+row.height()*n/count;
                        const int bottom=row.top()+row.height()*(n+1)/count;
                        courseArea=QRect(row.left(),top,row.width(),bottom-top);
                    }
                    if(active) {
                        painter.save();painter.setPen(QPen(QColor("#6F64E8"),2));
                        painter.setBrush(QColor(105,94,230,45));
                        painter.drawRoundedRect(courseArea.adjusted(1,1,-2,-2),3,3);painter.restore();
                    }
                    const QString title=item.abbreviate?course.name.left(1):course.name;
                    if(item.vertical) {
                        QString lines;
                        for(const auto& ch:title) {if(!lines.isEmpty())lines+='\n';lines+=ch;}
                        painter.drawText(courseArea,Qt::AlignCenter|Qt::TextWordWrap,lines);
                    } else {
                        const QString line=course.start.toString("HH:mm")+" "+title;
                        painter.drawText(courseArea.adjusted(3,0,-2,0),Qt::AlignLeft|Qt::AlignVCenter,
                            metrics.elidedText(line,Qt::ElideRight,std::max(1,courseArea.width()-5)));
                    }
                }
            } else if(item.type=="progressBar") {
                const QRect bar=row.adjusted(3,std::max(metrics.height()+3,row.height()-12),-3,-3);
                QColor track=ink;track.setAlpha(30);
                painter.setPen(Qt::NoPen);painter.setBrush(track);painter.drawRoundedRect(bar,3,3);
                QRectF fill(bar);fill.setWidth(bar.width()*courseProgress(store,now));
                painter.setBrush(ink);painter.drawRoundedRect(fill,3,3);painter.setPen(ink);
                const QString text=evaluate(item,store,now,elapsedMs);
                painter.drawText(QRect(row.left()+3,row.top(),row.width()-6,metrics.height()+3),
                    Qt::AlignLeft|Qt::AlignVCenter,metrics.elidedText(text,Qt::ElideRight,std::max(1,row.width()-6)));
            } else if(item.type=="analogClock") {
                paintClock(painter,row,item,now,ink,metrics);
            } else if(item.type=="weatherBlock") {
                QColor background=ink;background.setAlpha(15);
                painter.setPen(Qt::NoPen);painter.setBrush(background);
                painter.drawRoundedRect(row.adjusted(1,1,-1,-1),6,6);painter.setPen(ink);
                const auto lines=evaluate(item,store,now,elapsedMs).split('\n');
                const int first=row.top()+std::max(4,(row.height()-int(lines.size())*metrics.height())/2);
                for(int n=0;n<lines.size();++n)
                    painter.drawText(QRect(row.left()+8,first+n*metrics.height(),row.width()-16,metrics.height()),
                        Qt::AlignLeft|Qt::AlignVCenter,metrics.elidedText(lines[n],Qt::ElideRight,std::max(1,row.width()-16)));
            } else {
                const QString text=lineText(item,store,now,elapsedMs);
                const int textWidth=metrics.horizontalAdvance(text);
                if(element.marquee && textWidth>row.width()) {
                    const int baseline=row.top()+(row.height()+metrics.ascent()-metrics.descent())/2;
                    if(view.scrollPass) {
                        const qreal x=row.left()+row.width()-view.elapsed*scrollSpeed(element)/1000.0;
                        painter.drawText(QPointF(x,baseline),text);
                    } else {
                        const int gap=std::max(35,row.width()/3);
                        const qreal dx=std::fmod(view.elapsed*scrollSpeed(element)/1000.0,textWidth+gap);
                        painter.drawText(QPointF(row.left()-dx,baseline),text);
                        painter.drawText(QPointF(row.left()-dx+textWidth+gap,baseline),text);
                    }
                } else painter.drawText(row,Qt::AlignLeft|Qt::AlignVCenter|Qt::TextSingleLine,
                    metrics.elidedText(text,Qt::ElideRight,std::max(1,row.width())));
            }
            painter.restore();
        }
    }
}
}
