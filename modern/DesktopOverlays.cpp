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
#include <QTimer>
#include <QVBoxLayout>
#include <QUuid>
#include <algorithm>

namespace tt {
namespace {
const QStringList types={"clock","current","next","today","week","countdown","note"};
const QStringList names={QStringLiteral("时钟与日期"),QStringLiteral("当前课程"),QStringLiteral("下一节课程"),
                         QStringLiteral("今日课程"),QStringLiteral("教学周信息"),
                         QStringLiteral("目标日倒计时"),QStringLiteral("自定义便签")};
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
        setAttribute(Qt::WA_Hover);
        setMouseTracking(true);
        setWindowTitle(nameOrFallback(c));
        auto flags=Qt::Tool|Qt::FramelessWindowHint;
        if(c.topmost) flags|=Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        setMinimumSize(190,100);
        auto* content=new QVBoxLayout(this);
        content->setContentsMargins(18,14,18,15);
        content->setSpacing(7);
        heading_=overlayLabel(this);
        heading_->setFont(QFont(font().family(),10,QFont::DemiBold));
        main_=overlayLabel(this);
        secondary_=overlayLabel(this);
        footer_=overlayLabel(this);
        content->addWidget(heading_);
        content->addWidget(main_,1);
        content->addWidget(secondary_);
        content->addWidget(footer_);
        updateModel();
        setGeometry(c.geometry);
        setWindowOpacity(1.0);
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
        footer_->setStyleSheet("color:#766AE6;background:transparent;");
        auto font=main_->font();font.setPointSize(config_.fontSize+3);font.setBold(true);
        main_->setFont(font);
        auto f=secondary_->font();f.setPointSize(std::max(9,config_.fontSize-3));secondary_->setFont(f);
        auto current=store.currentCourse(now);
        Course next;QDateTime when;
        bool hasNext=store.nextCourse(now,&next,&when);
        QString big,small,foot;
        if(config_.type=="clock") {
            big=now.toString("HH:mm:ss");
            small=now.toString("yyyy-MM-dd  dddd");
            foot=QStringLiteral("双击打开课表 · 右键配置");
        } else if(config_.type=="current") {
            big=current?current->name:QStringLiteral("当前无课程");
            small=current ? QString("%1–%2  %3").arg(current->start.toString("HH:mm"),
                current->end.toString("HH:mm"),current->room) :
                QStringLiteral("点击查看今日完整课程");
            foot=current?QStringLiteral("距离下课 %1").arg(timeLeft(now.secsTo(QDateTime(now.date(),current->end))))
                :QStringLiteral("第 %1 周").arg(store.weekNumber(now.date()));
        } else if(config_.type=="next") {
            big=hasNext?next.name:QStringLiteral("暂无下一节课");
            small=hasNext?when.toString("MM-dd dddd HH:mm")+"  "+next.room:
                QStringLiteral("未来 15 天没有安排课程");
            foot=hasNext?QStringLiteral("距离开始 %1").arg(timeLeft(now.secsTo(when))):QString();
        } else if(config_.type=="today") {
            const auto list=store.coursesForDate(now.date());
            big=list.isEmpty()?QStringLiteral("今天没有课程"):QStringLiteral("今天 %1 节课程").arg(list.size());
            QStringList details;
            for(int i=0;i<std::min(2,int(list.size()));++i)
                details << list[i].start.toString("HH:mm")+" "+list[i].name;
            small=details.join("  /  ");
            foot=QStringLiteral("点击展开今天的安排");
        } else if(config_.type=="week") {
            const int week=store.weekNumber(now.date());
            big=week?QStringLiteral("第 %1 / %2 周").arg(week).arg(store.totalWeeks):
                QStringLiteral("当前不在学期内");
            small=QStringLiteral("本学期从 %1 开始").arg(store.semesterMonday.toString("yyyy-MM-dd"));
            foot=QStringLiteral("点击查看本周课程");
        } else if(config_.type=="countdown") {
            big=timeLeft(now.secsTo(config_.target));
            small=config_.target.isValid()?config_.target.toString("yyyy-MM-dd HH:mm"):
                QStringLiteral("请在窗口设置中选择目标日期");
            foot=QStringLiteral("目标倒计时");
        } else {
            big=config_.note.isEmpty()?QStringLiteral("点击右键编辑便签"):config_.note;
            small=QString();
            foot=QStringLiteral("自定义桌面信息");
        }
        if(main_->text()!=big) main_->setText(big);
        if(secondary_->text()!=small) secondary_->setText(small);
        if(footer_->text()!=foot) footer_->setText(foot);
        footer_->setVisible(!foot.isEmpty());
        secondary_->setVisible(!small.isEmpty());
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
        const bool dark=config_.theme=="dark";
        QColor base=dark?QColor("#202535"):QColor("#FDFEFF");
        base.setAlphaF(config_.opacity/100.0);
        p.setBrush(base);p.setPen(QPen(dark?QColor("#485069"):QColor("#DCE1ED"),1));
        p.drawRoundedRect(rect().adjusted(1,1,-2,-2),17,17);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#8170F8"));
        p.drawRoundedRect(QRect(15,18,3,15),1.5,1.5);
    }
    void mousePressEvent(QMouseEvent* ev) override {
        if(ev->button()==Qt::LeftButton) {
            down_=ev->globalPosition().toPoint();
            startPos_=pos();
            pressed_=true;moved_=false;
            ev->accept();return;
        }
        QWidget::mousePressEvent(ev);
    }
    void mouseMoveEvent(QMouseEvent* ev) override {
        if(pressed_ && !config_.locked && (ev->buttons()&Qt::LeftButton)) {
            const QPoint delta=ev->globalPosition().toPoint()-down_;
            if(delta.manhattanLength()>5) moved_=true;
            if(moved_) move(startPos_+delta);
            ev->accept();return;
        }
        QWidget::mouseMoveEvent(ev);
    }
    void mouseReleaseEvent(QMouseEvent* ev) override {
        if(ev->button()==Qt::LeftButton && pressed_) {
            pressed_=false;
            if(moved_) { ++clickSerial_; manager_->moved(config_.id,geometry()); }
            else if(config_.showDetailsOnClick) {
                // Do not open a popup on the first click of a double-click sequence.
                const int serial=++clickSerial_;
                const QPoint anchor=ev->globalPosition().toPoint();
                QTimer::singleShot(QApplication::doubleClickInterval(),this,[this,serial,anchor] {
                    if(serial==clickSerial_) manager_->showDetail(config_.id,anchor);
                });
            }
            ev->accept();return;
        }
        QWidget::mouseReleaseEvent(ev);
    }
    void mouseDoubleClickEvent(QMouseEvent* ev) override {
        if(ev->button()==Qt::LeftButton) {
            ++clickSerial_;
            manager_->openPlanner(config_.type=="week"?1:0);
            ev->accept();return;
        }
        QWidget::mouseDoubleClickEvent(ev);
    }
    void contextMenuEvent(QContextMenuEvent* ev) override {
        manager_->contextMenu(config_.id,ev->globalPos());
        ev->accept();
    }

private:
    OverlayConfig config_;
    DesktopOverlays* manager_;
    QLabel* heading_;
    QLabel* main_;
    QLabel* secondary_;
    QLabel* footer_;
    bool pressed_=false,moved_=false;
    int clickSerial_=0;
    QPoint down_,startPos_;
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
    c.geometry=QRect(screen.x()+25,screen.y()+25,310,150);
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
                config.geometry.width()<160||config.geometry.height()<85 ||
                config.opacity<30||config.opacity>100 || config.fontSize<8||config.fontSize>35)
                continue;
            if(config.theme!="dark"&&config.theme!="light") config.theme="light";
            configs_.append(config);
        }
    } else {
        auto clock=defaultConfig("clock");clock.geometry.setSize(QSize(260,140));
        auto current=defaultConfig("current");current.geometry.translate(275,0);
        auto next=defaultConfig("next");next.geometry.translate(600,0);
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
    rebuild();
    planner_->setOnScheduleChanged([this]{refresh();});
    auto* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this]{refresh();});
    timer->start(1000);
}
void DesktopOverlays::rebuild() {
    qDeleteAll(windows_);windows_.clear();
    for(const auto& c:configs_) {
        auto* w=new OverlayCard(c,this);
        windows_.insert(c.id,w);
        if(c.visible) {
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
    save();
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
    QAction* editWindow=menu.addAction(QStringLiteral("设置此窗口"));
    QAction* manage=menu.addAction(QStringLiteral("管理全部桌面窗口"));
    QAction* add=menu.addAction(QStringLiteral("新建桌面窗口"));
    menu.addSeparator();
    QAction* top=menu.addAction(c.topmost?QStringLiteral("取消置顶"):QStringLiteral("始终置顶"));
    QAction* lock=menu.addAction(c.locked?QStringLiteral("解锁拖动"):QStringLiteral("锁定位置"));
    QAction* hide=menu.addAction(QStringLiteral("隐藏此窗口"));
    QAction* removeWindow=menu.addAction(QStringLiteral("删除此窗口"));
    menu.addSeparator();
    QAction* quit=menu.addAction(QStringLiteral("退出 TimeTable Next"));
    QAction* selected=menu.exec(anchor);
    if(!selected) return;
    if(selected==details) showDetail(id,anchor);
    else if(selected==planner) openPlanner();
    else if(selected==editWindow) edit(id);
    else if(selected==manage) openConfiguration();
    else if(selected==add) edit();
    else if(selected==top) { configs_[index].topmost=!c.topmost;save();rebuild(); }
    else if(selected==lock) { configs_[index].locked=!c.locked;save();rebuild(); }
    else if(selected==hide) { configs_[index].visible=false;save();rebuild();openConfiguration(); }
    else if(selected==removeWindow) remove(id);
    else if(selected==quit) QApplication::quit();
}
void DesktopOverlays::refreshConfigurationList() {
    if(!configurationList_) return;
    configurationList_->clear();
    for(const auto& c:configs_)
        configurationList_->addItem(QString("%1  ·  %2  ·  %3")
            .arg(c.visible?QStringLiteral("●"):QStringLiteral("○"),
                 nameOrFallback(c),typeName(c.type)));
}
void DesktopOverlays::openConfiguration() {
    if(configurationDialog_) {
        configurationDialog_->raise();
        configurationDialog_->activateWindow();
        return;
    }
    auto* dlg=new QDialog(nullptr,Qt::Window);
    configurationDialog_=dlg;
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("桌面窗口管理 · TimeTable Next"));
    dlg->setMinimumSize(620,460);
    dlg->setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* layout=new QVBoxLayout(dlg);
    layout->setContentsMargins(22,20,22,20);layout->setSpacing(12);
    auto* title=new QLabel(QStringLiteral("独立桌面窗口"));
    auto font=title->font();font.setPointSize(17);font.setBold(true);title->setFont(font);
    layout->addWidget(title);
    auto* hint=new QLabel(QStringLiteral("每个窗口可独立置顶、拖动、锁定、隐藏和配置。\n左键展开信息，双击进入完整课表，右键访问快捷菜单。"));
    hint->setWordWrap(true);layout->addWidget(hint);
    auto* list=new QListWidget;configurationList_=list;
    layout->addWidget(list,1);
    auto* buttons=new QHBoxLayout;
    auto* add=new QPushButton(QStringLiteral("＋ 新建"));
    auto* editButton=new QPushButton(QStringLiteral("编辑选中"));
    auto* toggle=new QPushButton(QStringLiteral("显示 / 隐藏"));
    auto* removeButton=new QPushButton(QStringLiteral("删除"));
    buttons->addWidget(add);buttons->addWidget(editButton);buttons->addWidget(toggle);buttons->addWidget(removeButton);
    layout->addLayout(buttons);
    auto* footer=new QHBoxLayout;
    auto* full=new QPushButton(QStringLiteral("进入完整课程管理器"));
    auto* close=new QPushButton(QStringLiteral("关闭"));
    footer->addWidget(full);footer->addStretch();footer->addWidget(close);
    layout->addLayout(footer);
    const auto selectedIndex=[list] { return list->currentRow(); };
    connect(add,&QPushButton::clicked,dlg,[this]{edit();});
    connect(editButton,&QPushButton::clicked,dlg,[this,selectedIndex]{
        int index=selectedIndex();if(index>=0&&index<configs_.size()) edit(configs_[index].id);
    });
    connect(list,&QListWidget::itemDoubleClicked,dlg,[this,list](QListWidgetItem*){
        int index=list->currentRow();if(index>=0&&index<configs_.size()) edit(configs_[index].id);
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
    connect(full,&QPushButton::clicked,dlg,[this]{openPlanner();});
    connect(close,&QPushButton::clicked,dlg,&QDialog::close);
    refreshConfigurationList();
    dlg->show();
}

void DesktopOverlays::edit(const QString& id) {
    const int index=indexOf(id);
    OverlayConfig c=index>=0?configs_[index]:defaultConfig("current");
    QDialog dlg;
    dlg.setWindowTitle(index<0?QStringLiteral("添加桌面信息窗口"):QStringLiteral("编辑桌面信息窗口"));
    dlg.setMinimumWidth(420);
    dlg.setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* content=new QVBoxLayout(&dlg);
    auto* form=new QFormLayout;
    auto* title=new QLineEdit(c.title);
    auto* type=new QComboBox;
    for(int i=0;i<types.size();++i) type->addItem(names[i],types[i]);
    type->setCurrentIndex(std::max(0,int(types.indexOf(c.type))));
    auto* theme=new QComboBox;
    theme->addItem(QStringLiteral("柔和浅色"),"light");
    theme->addItem(QStringLiteral("夜间深色"),"dark");
    theme->setCurrentIndex(c.theme=="dark"?1:0);
    auto* size=new QSpinBox;size->setRange(8,35);size->setValue(c.fontSize);
    auto* width=new QSpinBox;width->setRange(190,850);width->setValue(c.geometry.width());
    auto* height=new QSpinBox;height->setRange(100,480);height->setValue(c.geometry.height());
    auto* alpha=new QSpinBox;alpha->setRange(30,100);alpha->setSuffix("%");alpha->setValue(c.opacity);
    auto* note=new QLineEdit(c.note);
    auto* target=new QDateTimeEdit(c.target.isValid()?c.target:QDateTime::currentDateTime().addDays(30));
    target->setCalendarPopup(true);target->setDisplayFormat("yyyy-MM-dd HH:mm");
    auto* visible=new QCheckBox(QStringLiteral("启动时显示"));visible->setChecked(c.visible);
    auto* top=new QCheckBox(QStringLiteral("保持在其他窗口之上"));top->setChecked(c.topmost);
    auto* lock=new QCheckBox(QStringLiteral("锁定位置（不可拖动）"));lock->setChecked(c.locked);
    auto* details=new QCheckBox(QStringLiteral("单击弹出更多内容"));details->setChecked(c.showDetailsOnClick);
    form->addRow(QStringLiteral("窗口名称"),title);
    form->addRow(QStringLiteral("信息类型"),type);
    form->addRow(QStringLiteral("独立配色"),theme);
    form->addRow(QStringLiteral("文字大小"),size);
    form->addRow(QStringLiteral("窗口宽度"),width);
    form->addRow(QStringLiteral("窗口高度"),height);
    form->addRow(QStringLiteral("背景不透明度"),alpha);
    form->addRow(QStringLiteral("便签文本"),note);
    form->addRow(QStringLiteral("倒计时目标"),target);
    form->addRow(visible);form->addRow(top);form->addRow(lock);form->addRow(details);
    content->addLayout(form);
    auto* bar=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    bar->button(QDialogButtonBox::Save)->setText(QStringLiteral("保存窗口"));
    bar->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    content->addWidget(bar);
    connect(bar,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    connect(bar,&QDialogButtonBox::accepted,&dlg,[&]{
        c.title=title->text().trimmed();
        c.type=type->currentData().toString();
        c.theme=theme->currentData().toString();
        c.fontSize=size->value();
        c.geometry.setSize(QSize(width->value(),height->value()));
        c.opacity=alpha->value();
        c.note=note->text();
        c.target=target->dateTime();
        c.visible=visible->isChecked();
        c.topmost=top->isChecked();
        c.locked=lock->isChecked();
        c.showDetailsOnClick=details->isChecked();
        if(index>=0) configs_[index]=c;
        else configs_.append(c);
        save();rebuild();dlg.accept();
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
