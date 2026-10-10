#include "DesktopOverlays.h"
#include "MainWindow.h"
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QFont>
#include <QFrame>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFile>
#include <QFormLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QFontMetrics>
#include <QCoreApplication>
#include <QEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QUuid>
#include <algorithm>

namespace tt {
namespace {
const QStringList types={"clock","current","next","today","week","countdown","note","duty","combined"};
const QStringList names={QStringLiteral("时钟与日期"),QStringLiteral("当前课程"),QStringLiteral("下一节课程"),
                         QStringLiteral("今日课程"),QStringLiteral("教学周信息"),
                         QStringLiteral("目标日倒计时"),QStringLiteral("自定义便签"),QStringLiteral("每日信息 / 值日"),QStringLiteral("组合信息栏")};
QRect visibleGeometry(QRect rectangle) {
    QScreen* screen=QGuiApplication::screenAt(rectangle.center());
    if(!screen) screen=QGuiApplication::primaryScreen();
    if(!screen) return rectangle;
    const QRect bounds=screen->availableGeometry();
    rectangle.setSize(rectangle.size().boundedTo(bounds.size()));
    rectangle.moveLeft(std::clamp(rectangle.left(),bounds.left(),
        std::max(bounds.left(),bounds.right()-rectangle.width()+1)));
    rectangle.moveTop(std::clamp(rectangle.top(),bounds.top(),
        std::max(bounds.top(),bounds.bottom()-rectangle.height()+1)));
    return rectangle;
}
QString typeName(const QString& type) {
    const int i=types.indexOf(type);
    return names.value(i,QStringLiteral("信息组件"));
}
QString timeLeft(qint64 seconds) {
    if(seconds < 0) return QStringLiteral("已结束");
    const qint64 day=seconds/86400, hour=seconds%86400/3600, minute=seconds%3600/60;
    if(day) return QStringLiteral("%1 天 %2 小时").arg(day).arg(hour);
    if(hour) return QStringLiteral("%1 小时 %2 分").arg(hour).arg(minute);
    return QStringLiteral("%1 分 %2 秒").arg(minute).arg(seconds%60);
}
QString nameOrFallback(const OverlayConfig& c) {
    return c.title.isEmpty()?typeName(c.type):c.title;
}
QLabel* overlayLabel(QWidget* parent) {
    auto* result=new QLabel(parent);
    result->setTextFormat(Qt::PlainText);
    result->setWordWrap(true);
    result->setAttribute(Qt::WA_TransparentForMouseEvents);
    return result;
}
QJsonObject toJson(const OverlayConfig& c) {
    return QJsonObject{
        {"id",c.id},{"title",c.title},{"type",c.type},{"theme",c.theme},{"note",c.note},
        {"x",c.geometry.x()},{"y",c.geometry.y()},{"width",c.geometry.width()},
        {"height",c.geometry.height()},{"visible",c.visible},{"topmost",c.topmost},
        {"locked",c.locked},{"showDetailsOnClick",c.showDetailsOnClick},
        {"collapsed",c.collapsed},{"compactShowTitle",c.compactShowTitle},
        {"combinedItems",c.combinedItems},{"cycleItems",c.cycleItems},{"cycleSeconds",c.cycleSeconds},
        {"compactX",c.compactGeometry.x()},{"compactY",c.compactGeometry.y()},
        {"compactWidth",c.compactGeometry.width()},{"compactHeight",c.compactGeometry.height()},
        {"expandedX",c.expandedGeometry.x()},{"expandedY",c.expandedGeometry.y()},
        {"expandedWidth",c.expandedGeometry.width()},{"expandedHeight",c.expandedGeometry.height()},
        {"fontSize",c.fontSize},{"opacity",c.opacity},{"target",c.target.toString(Qt::ISODate)}
    };
}
OverlayConfig fromJson(const QJsonObject& j) {
    OverlayConfig c;
    c.id=j.value("id").toString();
    c.title=j.value("title").toString();
    c.type=j.value("type").toString("current");
    c.theme=j.value("theme").toString("light");
    c.note=j.value("note").toString();
    c.geometry=QRect(j.value("x").toInt(20),j.value("y").toInt(20),
                     j.value("width").toInt(300),j.value("height").toInt(160));
    c.collapsed=j.value("collapsed").toBool(true);
    c.compactShowTitle=j.value("compactShowTitle").toBool(false);
    c.combinedItems=j.value("combinedItems").toString("clock,current,next");
    c.cycleItems=j.value("cycleItems").toBool(false);
    c.cycleSeconds=std::clamp(j.value("cycleSeconds").toInt(5),2,300);
    c.compactGeometry=QRect(j.value("compactX").toInt(c.geometry.x()),j.value("compactY").toInt(c.geometry.y()),
        j.value("compactWidth").toInt(195),j.value("compactHeight").toInt(54));
    c.expandedGeometry=QRect(j.value("expandedX").toInt(c.geometry.x()),j.value("expandedY").toInt(c.geometry.y()),
        j.value("expandedWidth").toInt(std::max(230,c.geometry.width())),
        j.value("expandedHeight").toInt(std::max(130,c.geometry.height())));
    c.geometry=c.collapsed?c.compactGeometry:c.expandedGeometry;
    c.visible=j.value("visible").toBool(true);
    c.topmost=j.value("topmost").toBool(true);
    c.locked=j.value("locked").toBool(false);
    c.showDetailsOnClick=j.value("showDetailsOnClick").toBool(true);
    c.fontSize=j.value("fontSize").toInt(13);
    c.opacity=j.value("opacity").toInt(94);
    c.target=QDateTime::fromString(j.value("target").toString(),Qt::ISODate);
    return c;
}
QString paletteCss(bool dark) {
    const QString bg=dark?"#202535":"#FFFFFF";
    const QString fg=dark?"#F5F7FF":"#252B3B";
    const QString edge=dark?"#474E62":"#DCE1ED";
    return QString(
        "QDialog{background:%1;color:%2;} QLabel,QCheckBox{color:%2;background:transparent;}"
        "QLineEdit,QComboBox,QSpinBox,QDateTimeEdit,QListWidget{background:%1;color:%2;border:1px solid %3;border-radius:7px;padding:6px;}"
        "QPushButton{background:%1;color:%2;border:1px solid %3;border-radius:8px;padding:8px 13px;}"
        "QPushButton:hover{background:%3;} QListWidget::item:selected{background:#625DE1;color:white;}"
    ).arg(bg,fg,edge);
}
} // namespace

class OverlayCard final : public QWidget {
public:
    OverlayCard(const OverlayConfig& c, DesktopOverlays* manager)
        : QWidget(nullptr), config_(c), manager_(manager) {
        setObjectName("desktopOverlay");
        setAttribute(Qt::WA_TranslucentBackground);
        setMouseTracking(true);
        setWindowTitle(nameOrFallback(c));
        setToolTip(QStringLiteral("拖动移动 · 拖右下角缩放 · 右键折叠/配置 · 单击详情"));
        auto flags=Qt::Tool|Qt::FramelessWindowHint;
        if(c.topmost) flags|=Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        setMinimumSize(c.collapsed?QSize(125,44):QSize(190,102));
        auto* content=new QVBoxLayout(this);
        content->setContentsMargins(17,13,17,12);
        content->setSpacing(6);
        heading_=overlayLabel(this);
        heading_->setFont(QFont(font().family(),10,QFont::DemiBold));
        main_=overlayLabel(this);
        secondary_=overlayLabel(this);
        footer_=overlayLabel(this);
        content->addWidget(heading_);
        content->addWidget(main_,1);
        content->addWidget(secondary_);
        content->addWidget(footer_);
        for(auto* child:{heading_,main_,secondary_,footer_})
            child->setVisible(!c.collapsed);
        setGeometry(c.geometry);
        updateModel();
    }

    void updateModel() {
        const auto& store=manager_->planner_->schedule();
        const auto now=QDateTime::currentDateTime();
        const bool dark=config_.theme=="dark";
        const QString ink=dark?"#F5F6FC":"#22283A";
        const QString muted=dark?"#ADB8CE":"#667187";
        heading_->setText(nameOrFallback(config_));
        heading_->setStyleSheet("color:"+muted+";background:transparent;");
        main_->setStyleSheet("color:"+ink+";background:transparent;");
        secondary_->setStyleSheet("color:"+muted+";background:transparent;");
        footer_->setStyleSheet("color:"+(dark?QString("#BCB5FF"):QString("#6153D2"))+";background:transparent;");
        QFont large=main_->font();
        large.setPointSize(config_.fontSize+3);
        large.setBold(true); main_->setFont(large);
        QFont small=secondary_->font();
        small.setPointSize(std::max(9,config_.fontSize-3)); secondary_->setFont(small);
        const Course* current=store.currentCourse(now);
        Course next;QDateTime when;
        const bool hasNext=store.nextCourse(now,&next,&when);
        QString big,smallText,foot;
        if(config_.type=="clock") {
            big=now.toString("HH:mm:ss");
            smallText=now.toString("yyyy-MM-dd dddd");
            foot=QStringLiteral("双击打开课表 · 右键设置");
        } else if(config_.type=="current") {
            big=current?current->name:QStringLiteral("当前无课程");
            smallText=current ? current->start.toString("HH:mm")+"–"+
                current->end.toString("HH:mm")+"  "+current->room :
                QStringLiteral("点击查看今日完整课程");
            foot=current?QStringLiteral("距离下课 %1").arg(timeLeft(now.secsTo(QDateTime(now.date(),current->end))))
                :QStringLiteral("第 %1 周").arg(store.weekNumber(now.date()));
        } else if(config_.type=="next") {
            big=hasNext?next.name:QStringLiteral("暂无下一节课");
            smallText=hasNext?when.toString("MM-dd dddd HH:mm")+"  "+next.room:
                QStringLiteral("本学期没有后续课程");
            foot=hasNext?QStringLiteral("距离开始 %1").arg(timeLeft(now.secsTo(when))):QString();
        } else if(config_.type=="today") {
            const auto today=store.coursesForDate(now.date());
            big=today.isEmpty()?QStringLiteral("今天没有课程"):QStringLiteral("今日 %1 节课").arg(today.size());
            QStringList summaries;
            for(int i=0;i<std::min(3,int(today.size()));++i)
                summaries << today[i].start.toString("HH:mm")+" "+today[i].name;
            smallText=summaries.join("\n");
            foot=QStringLiteral("点击展开完整课程");
        } else if(config_.type=="week") {
            const int week=store.weekNumber(now.date());
            big=week?QStringLiteral("第 %1/%2 周").arg(week).arg(store.totalWeeks):
                QStringLiteral("当前不在学期内");
            smallText=QStringLiteral("开学 %1").arg(store.semesterMonday.toString("yyyy-MM-dd"));
            foot=QStringLiteral("点击查看周课表");
        } else if(config_.type=="countdown") {
            big=config_.target.isValid()?timeLeft(now.secsTo(config_.target)):QStringLiteral("未设置目标");
            smallText=config_.target.isValid()?config_.target.toString("yyyy-MM-dd HH:mm"):
                QStringLiteral("请右键设置目标日期");
            foot=QStringLiteral("倒计日");
        } else if(config_.type=="duty") {
            const QString duty=QSettings().value("desktop/duty/"+QString::number(now.date().dayOfWeek())).toString();
            big=duty.isEmpty()?QStringLiteral("今日无提醒"):duty;
            smallText=now.toString("dddd");
            foot=QStringLiteral("右键可配置每周值日/提示");
        } else if(config_.type=="combined") {
            QStringList parts;
            for(const QString& token:config_.combinedItems.split(',',Qt::SkipEmptyParts)) {
                const QString key=token.trimmed().toLower();
                if(key=="clock") parts<<now.toString("HH:mm");
                else if(key=="date") parts<<now.toString("MM/dd");
                else if(key=="current") parts<<(current?current->name:QStringLiteral("无课"));
                else if(key=="next") parts<<(hasNext?QStringLiteral("下一节 ")+next.name:QStringLiteral("无后续课"));
                else if(key=="week") parts<<QStringLiteral("第%1周").arg(store.weekNumber(now.date()));
                else if(key=="duty") {
                    const QString duty=QSettings().value("desktop/duty/"+QString::number(now.date().dayOfWeek())).toString();
                    if(!duty.isEmpty()) parts<<duty;
                } else if(key=="countdown"&&config_.target.isValid()) parts<<timeLeft(now.secsTo(config_.target));
                else if(key=="note"&&!config_.note.isEmpty()) parts<<config_.note;
            }
            if(parts.isEmpty()) big=QStringLiteral("请配置组合信息");
            else if(config_.cycleItems) {
                const auto step=now.toSecsSinceEpoch()/std::max(2,config_.cycleSeconds);
                big=parts.at(int(step%parts.size()));
                foot=QStringLiteral("信息轮播 %1/%2").arg(int(step%parts.size())+1).arg(parts.size());
            } else big=parts.join("  ·  ");
            smallText=parts.join("\n");
            foot=QStringLiteral("右键编辑组合信息");
        } else {
            big=config_.note.isEmpty()?QStringLiteral("右键编辑便签"):config_.note;
            foot=QStringLiteral("自定义便签");
        }
        if(value_!=big) {
            value_=big;
            if(config_.collapsed) update();
        }
        if(main_->text()!=big) main_->setText(big);
        if(secondary_->text()!=smallText) secondary_->setText(smallText);
        if(footer_->text()!=foot) footer_->setText(foot);
        if(!config_.collapsed) {
            secondary_->setVisible(!smallText.isEmpty());
            footer_->setVisible(!foot.isEmpty());
        }
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const bool dark=config_.theme=="dark";
        QColor background=dark?QColor("#202535"):QColor("#FDFEFF");
        background.setAlphaF(config_.opacity/100.0);
        p.setBrush(background);
        p.setPen(QPen(dark?QColor("#485069"):QColor("#DCE1ED"),1));
        p.drawRoundedRect(rect().adjusted(1,1,-2,-2),config_.collapsed?11:17,config_.collapsed?11:17);
        if(config_.collapsed) {
            const int inset=10;
            QRect textRect=rect().adjusted(inset,4,-19,-4);
            const QColor ink=dark?QColor("#F5F6FC"):QColor("#22283A");
            if(config_.compactShowTitle) {
                QFont titleFont=font();titleFont.setPointSize(9);
                p.setFont(titleFont);p.setPen(dark?QColor("#ACB5CE"):QColor("#70798A"));
                const QString prefix=nameOrFallback(config_);
                const int titleWidth=std::min(QFontMetrics(titleFont).horizontalAdvance(prefix)+11,textRect.width()/2);
                QRect titleRect=textRect;titleRect.setWidth(titleWidth);
                p.drawText(titleRect,Qt::AlignLeft|Qt::AlignVCenter,
                           QFontMetrics(titleFont).elidedText(prefix,Qt::ElideRight,titleWidth-6));
                textRect.setLeft(textRect.left()+titleWidth);
            }
            QFont boldFont=font();
            boldFont.setPointSize(config_.fontSize);
            boldFont.setBold(true);
            p.setFont(boldFont);p.setPen(ink);
            const QString oneLine=value_.simplified();
            p.drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,
                QFontMetrics(boldFont).elidedText(oneLine,Qt::ElideRight,std::max(1,textRect.width())));
        } else {
            p.setPen(Qt::NoPen);p.setBrush(QColor("#8170F8"));
            p.drawRoundedRect(QRect(12,16,3,14),1.5,1.5);
        }
        // Compact resize affordance only while hovered. Bottom-right drag changes size.
        if(!config_.locked && underMouse()) {
            p.setPen(QPen(dark?QColor("#8890A3"):QColor("#9AA3B5"),1));
            for(int n=0;n<3;++n) {
                p.drawLine(width()-6-n*4,height()-5,width()-5,height()-6-n*4);
            }
        }
    }
    void mousePressEvent(QMouseEvent* event) override {
        if(event->button()==Qt::LeftButton) {
            down_=event->globalPosition().toPoint();
            originalGeometry_=geometry();
            pressed_=true; moved_=false;
            resizing_=!config_.locked && event->position().x()>width()-18 &&
                      event->position().y()>height()-18;
            event->accept();return;
        }
        QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(pressed_ && (event->buttons()&Qt::LeftButton)) {
            const QPoint delta=event->globalPosition().toPoint()-down_;
            if(delta.manhattanLength()>4) moved_=true;
            if(moved_ && !config_.locked) {
                if(resizing_) resize(QSize(originalGeometry_.width()+delta.x(),
                                           originalGeometry_.height()+delta.y())
                                    .expandedTo(minimumSize()));
                else move(originalGeometry_.topLeft()+delta);
            }
            event->accept();return;
        }
        const bool overCorner=event->position().x()>width()-18 &&
                              event->position().y()>height()-18;
        setCursor(!config_.locked && overCorner?Qt::SizeFDiagCursor:Qt::ArrowCursor);
        update();
        QWidget::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        if(event->button()==Qt::LeftButton && pressed_) {
            pressed_=false;
            if(moved_) {
                ++clickSerial_;
                if(!config_.locked) manager_->moved(config_.id,geometry());
            } else if(config_.showDetailsOnClick && !resizing_) {
                const int serial=++clickSerial_;
                const QPoint anchor=event->globalPosition().toPoint();
                QTimer::singleShot(QApplication::doubleClickInterval(),this,[this,serial,anchor] {
                    if(serial==clickSerial_) manager_->showDetail(config_.id,anchor);
                });
            }
            resizing_=false;
            event->accept();return;
        }
        QWidget::mouseReleaseEvent(event);
    }
    void mouseDoubleClickEvent(QMouseEvent* event) override {
        if(event->button()==Qt::LeftButton) {
            ++clickSerial_;
            manager_->openPlanner(config_.type=="week"?1:0);
            event->accept();return;
        }
        QWidget::mouseDoubleClickEvent(event);
    }
    void contextMenuEvent(QContextMenuEvent* event) override {
        manager_->contextMenu(config_.id,event->globalPos());
        event->accept();
    }
private:
    OverlayConfig config_;
    DesktopOverlays* manager_;
    QLabel *heading_,*main_,*secondary_,*footer_;
    QString value_;
    bool pressed_=false,moved_=false,resizing_=false;
    int clickSerial_=0;
    QPoint down_;
    QRect originalGeometry_;
};

DesktopOverlays::DesktopOverlays(MainWindow* planner):QObject(planner),planner_(planner) {}
DesktopOverlays::~DesktopOverlays() { qDeleteAll(windows_);windows_.clear(); }

OverlayConfig DesktopOverlays::defaultConfig(const QString& type) const {
    OverlayConfig c;
    c.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    c.title=typeName(type);
    c.type=type;
    c.theme=QSettings().value("desktop/defaultTheme","light").toString();
    const QRect screen=QGuiApplication::primaryScreen()->availableGeometry();
    c.compactGeometry=QRect(screen.x()+25,screen.y()+25,195,54);
    c.expandedGeometry=QRect(screen.x()+25,screen.y()+25,310,150);
    c.geometry=c.compactGeometry;
    c.collapsed=true;
    c.target=QDateTime(QDate::currentDate().addDays(30),QTime(0,0));
    return c;
}
void DesktopOverlays::load() {
    configs_.clear();
    QSettings settings;
    const QByteArray json=settings.value("desktop/windows").toByteArray();
    if(!json.isEmpty()) {
        const auto doc=QJsonDocument::fromJson(json);
        if(doc.isArray()) for(const auto& item:doc.array()) {
            auto config=fromJson(item.toObject());
            if(config.id.isEmpty()||!types.contains(config.type)||
                config.opacity<30||config.opacity>100 || config.fontSize<8||config.fontSize>35)
                continue;
            if(config.theme!="dark"&&config.theme!="light") config.theme="light";
            config.compactGeometry.setSize(config.compactGeometry.size().expandedTo(QSize(125,44)));
            config.expandedGeometry.setSize(config.expandedGeometry.size().expandedTo(QSize(190,102)));
            config.compactGeometry=visibleGeometry(config.compactGeometry);
            config.expandedGeometry=visibleGeometry(config.expandedGeometry);
            config.geometry=config.collapsed?config.compactGeometry:config.expandedGeometry;
            configs_.append(config);
        }
    } else {
        auto clock=defaultConfig("clock");clock.compactGeometry.setSize(QSize(158,48));clock.geometry=clock.compactGeometry;
        auto current=defaultConfig("current");current.compactGeometry.moveTopLeft(clock.compactGeometry.topRight()+QPoint(12,0));current.geometry=current.compactGeometry;
        auto next=defaultConfig("next");next.compactGeometry.moveTopLeft(current.compactGeometry.topRight()+QPoint(12,0));next.geometry=next.compactGeometry;
        configs_={clock,current,next};
        save();
    }
}
void DesktopOverlays::save() const {
    QJsonArray array;
    for(const auto& c:configs_) array.append(toJson(c));
    QSettings().setValue("desktop/windows",QJsonDocument(array).toJson(QJsonDocument::Compact));
}
void DesktopOverlays::start() {
    load();
    planner_->installEventFilter(this);
    rebuild();
    planner_->setOnScheduleChanged([this]{refresh();});
    auto* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this]{refresh();});
    timer->start(1000);
}
void DesktopOverlays::rebuild() {
    // Context-menu actions may rebuild a window while its event handler is active.
    // Defer deletion to the event loop to avoid destroying the event receiver.
    for(auto* w:windows_) { w->hide(); w->deleteLater(); }
    windows_.clear();
    for(const auto& c:configs_) {
        auto* w=new OverlayCard(c,this);
        windows_.insert(c.id,w);
        if(c.visible && !planner_->isVisible() &&
            !(configurationDialog_ && configurationDialog_->isVisible())) {
            w->show();
            auto* animation=new QPropertyAnimation(w,"windowOpacity",w);
            animation->setDuration(190);
            animation->setStartValue(0.0);
            animation->setEndValue(1.0);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
    refreshConfigurationList();
}
bool DesktopOverlays::eventFilter(QObject* watched, QEvent* event) {
    if(watched==planner_ || watched==configurationDialog_.data()) {
        if(event->type()==QEvent::Show || event->type()==QEvent::Hide) {
            // Complete widget visibility changes at the next event-loop turn.
            QTimer::singleShot(0,this,[this]{syncVisibility();});
        }
    }
    return QObject::eventFilter(watched,event);
}
void DesktopOverlays::syncVisibility() {
    const bool covered=planner_->isVisible() ||
        (configurationDialog_ && configurationDialog_->isVisible());
    for(const auto& c:configs_) {
        if(auto* w=windows_.value(c.id,nullptr)) {
            if(c.visible && !covered) w->show();
            else w->hide();
        }
    }
}
int DesktopOverlays::indexOf(const QString& id) const {
    for(int i=0;i<configs_.size();++i) if(configs_[i].id==id) return i;
    return -1;
}
void DesktopOverlays::refresh() {
    for(auto* w:windows_) if(w && w->isVisible()) w->updateModel();
}
void DesktopOverlays::moved(const QString& id,const QRect& geometry) {
    const int index=indexOf(id);
    if(index<0) return;
    configs_[index].geometry=geometry;
    if(configs_[index].collapsed) configs_[index].compactGeometry=geometry;
    else configs_[index].expandedGeometry=geometry;
    save();
}
void DesktopOverlays::toggleCollapsed(const QString& id) {
    const int index=indexOf(id);
    if(index<0) return;
    OverlayConfig& c=configs_[index];
    if(auto* window=windows_.value(id,nullptr)) {
        c.geometry=window->geometry();
    }
    if(c.collapsed) c.compactGeometry=c.geometry;
    else c.expandedGeometry=c.geometry;
    c.collapsed=!c.collapsed;
    const QRect other=c.collapsed?c.compactGeometry:c.expandedGeometry;
    // Preserve the desktop position when switching modes, but remember each size.
    c.geometry=QRect(c.geometry.topLeft(),other.size());
    if(c.collapsed) c.compactGeometry=c.geometry;
    else c.expandedGeometry=c.geometry;
    save();rebuild();
}
void DesktopOverlays::openPlanner(int page) {
    planner_->openPage(page);
    planner_->showNormal();
    planner_->raise();
    planner_->activateWindow();
}
QString DesktopOverlays::detailsText(const OverlayConfig& c) const {
    const auto& store=planner_->schedule();
    const auto now=QDateTime::currentDateTime();
    QStringList lines;
    if(c.type=="note") return c.note;
    if(c.type=="countdown")
        return nameOrFallback(c)+"\n"+c.target.toString("yyyy-MM-dd HH:mm")+
            "\n"+timeLeft(now.secsTo(c.target));
    if(c.type=="clock") lines << now.toString("yyyy-MM-dd dddd HH:mm:ss");
    const QDate date=now.date();
    if(c.type=="week") {
        const auto monday=ScheduleStore::mondayOf(date);
        for(int day=0;day<7;++day) {
            const auto d=monday.addDays(day);
            const auto lessons=store.coursesForDate(d);
            lines << d.toString("MM-dd dddd")+QStringLiteral(" · %1 节").arg(lessons.size());
            for(const auto& course:lessons)
                lines << "  "+course.start.toString("HH:mm")+" "+course.name+
                    (course.room.isEmpty()?"":" · "+course.room);
        }
    } else if(c.type=="next") {
        Course next;QDateTime start;
        if(store.nextCourse(now,&next,&start)) {
            lines << start.toString("yyyy-MM-dd dddd HH:mm")
                  << next.name
                  << QStringLiteral("结束 %1").arg(next.end.toString("HH:mm"))
                  << next.room << next.teacher
                  << QStringLiteral("有效周：%1").arg(ScheduleStore::formatWeeks(next.weeks));
        } else lines << QStringLiteral("未来 15 天无课程");
    } else {
        lines << date.toString("yyyy-MM-dd dddd");
        const auto lessons=store.coursesForDate(date);
        if(lessons.isEmpty()) lines << QStringLiteral("今天没有课程");
        for(const auto& course:lessons) {
            lines << course.start.toString("HH:mm")+"–"+course.end.toString("HH:mm")+
                "  "+course.name+(course.room.isEmpty()?"":" · "+course.room);
        }
    }
    return lines.join("\n");
}
void DesktopOverlays::showDetail(const QString& id,const QPoint& anchor) {
    const int index=indexOf(id);
    if(index<0) return;
    const OverlayConfig c=configs_[index];
    auto* popup=new QDialog(nullptr,Qt::Popup|Qt::FramelessWindowHint);
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setStyleSheet(paletteCss(c.theme=="dark"));
    auto* layout=new QVBoxLayout(popup);
    layout->setContentsMargins(20,17,20,17);layout->setSpacing(12);
    auto* title=new QLabel(nameOrFallback(c));
    QFont f=title->font();f.setPointSize(14);f.setBold(true);title->setFont(f);
    layout->addWidget(title);
    auto* scroll=new QScrollArea;
    scroll->setFrameShape(QFrame::NoFrame);scroll->setWidgetResizable(true);
    auto* details=new QLabel(detailsText(c));
    details->setTextFormat(Qt::PlainText);
    details->setTextInteractionFlags(Qt::TextSelectableByMouse);
    details->setWordWrap(true);
    details->setMargin(4);
    scroll->setWidget(details);scroll->setMinimumHeight(100);
    layout->addWidget(scroll,1);
    auto* action=new QPushButton(QStringLiteral("打开完整课表与设置"));
    layout->addWidget(action);
    connect(action,&QPushButton::clicked,popup,[this,popup,c]{
        popup->close();openPlanner(c.type=="week"?1:0);
    });
    popup->resize(390,300);
    QScreen* screen=QGuiApplication::screenAt(anchor);
    if(!screen) screen=QGuiApplication::primaryScreen();
    const QRect bounds=screen->availableGeometry();
    const int x=std::clamp(anchor.x(),bounds.left(),std::max(bounds.left(),bounds.right()-popup->width()));
    const int y=std::clamp(anchor.y(),bounds.top(),std::max(bounds.top(),bounds.bottom()-popup->height()));
    popup->move(x,y);popup->show();
}
void DesktopOverlays::contextMenu(const QString& id,const QPoint& anchor) {
    const int index=indexOf(id);
    if(index<0) return;
    const OverlayConfig c=configs_[index];
    QMenu menu;
    QAction* details=menu.addAction(QStringLiteral("展开此窗口详情"));
    QAction* planner=menu.addAction(QStringLiteral("打开完整课程管理器"));
    menu.addSeparator();
    QAction* collapse=menu.addAction(c.collapsed?QStringLiteral("展开卡片（显示完整信息）"):QStringLiteral("折叠成小卡片"));
    QAction* editWindow=menu.addAction(QStringLiteral("设置此窗口"));
    QAction* manage=menu.addAction(QStringLiteral("管理全部桌面窗口"));
    QAction* add=menu.addAction(QStringLiteral("新建桌面窗口"));
    QAction* duty=menu.addAction(QStringLiteral("编辑每周值日 / 提示"));
    menu.addSeparator();
    QAction* top=menu.addAction(c.topmost?QStringLiteral("取消置顶"):QStringLiteral("始终置顶"));
    QAction* lock=menu.addAction(c.locked?QStringLiteral("解锁拖动"):QStringLiteral("锁定位置"));
    QAction* hide=menu.addAction(QStringLiteral("隐藏此窗口"));
    QAction* hideEverything=menu.addAction(QStringLiteral("隐藏全部桌面卡片（可从托盘恢复）"));
    QAction* removeWindow=menu.addAction(QStringLiteral("删除此窗口"));
    menu.addSeparator();
    QAction* quit=menu.addAction(QStringLiteral("退出 TimeTable Next"));
    QAction* selected=menu.exec(anchor);
    if(!selected) return;
    if(selected==details) showDetail(id,anchor);
    else if(selected==planner) openPlanner();
    else if(selected==collapse) toggleCollapsed(id);
    else if(selected==editWindow) edit(id);
    else if(selected==manage) openConfiguration();
    else if(selected==add) edit();
    else if(selected==duty) editDutyNotes();
    else if(selected==top) { configs_[index].topmost=!c.topmost;save();rebuild(); }
    else if(selected==lock) { configs_[index].locked=!c.locked;save();rebuild(); }
    else if(selected==hide) { configs_[index].visible=false;save();rebuild();openConfiguration(); }
    else if(selected==hideEverything) hideAll();
    else if(selected==removeWindow) remove(id);
    else if(selected==quit) QApplication::quit();
}
void DesktopOverlays::refreshConfigurationList() {
    if(!configurationList_) return;
    configurationList_->clear();
    for(const auto& c:configs_)
        configurationList_->addItem(QString("%1  ·  %2  ·  %3")
            .arg(c.visible?QStringLiteral("●"):QStringLiteral("○"),
                 nameOrFallback(c)+(c.collapsed?QStringLiteral(" [折叠]"):QStringLiteral(" [展开]")),typeName(c.type)));
}
void DesktopOverlays::openConfiguration() {
    if(configurationDialog_) {
        configurationDialog_->raise();
        configurationDialog_->activateWindow();
        return;
    }
    auto* dlg=new QDialog(nullptr,Qt::Window);
    configurationDialog_=dlg;
    dlg->installEventFilter(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("桌面窗口管理 · TimeTable Next"));
    dlg->setMinimumSize(650,480);
    dlg->setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* layout=new QVBoxLayout(dlg);
    layout->setContentsMargins(22,20,22,20);layout->setSpacing(12);
    auto* title=new QLabel(QStringLiteral("独立桌面窗口"));
    auto font=title->font();font.setPointSize(17);font.setBold(true);title->setFont(font);
    layout->addWidget(title);
    auto* hint=new QLabel(QStringLiteral("默认小卡片仅占一行；拖动右下角缩放，右键可折叠或展开。\n左键弹出详情，双击打开课表管理器，位置与两种尺寸分别保存。"));
    hint->setWordWrap(true);layout->addWidget(hint);
    auto* list=new QListWidget;configurationList_=list;
    layout->addWidget(list,1);
    auto* buttons=new QHBoxLayout;
    auto* add=new QPushButton(QStringLiteral("＋ 新建"));
    auto* importOld=new QPushButton(QStringLiteral("导入旧版窗口"));
    auto* showAllButton=new QPushButton(QStringLiteral("全部显示"));
    auto* hideAllButton=new QPushButton(QStringLiteral("全部隐藏"));
    auto* editButton=new QPushButton(QStringLiteral("编辑选中"));
    auto* toggle=new QPushButton(QStringLiteral("显示 / 隐藏"));
    auto* removeButton=new QPushButton(QStringLiteral("删除"));
    auto* collapseButton=new QPushButton(QStringLiteral("折叠 / 展开"));
    buttons->addWidget(add);buttons->addWidget(editButton);buttons->addWidget(collapseButton);
    buttons->addWidget(toggle);buttons->addWidget(removeButton);
    layout->addLayout(buttons);
    auto* migrationHint=new QLabel(QStringLiteral("旧版 config.json 的窗口信息可导入为组合卡片；不影响原课表文件。"));
    migrationHint->setWordWrap(true);
    layout->addWidget(migrationHint);
    auto* allActions=new QHBoxLayout;
    allActions->addWidget(importOld);allActions->addWidget(showAllButton);allActions->addWidget(hideAllButton);
    layout->addLayout(allActions);
    auto* footer=new QHBoxLayout;
    auto* notes=new QPushButton(QStringLiteral("每周值日 / 提示"));
    auto* autostart=new QCheckBox(QStringLiteral("开机自启动"));
#ifdef Q_OS_WIN
    const QString startupPath=QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings startup(startupPath,QSettings::NativeFormat);
    autostart->setChecked(startup.contains("TimeTableNext"));
#else
    autostart->setEnabled(false);
    autostart->setToolTip(QStringLiteral("仅在 Windows 上可用"));
#endif
    auto* full=new QPushButton(QStringLiteral("进入完整课程管理器"));
    auto* close=new QPushButton(QStringLiteral("关闭"));
    footer->addWidget(notes);footer->addWidget(autostart);footer->addStretch();
    footer->addWidget(full);footer->addWidget(close);
    layout->addLayout(footer);
    const auto selectedIndex=[list] { return list->currentRow(); };
    connect(add,&QPushButton::clicked,dlg,[this]{edit();});
    connect(importOld,&QPushButton::clicked,dlg,[this]{importLegacyWindows();});
    connect(showAllButton,&QPushButton::clicked,dlg,[this]{showAll();});
    connect(hideAllButton,&QPushButton::clicked,dlg,[this]{hideAll();});
    connect(editButton,&QPushButton::clicked,dlg,[this,selectedIndex]{
        int index=selectedIndex();if(index>=0&&index<configs_.size()) edit(configs_[index].id);
    });
    connect(list,&QListWidget::itemDoubleClicked,dlg,[this,list](QListWidgetItem*){
        int index=list->currentRow();if(index>=0&&index<configs_.size()) edit(configs_[index].id);
    });
    connect(collapseButton,&QPushButton::clicked,dlg,[this,selectedIndex]{
        const int index=selectedIndex();
        if(index>=0&&index<configs_.size()) toggleCollapsed(configs_[index].id);
    });
    connect(toggle,&QPushButton::clicked,dlg,[this,selectedIndex]{
        int index=selectedIndex();
        if(index>=0&&index<configs_.size()) {
            configs_[index].visible=!configs_[index].visible;save();rebuild();
        }
    });
    connect(removeButton,&QPushButton::clicked,dlg,[this,selectedIndex]{
        int index=selectedIndex();if(index>=0&&index<configs_.size()) remove(configs_[index].id);
    });
    connect(notes,&QPushButton::clicked,dlg,[this]{editDutyNotes();});
#ifdef Q_OS_WIN
    connect(autostart,&QCheckBox::toggled,dlg,[](bool enabled){
        const QString key=QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
        QSettings reg(key,QSettings::NativeFormat);
        if(enabled) reg.setValue("TimeTableNext",QStringLiteral("\"")+QCoreApplication::applicationFilePath().replace('/','\\')+QStringLiteral("\""));
        else reg.remove("TimeTableNext");
    });
#endif
    connect(full,&QPushButton::clicked,dlg,[this]{openPlanner();});
    connect(close,&QPushButton::clicked,dlg,&QDialog::close);
    refreshConfigurationList();
    dlg->show();
}

void DesktopOverlays::edit(const QString& id) {
    const int index=indexOf(id);
    OverlayConfig c=index>=0?configs_[index]:defaultConfig("current");
    QDialog dlg(configurationDialog_?configurationDialog_.data():nullptr);
    dlg.setWindowTitle(index<0?QStringLiteral("创建桌面卡片"):QStringLiteral("桌面卡片设置"));
    dlg.resize(470,550);
    dlg.setMinimumWidth(450);
    dlg.setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* root=new QVBoxLayout(&dlg);
    auto* tabs=new QTabWidget(&dlg);
    root->addWidget(tabs,1);

    auto* basics=new QWidget;
    auto* form=new QFormLayout(basics);
    form->setSpacing(12);
    auto* title=new QLineEdit(c.title);
    auto* type=new QComboBox;
    for(int i=0;i<types.size();++i) type->addItem(names[i],types[i]);
    type->setCurrentIndex(std::max(0,int(types.indexOf(c.type))));
    auto* theme=new QComboBox;
    theme->addItem(QStringLiteral("柔和浅色"),"light");
    theme->addItem(QStringLiteral("夜间深色"),"dark");
    theme->setCurrentIndex(c.theme=="dark"?1:0);
    auto* fontSize=new QSpinBox;
    fontSize->setRange(8,35);fontSize->setValue(c.fontSize);
    auto* alpha=new QSpinBox;
    alpha->setRange(30,100);alpha->setSuffix("%");alpha->setValue(c.opacity);
    auto* visible=new QCheckBox(QStringLiteral("启动时显示"));
    visible->setChecked(c.visible);
    auto* top=new QCheckBox(QStringLiteral("始终置顶"));
    top->setChecked(c.topmost);
    auto* lock=new QCheckBox(QStringLiteral("锁定位置与大小"));
    lock->setChecked(c.locked);
    auto* details=new QCheckBox(QStringLiteral("单击弹出详情"));
    details->setChecked(c.showDetailsOnClick);
    form->addRow(QStringLiteral("窗口标题"),title);
    form->addRow(QStringLiteral("信息类型"),type);
    form->addRow(QStringLiteral("窗口主题"),theme);
    form->addRow(QStringLiteral("字体大小"),fontSize);
    form->addRow(QStringLiteral("背景不透明度"),alpha);
    form->addRow(visible);
    form->addRow(top);
    form->addRow(lock);
    form->addRow(details);
    auto* explanation=new QLabel(QStringLiteral("折叠后的卡片可以小到 125 × 44。"
        "\n桌面直接拖动移动，拖右下角改变尺寸。"));
    explanation->setWordWrap(true);
    form->addRow(explanation);
    tabs->addTab(basics,QStringLiteral("外观与行为"));

    auto* layoutTab=new QWidget;
    auto* layoutForm=new QFormLayout(layoutTab);
    layoutForm->setSpacing(12);
    auto* collapsed=new QCheckBox(QStringLiteral("使用紧凑折叠样式"));
    collapsed->setChecked(c.collapsed);
    auto* showTitle=new QCheckBox(QStringLiteral("紧凑模式显示窗口标题"));
    showTitle->setChecked(c.compactShowTitle);
    auto* compactWidth=new QSpinBox;compactWidth->setRange(125,900);
    compactWidth->setValue(std::max(125,c.compactGeometry.width()));
    auto* compactHeight=new QSpinBox;compactHeight->setRange(44,360);
    compactHeight->setValue(std::max(44,c.compactGeometry.height()));
    auto* expandedWidth=new QSpinBox;expandedWidth->setRange(190,900);
    expandedWidth->setValue(std::max(190,c.expandedGeometry.width()));
    auto* expandedHeight=new QSpinBox;expandedHeight->setRange(102,600);
    expandedHeight->setValue(std::max(102,c.expandedGeometry.height()));
    layoutForm->addRow(collapsed);
    layoutForm->addRow(showTitle);
    layoutForm->addRow(QStringLiteral("折叠宽度"),compactWidth);
    layoutForm->addRow(QStringLiteral("折叠高度"),compactHeight);
    layoutForm->addRow(QStringLiteral("展开宽度"),expandedWidth);
    layoutForm->addRow(QStringLiteral("展开高度"),expandedHeight);
    auto* hint=new QLabel(QStringLiteral("折叠只显示关键文字，展开时显示标题、时间、地点及提示。"
        "\n两种尺寸分别记忆，右键即可随时切换。"));
    hint->setWordWrap(true);
    layoutForm->addRow(hint);
    tabs->addTab(layoutTab,QStringLiteral("折叠与尺寸"));

    auto* dataTab=new QWidget;
    auto* dataForm=new QFormLayout(dataTab);
    auto* note=new QLineEdit(c.note);
    note->setPlaceholderText(QStringLiteral("便签文本 / 组合卡片自定义备注"));
    auto* items=new QLineEdit(c.combinedItems);
    items->setPlaceholderText("clock,date,current,next,week,duty,countdown,note");
    auto* cycle=new QCheckBox(QStringLiteral("逐项轮播（兼容旧版按顺序显示多个信息）"));
    cycle->setChecked(c.cycleItems);
    auto* seconds=new QSpinBox;
    seconds->setRange(2,300);seconds->setSuffix(QStringLiteral(" 秒"));seconds->setValue(c.cycleSeconds);
    seconds->setEnabled(c.cycleItems);
    connect(cycle,&QCheckBox::toggled,seconds,&QWidget::setEnabled);
    auto* tokens=new QLabel(QStringLiteral("组合信息可按顺序输入，用英文逗号隔开：\n"
        "clock 时间 · date 日期 · current 当前课程 · next 下一节 · week 教学周\n"
        "duty 每日值日 · countdown 倒计日 · note 备注"));
    tokens->setWordWrap(true);
    auto* target=new QDateTimeEdit(c.target.isValid()?c.target:QDateTime::currentDateTime().addDays(30));
    target->setCalendarPopup(true);target->setDisplayFormat("yyyy-MM-dd HH:mm");
    auto* editDuty=new QPushButton(QStringLiteral("编辑每周值日信息"));
    dataForm->addRow(QStringLiteral("备注文字"),note);
    dataForm->addRow(QStringLiteral("组合信息顺序"),items);
    dataForm->addRow(cycle);
    dataForm->addRow(QStringLiteral("轮播间隔"),seconds);
    dataForm->addRow(tokens);
    dataForm->addRow(QStringLiteral("倒计日目标"),target);
    dataForm->addRow(editDuty);
    connect(editDuty,&QPushButton::clicked,&dlg,[this]{editDutyNotes();});
    tabs->addTab(dataTab,QStringLiteral("显示内容"));

    // Updating the type from a built-in title should follow the selected template.
    connect(type,QOverload<int>::of(&QComboBox::currentIndexChanged),&dlg,
        [title,c,type](int selected){
            if(title->text()==c.title && c.title==typeName(c.type))
                title->setText(typeName(types.at(selected)));
        });

    auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    actions->button(QDialogButtonBox::Save)->setText(QStringLiteral("应用并保存"));
    actions->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    root->addWidget(actions);
    connect(actions,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    connect(actions,&QDialogButtonBox::accepted,&dlg,[&,this] {
        const QStringList allowed={"clock","date","current","next","week","duty","countdown","note"};
        for(const auto& raw:items->text().split(',',Qt::SkipEmptyParts)) {
            if(!allowed.contains(raw.trimmed().toLower())) {
                QMessageBox::warning(&dlg,QStringLiteral("不支持的组合字段"),
                    QStringLiteral("无法识别「%1」，请使用“显示内容”中列出的字段。").arg(raw.trimmed()));
                tabs->setCurrentWidget(dataTab);
                return;
            }
        }
        if(index>=0) {
            if(auto* w=windows_.value(c.id,nullptr)) c.geometry=w->geometry();
            if(c.collapsed) c.compactGeometry=c.geometry;
            else c.expandedGeometry=c.geometry;
        }
        const QPoint origin=c.geometry.topLeft();
        c.title=title->text().trimmed();
        c.type=type->currentData().toString();
        c.theme=theme->currentData().toString();
        c.fontSize=fontSize->value();
        c.opacity=alpha->value();
        c.visible=visible->isChecked();
        c.topmost=top->isChecked();
        c.locked=lock->isChecked();
        c.showDetailsOnClick=details->isChecked();
        c.compactShowTitle=showTitle->isChecked();
        c.combinedItems=items->text().trimmed();
        c.cycleItems=cycle->isChecked();
        c.cycleSeconds=seconds->value();
        c.note=note->text();
        c.target=target->dateTime();
        c.compactGeometry=QRect(origin,QSize(compactWidth->value(),compactHeight->value()));
        c.expandedGeometry=QRect(origin,QSize(expandedWidth->value(),expandedHeight->value()));
        c.collapsed=collapsed->isChecked();
        c.geometry=c.collapsed?c.compactGeometry:c.expandedGeometry;
        if(index>=0) configs_[index]=c;
        else configs_.append(c);
        save();rebuild();dlg.accept();
    });
    dlg.exec();
}

void DesktopOverlays::showAll() {
    for(auto& c:configs_) c.visible=true;
    save();rebuild();
}
void DesktopOverlays::hideAll() {
    for(auto& c:configs_) c.visible=false;
    save();rebuild();
}
void DesktopOverlays::importLegacyWindows() {
    const QString path=QFileDialog::getOpenFileName(configurationDialog_.data(),
        QStringLiteral("导入旧版 TimeTable-Qt 窗口布局"),
        QString(),QStringLiteral("旧版配置文件 (*.json)"));
    if(path.isEmpty()) return;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(configurationDialog_.data(),QStringLiteral("打开失败"),file.errorString());
        return;
    }
    QByteArray bytes=file.readAll();
    if(bytes.startsWith(QByteArray::fromHex("EFBBBF"))) bytes.remove(0,3);
    QJsonParseError error;
    const auto parsed=QJsonDocument::fromJson(bytes,&error);
    if(!parsed.isObject()) {
        QMessageBox::warning(configurationDialog_.data(),QStringLiteral("JSON 格式错误"),error.errorString());
        return;
    }
    const auto root=parsed.object();
    auto legacyWindows=root.value("Windows").toArray();
    if(legacyWindows.isEmpty() && root.value("MainWindow").isObject())
        legacyWindows.append(root.value("MainWindow"));
    if(legacyWindows.isEmpty()) {
        QMessageBox::warning(configurationDialog_.data(),QStringLiteral("配置无法识别"),
            QStringLiteral("找不到旧版 Windows 或 MainWindow 数组。"));
        return;
    }
    QVector<OverlayConfig> imported;
    int skipped=0;
    for(const auto& rawWindow:legacyWindows) {
        const auto legacy=rawWindow.toObject();
        const auto elements=legacy.value("UIElements").toArray();
        if(elements.isEmpty()) {++skipped;continue;}
        QStringList tokens;
        QDateTime target;
        QString note;
        for(const auto& e:elements) {
            const auto payload=e.toObject().value("Data").toArray();
            for(const auto& entry:payload) {
                const auto data=entry.toObject();
                const int kind=data.value("Type").toInt(-1);
                QString token;
                if(kind==1) token="clock";
                else if(kind==2) token="current";
                else if(kind==3||kind==4) token="next";
                else if(kind==5) {
                    token="countdown";
                    const auto dateFields=data.value("Data").toObject().value("TargetTime").toArray();
                    if(dateFields.size()>=3) {
                        const QDate date(dateFields.at(0).toInt(),dateFields.at(1).toInt(),dateFields.at(2).toInt());
                        const QTime time(dateFields.size()>3?dateFields.at(3).toInt():0,
                            dateFields.size()>4?dateFields.at(4).toInt():0,
                            dateFields.size()>5?dateFields.at(5).toInt():0);
                        if(date.isValid() && time.isValid()) target=QDateTime(date,time);
                    }
                } else if(kind==6) token="duty";
                else if(kind==0 && !data.value("TextFormat").toString().isEmpty()) {
                    token="note";note=data.value("TextFormat").toString();
                } else {++skipped;continue;}
                if(!tokens.contains(token)) tokens<<token;
            }
        }
        if(tokens.isEmpty()) {++skipped;continue;}
        OverlayConfig c=defaultConfig(tokens.size()==1?tokens.first():"combined");
        c.title=legacy.value("Name").toString().trimmed();
        if(c.title.isEmpty()) c.title=QStringLiteral("旧版窗口 %1").arg(imported.size()+1);
        c.combinedItems=tokens.join(",");
        c.note=note;
        if(target.isValid()) c.target=target;
        const auto location=legacy.value("WindowLocation").toArray();
        const auto dims=legacy.value("WindowSize").toArray();
        const QPoint position(location.size()>=2 ? location[0].toInt(c.geometry.x()) : c.geometry.x(),
                              location.size()>=2 ? location[1].toInt(c.geometry.y()) : c.geometry.y());
        c.compactGeometry.moveTopLeft(position);
        c.expandedGeometry.moveTopLeft(position);
        if(dims.size()>=2) {
            c.expandedGeometry.setSize(QSize(std::clamp(dims[0].toInt(),190,900),
                                              std::clamp(dims[1].toInt(),102,600)));
        }
        c.collapsed=true;
        c.geometry=c.compactGeometry;
        c.visible=legacy.value("AutoOpen").toBool(true);
        c.topmost=legacy.value("TopMost").toBool(true);
        c.locked=!legacy.value("Moveable").toBool(true);
        imported<<c;
    }
    if(imported.isEmpty()) {
        QMessageBox::information(configurationDialog_.data(),QStringLiteral("无可迁移窗口"),
            QStringLiteral("旧配置中的窗口没有可识别的信息元素，原有设置没有变化。"));
        return;
    }
    const QString message=QStringLiteral("识别出 %1 个桌面窗口，跳过 %2 个不兼容信息项。\n"
        "导入会添加到现有窗口列表中，不会覆盖课程文件。是否继续？")
        .arg(imported.size()).arg(skipped);
    if(QMessageBox::question(configurationDialog_.data(),QStringLiteral("确认导入窗口"),message)
       !=QMessageBox::Yes) return;
    for(const auto& config:imported) configs_.append(config);
    save();rebuild();
}
void DesktopOverlays::editDutyNotes() {
    QDialog dlg(configurationDialog_?configurationDialog_.data():nullptr);
    dlg.setWindowTitle(QStringLiteral("每周值日 / 每日提醒"));
    dlg.setMinimumWidth(430);
    dlg.setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* content=new QVBoxLayout(&dlg);
    auto* intro=new QLabel(QStringLiteral("为每个星期设置一条值日、值班或日常提醒；留空则不显示。"
        "\n可以使用“每日信息”卡片，也可插入到“组合信息”卡片。"));
    intro->setWordWrap(true);
    content->addWidget(intro);
    auto* form=new QFormLayout;
    const QStringList days={QStringLiteral("周一"),QStringLiteral("周二"),QStringLiteral("周三"),
        QStringLiteral("周四"),QStringLiteral("周五"),QStringLiteral("周六"),QStringLiteral("周日")};
    QVector<QLineEdit*> fields;
    QSettings settings;
    for(int day=1;day<=7;++day) {
        auto* field=new QLineEdit(settings.value("desktop/duty/"+QString::number(day)).toString());
        field->setPlaceholderText(QStringLiteral("例如：值日 A 组 / 晚自习"));
        form->addRow(days[day-1],field);fields<<field;
    }
    content->addLayout(form);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    content->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::accepted,&dlg,[this,&dlg,fields] {
        QSettings settings;
        for(int day=1;day<=7;++day)
            settings.setValue("desktop/duty/"+QString::number(day),fields[day-1]->text().trimmed());
        refresh();
        dlg.accept();
    });
    dlg.exec();
}
void DesktopOverlays::remove(const QString& id) {
    const int index=indexOf(id);
    if(index<0) return;
    if(QMessageBox::question(configurationDialog_?configurationDialog_.data():nullptr,
        QStringLiteral("删除桌面信息窗口"),
        QStringLiteral("删除「%1」？此操作不会删除课程数据。").arg(nameOrFallback(configs_[index])))!=QMessageBox::Yes)
        return;
    configs_.removeAt(index);
    save();rebuild();
}
} // namespace tt
