#include "DesktopOverlays.h"
#include "Translation.h"
#include "DesktopLayoutStore.h"
#include "WindowEffects.h"
#include "QtCompat.h"
#include "ElementEditor.h"
#include "MainWindow.h"
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QFont>
#include <QFrame>
#include <QDateTimeEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QPixmap>
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
#include <QPainterPath>
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
#include <QElapsedTimer>
#include <QHideEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QUuid>
#include <algorithm>
#include <memory>

namespace tt {
namespace {
const QStringList types={"clock","current","next","today","week","countdown","note","duty","combined","elements"};
QStringList typeNames() { return {tt::translate("时钟与日期"),tt::translate("当前课程"),tt::translate("下一节课程"),
                         tt::translate("今日课程"),tt::translate("教学周信息"),
                         tt::translate("目标日倒计时"),tt::translate("自定义便签"),tt::translate("每日信息 / 值日"),tt::translate("组合信息栏"),tt::translate("自由元素布局")}; }
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
    return typeNames().value(i,tt::translate("信息组件"));
}
QString timeLeft(qint64 seconds) {
    if(seconds < 0) return tt::translate("已结束");
    const qint64 day=seconds/86400, hour=seconds%86400/3600, minute=seconds%3600/60;
    if(day) return tt::translate("%1 天 %2 小时").arg(day).arg(hour);
    if(hour) return tt::translate("%1 小时 %2 分").arg(hour).arg(minute);
    return tt::translate("%1 分 %2 秒").arg(minute).arg(seconds%60);
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
        setToolTip(tt::translate("拖动移动 · 拖右下角缩放 · 右键折叠/配置 · 单击详情"));
        auto flags=Qt::Tool|Qt::FramelessWindowHint;
        if(c.topmost) flags|=Qt::WindowStaysOnTopHint;
        setWindowFlags(flags);
        animationClock_.start();
        animationTimer_=new QTimer(this);
        animationTimer_->setObjectName("elementAnimationTimer");
        animationTimer_->setTimerType(Qt::PreciseTimer);
        animationTimer_->setInterval(c.fps>0?(1000+c.fps-1)/c.fps:ElementLayout::AnimationIntervalMs);
        connect(animationTimer_,&QTimer::timeout,this,QOverload<>::of(&QWidget::update));
        cycleTimer_=new QTimer(this);
        cycleTimer_->setObjectName("elementCycleTimer");
        cycleTimer_->setSingleShot(true);
        cycleTimer_->setTimerType(Qt::PreciseTimer);
        connect(cycleTimer_,&QTimer::timeout,this,[this]{updateModel();});
        if(c.imageBackground) {
            if(!c.backgroundAsset.isEmpty())backgroundPixmap_.loadFromData(c.backgroundAsset);
            else if(!c.backgroundImage.isEmpty())backgroundPixmap_.load(c.backgroundImage);
        }
        setMinimumSize(c.type=="elements"?QSize(1,1):(c.collapsed?QSize(125,44):QSize(190,102)));
        auto* content=new QVBoxLayout(this);
        if(c.type=="elements")content->setSizeConstraint(QLayout::SetNoConstraint);
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
        if(config_.type=="elements") {
            for(auto* child:{heading_,main_,secondary_,footer_})child->hide();
        }
        updateModel();
    }

    void updateModel() {
        if(config_.type=="elements") {
            if(config_.autoSize && !config_.collapsed && !config_.elements.isEmpty()) {
                const QRect bounds=ElementLayout::contentBounds(config_.elements,manager_->planner_->schedule(),QDateTime::currentDateTime());
                const QSize size(std::clamp(bounds.right()+1+2*config_.elementInset,1,16384),
                                 std::clamp(bounds.bottom()+1+2*config_.elementInset,1,16384));
                if(size!=this->size()) {
                    config_.geometry.setSize(size);config_.expandedGeometry.setSize(size);
                    resize(size);
                    manager_->autoResized(config_.id,geometry());
                }
            }
            updateAnimationState();
            update();
            return;
        }
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
            foot=tt::translate("双击打开课表 · 右键设置");
        } else if(config_.type=="current") {
            big=current?current->name:tt::translate("当前无课程");
            smallText=current ? current->start.toString("HH:mm")+"–"+
                current->end.toString("HH:mm")+"  "+current->room :
                tt::translate("点击查看今日完整课程");
            foot=current?tt::translate("距离下课 %1").arg(timeLeft(now.secsTo(QDateTime(now.date(),current->end))))
                :tt::translate("第 %1 周").arg(store.weekNumber(now.date()));
        } else if(config_.type=="next") {
            big=hasNext?next.name:tt::translate("暂无下一节课");
            smallText=hasNext?when.toString("MM-dd dddd HH:mm")+"  "+next.room:
                tt::translate("本学期没有后续课程");
            foot=hasNext?tt::translate("距离开始 %1").arg(timeLeft(now.secsTo(when))):QString();
        } else if(config_.type=="today") {
            const auto today=store.coursesForDate(now.date());
            big=today.isEmpty()?tt::translate("今天没有课程"):tt::translate("今日 %1 节课").arg(today.size());
            QStringList summaries;
            for(int i=0;i<std::min(3,int(today.size()));++i)
                summaries << today[i].start.toString("HH:mm")+" "+today[i].name;
            smallText=summaries.join("\n");
            foot=tt::translate("点击展开完整课程");
        } else if(config_.type=="week") {
            const int week=store.weekNumber(now.date());
            big=week?tt::translate("第 %1/%2 周").arg(week).arg(store.totalWeeks):
                tt::translate("当前不在学期内");
            smallText=tt::translate("开学 %1").arg(store.semesterMonday.toString("yyyy-MM-dd"));
            foot=tt::translate("点击查看周课表");
        } else if(config_.type=="countdown") {
            big=config_.target.isValid()?timeLeft(now.secsTo(config_.target)):tt::translate("未设置目标");
            smallText=config_.target.isValid()?config_.target.toString("yyyy-MM-dd HH:mm"):
                tt::translate("请右键设置目标日期");
            foot=tt::translate("倒计日");
        } else if(config_.type=="duty") {
            const QString duty=store.dayInfos.value(now.date().dayOfWeek()).join(QStringLiteral(" · "));
            big=duty.isEmpty()?tt::translate("今日无提醒"):duty;
            smallText=now.toString("dddd");
            foot=tt::translate("右键可配置每周值日/提示");
        } else if(config_.type=="combined") {
            QStringList parts;
            for(const QString& token:config_.combinedItems.split(',',Qt::SkipEmptyParts)) {
                const QString key=token.trimmed().toLower();
                if(key=="clock") parts<<now.toString("HH:mm");
                else if(key=="date") parts<<now.toString("MM/dd");
                else if(key=="current") parts<<(current?current->name:tt::translate("无课"));
                else if(key=="next") parts<<(hasNext?tt::translate("下一节 ")+next.name:tt::translate("无后续课"));
                else if(key=="week") parts<<tt::translate("第%1周").arg(store.weekNumber(now.date()));
                else if(key=="duty") {
                    const QString duty=store.dayInfos.value(now.date().dayOfWeek()).join(QStringLiteral(" · "));
                    if(!duty.isEmpty()) parts<<duty;
                } else if(key=="countdown"&&config_.target.isValid()) parts<<timeLeft(now.secsTo(config_.target));
                else if(key=="note"&&!config_.note.isEmpty()) parts<<config_.note;
            }
            if(parts.isEmpty()) big=tt::translate("请配置组合信息");
            else if(config_.cycleItems) {
                const auto step=now.toSecsSinceEpoch()/std::max(2,config_.cycleSeconds);
                big=parts.at(int(step%parts.size()));
                foot=tt::translate("信息轮播 %1/%2").arg(int(step%parts.size())+1).arg(parts.size());
            } else big=parts.join("  ·  ");
            smallText=parts.join("\n");
            foot=tt::translate("右键编辑组合信息");
        } else {
            big=config_.note.isEmpty()?tt::translate("右键编辑便签"):config_.note;
            foot=tt::translate("自定义便签");
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
    void showEvent(QShowEvent* event) override {
        QWidget::showEvent(event);
        if(config_.acrylic)applyAcrylic(this,true,config_.backgroundColor);
        updateModel();
    }
    void hideEvent(QHideEvent* event) override {
        animationTimer_->stop();
        cycleTimer_->stop();
        QWidget::hideEvent(event);
    }
    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        updateAnimationState();
    }
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const bool dark=config_.theme=="dark";
        QColor background=config_.backgroundColor;
        if(!background.isValid()) background=dark?QColor("#202535"):QColor("#FDFEFF");
        background.setAlphaF(background.alphaF()*config_.opacity/100.0);
        p.setBrush(background);
        p.setPen(QPen(dark?QColor("#485069"):QColor("#DCE1ED"),1));
        const QRect surface=config_.cornerRadius==0?rect():rect().adjusted(1,1,-2,-2);
        if(config_.cornerRadius==0)p.setPen(Qt::NoPen);
        p.drawRoundedRect(surface,config_.cornerRadius,config_.cornerRadius);
        if(!backgroundPixmap_.isNull()) {
            QPainterPath clip;
            clip.addRoundedRect(surface,config_.cornerRadius,config_.cornerRadius);
            p.save();p.setClipPath(clip);
            p.setOpacity(config_.opacity/100.0);
            p.drawPixmap(rect(),backgroundPixmap_);
            p.restore();
        }
        if(config_.type=="elements") {
            ElementLayout::paint(p,elementArea(),config_.elements,
                manager_->planner_->schedule(),QDateTime::currentDateTime(),
                animationClock_.elapsed(),dark);
        } else if(config_.collapsed) {
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
        if(config_.debug) {
            p.save();p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor("#E04167"),1,Qt::DashLine));
            p.drawRect(rect().adjusted(0,0,-1,-1));
            for(const auto& element:config_.elements)
                p.drawRect(QRect(elementArea().topLeft()+element.rect.topLeft(),element.rect.size()));
            p.restore();
        }
        if(config_.showFPS) {
            ++paintCount_;
            const qint64 elapsed=animationClock_.elapsed();
            if(elapsed-lastFPSAt_>=1000) {
                measuredFPS_=qRound(1000.0*paintCount_/std::max<qint64>(1,elapsed-lastFPSAt_));
                paintCount_=0;lastFPSAt_=elapsed;
            }
            p.save();p.setPen(dark?Qt::white:Qt::black);p.setFont(QFont(font().family(),8));
            p.drawText(rect().adjusted(3,2,-3,-2),Qt::AlignRight|Qt::AlignTop,
                QStringLiteral("%1 FPS").arg(measuredFPS_));p.restore();
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
            down_=mouseGlobalPoint(event);
            originalGeometry_=geometry();
            pressed_=true; moved_=false;
            resizing_=!config_.locked && mouseLocalPoint(event).x()>width()-18 &&
                      mouseLocalPoint(event).y()>height()-18;
            event->accept();return;
        }
        QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent* event) override {
        if(pressed_ && (event->buttons()&Qt::LeftButton)) {
            const QPoint delta=mouseGlobalPoint(event)-down_;
            if(delta.manhattanLength()>4) moved_=true;
            if(moved_ && !config_.locked) {
                if(resizing_) resize(QSize(originalGeometry_.width()+delta.x(),
                                           originalGeometry_.height()+delta.y())
                                    .expandedTo(minimumSize()));
                else move(originalGeometry_.topLeft()+delta);
            }
            event->accept();return;
        }
        const bool overCorner=mouseLocalPoint(event).x()>width()-18 &&
                              mouseLocalPoint(event).y()>height()-18;
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
                const QPoint anchor=mouseGlobalPoint(event);
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
    QRect elementArea() const {
        const int inset=config_.elementInset;
        return rect().adjusted(inset,inset,-inset,-inset);
    }
    void updateAnimationState() {
        const bool visibleElements=isVisible() && config_.type=="elements";
        const QRect area=elementArea();
        const qint64 elapsed=animationClock_.elapsed();
        const bool active=visibleElements && config_.fps>0 &&
            ElementLayout::hasScrollingText(area,config_.elements,
                manager_->planner_->schedule(),QDateTime::currentDateTime(),elapsed);
        if(active && !animationTimer_->isActive())animationTimer_->start();
        else if(!active)animationTimer_->stop();
        // A short cycle item has no frame timer. Wake exactly at the next item
        // boundary, independently of the manager's one-second refresh phase.
        const int nextCycleMs=visibleElements?ElementLayout::nextChangeDelay(area,config_.elements,
            manager_->planner_->schedule(),QDateTime::currentDateTime(),elapsed):0;
        if(nextCycleMs)cycleTimer_->start(nextCycleMs);
        else cycleTimer_->stop();
    }
    OverlayConfig config_;
    DesktopOverlays* manager_;
    QLabel *heading_,*main_,*secondary_,*footer_;
    QString value_;
    QTimer* animationTimer_=nullptr;
    QTimer* cycleTimer_=nullptr;
    QElapsedTimer animationClock_;
    QPixmap backgroundPixmap_;
    bool pressed_=false,moved_=false,resizing_=false;
    int clickSerial_=0;
    int paintCount_=0,measuredFPS_=0;
    qint64 lastFPSAt_=0;
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
    configurationPath_=settings.value("desktop/configurationPath",DesktopLayoutStore::defaultPath()).toString();
    if(configurationPath_.isEmpty())configurationPath_=DesktopLayoutStore::defaultPath();
    QString error;
    if(QFileInfo::exists(configurationPath_)) {
        if(!DesktopLayoutStore::read(configurationPath_,&configs_,&error)) {
            storageWritable_=false;
            qWarning()<<"Desktop layout was not loaded; the original file is preserved:"<<error;
        }
    } else {
        const bool missingCustom=settings.contains("desktop/configurationPath") &&
            QDir::cleanPath(configurationPath_)!=QDir::cleanPath(DesktopLayoutStore::defaultPath());
        if(missingCustom) {
            storageWritable_=false;
            qWarning()<<"Custom desktop layout is missing; retained settings are shown and automatic saving is paused:"<<configurationPath_;
        }
        const auto previous=settings.value("desktop/windows").toByteArray();
        if(!previous.isEmpty()) {
            if(!DesktopLayoutStore::parse(previous,&configs_,&error)) {
                storageWritable_=false;
                qWarning()<<"Previous desktop settings were not migrated:"<<error;
            }
        } else if(!missingCustom) {
            auto clock=defaultConfig("clock");clock.compactGeometry.setSize(QSize(158,48));clock.geometry=clock.compactGeometry;
            auto current=defaultConfig("current");current.compactGeometry.moveTopLeft(clock.compactGeometry.topRight()+QPoint(12,0));current.geometry=current.compactGeometry;
            auto next=defaultConfig("next");next.compactGeometry.moveTopLeft(current.compactGeometry.topRight()+QPoint(12,0));next.geometry=next.compactGeometry;
            configs_={clock,current,next};
        }
        if(storageWritable_)save();
    }
    // Keep exact element card sizes, including narrow legacy strips.
    for(auto& config:configs_) {
        config.compactGeometry=visibleGeometry(config.compactGeometry);
        config.expandedGeometry=visibleGeometry(config.expandedGeometry);
        config.geometry=config.collapsed?config.compactGeometry:config.expandedGeometry;
    }
}
void DesktopOverlays::save() const {
    if(!storageWritable_)return;
    QString error;
    if(!DesktopLayoutStore::write(configurationPath_,configs_,&error)) {
        qWarning()<<"Desktop layout could not be saved:"<<error;
        return;
    }
    QJsonArray previous;
    for(const auto& config:configs_)previous.append(DesktopLayoutStore::toJson(config));
    QSettings settings;
    settings.setValue("desktop/configurationPath",configurationPath_);
    settings.setValue("desktop/windows",QJsonDocument(previous).toJson(QJsonDocument::Compact));
}
QString DesktopOverlays::configurationPath() const { return configurationPath_; }
bool DesktopOverlays::exportLayout(const QString& path,QString* error) const {
    return DesktopLayoutStore::write(path,configs_,error,true);
}
bool DesktopOverlays::restoreLayout(const QString& path,QString* error) {
    QVector<OverlayConfig> candidate;
    if(!DesktopLayoutStore::read(path,&candidate,error))return false;
    if(!DesktopLayoutStore::write(configurationPath_,candidate,error))return false;
    configs_=candidate;storageWritable_=true;temporarilyHiddenWindows_.clear();
    save();rebuild();return true;
}
bool DesktopOverlays::setConfigurationPath(const QString& path,QString* error) {
    if(path.trimmed().isEmpty()){if(error)*error=tt::translate("布局路径不能为空。");return false;}
    const QString absolute=QFileInfo(path).absoluteFilePath();
    QVector<OverlayConfig> candidate=configs_;
    if(QFileInfo::exists(absolute)) {
        if(!DesktopLayoutStore::read(absolute,&candidate,error))return false;
    } else if(!DesktopLayoutStore::write(absolute,candidate,error))return false;
    configurationPath_=absolute;configs_=candidate;storageWritable_=true;
    QSettings().setValue("desktop/configurationPath",configurationPath_);
    temporarilyHiddenWindows_.clear();rebuild();return true;
}
bool DesktopOverlays::shouldShow(const OverlayConfig& config) const {
    return !temporarilyHidden_ && !temporarilyHiddenWindows_.contains(config.id) &&
        (config.visible || temporarilyShowAll_) && !planner_->isVisible() &&
        !(configurationDialog_ && configurationDialog_->isVisible());
}
void DesktopOverlays::setStartupVisible(const QString& id,bool visible) {
    const int index=indexOf(id);if(index<0)return;
    configs_[index].visible=visible;save();syncVisibility();refreshConfigurationList();
}
void DesktopOverlays::autoResized(const QString& id,const QRect& geometry) {
    const int index=indexOf(id);if(index<0)return;
    auto& config=configs_[index];
    if(config.geometry==geometry)return;
    config.geometry=geometry;config.expandedGeometry=geometry;
    save();
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
        if(shouldShow(c)) {
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
    for(const auto& c:configs_) {
        if(auto* w=windows_.value(c.id,nullptr)) {
            if(shouldShow(c)) w->show();
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
            lines << d.toString("MM-dd dddd")+tt::translate(" · %1 节").arg(lessons.size());
            for(const auto& course:lessons)
                lines << "  "+course.start.toString("HH:mm")+" "+course.name+
                    (course.room.isEmpty()?"":" · "+course.room);
        }
    } else if(c.type=="next") {
        Course next;QDateTime start;
        if(store.nextCourse(now,&next,&start)) {
            lines << start.toString("yyyy-MM-dd dddd HH:mm")
                  << next.name
                  << tt::translate("结束 %1").arg(next.end.toString("HH:mm"))
                  << next.room << next.teacher
                  << tt::translate("有效周：%1").arg(ScheduleStore::formatWeeks(next.weeks));
        } else lines << tt::translate("未来 15 天无课程");
    } else {
        lines << date.toString("yyyy-MM-dd dddd");
        const auto lessons=store.coursesForDate(date);
        if(lessons.isEmpty()) lines << tt::translate("今天没有课程");
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
    auto* popup=new QDialog(nullptr,Qt::Popup|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
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
    auto* action=new QPushButton(tt::translate("打开完整课表与设置"));
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
    QAction* details=menu.addAction(tt::translate("展开此窗口详情"));
    QAction* planner=menu.addAction(tt::translate("打开完整课程管理器"));
    menu.addSeparator();
    QAction* collapse=menu.addAction(c.collapsed?tt::translate("展开卡片（显示完整信息）"):tt::translate("折叠成小卡片"));
    QAction* editWindow=menu.addAction(tt::translate("设置此窗口"));
    QAction* editLayout=menu.addAction(tt::translate("编辑自由元素布局"));
    QAction* manage=menu.addAction(tt::translate("管理全部桌面窗口"));
    QAction* add=menu.addAction(tt::translate("新建桌面窗口"));
    QAction* duty=menu.addAction(tt::translate("编辑每周值日 / 提示"));
    menu.addSeparator();
    QAction* top=menu.addAction(c.topmost?tt::translate("取消置顶"):tt::translate("始终置顶"));
    QAction* lock=menu.addAction(c.locked?tt::translate("解锁拖动"):tt::translate("锁定位置"));
    QAction* hide=menu.addAction(tt::translate("隐藏此窗口"));
    QAction* hideEverything=menu.addAction(tt::translate("隐藏全部桌面卡片（可从托盘恢复）"));
    QAction* removeWindow=menu.addAction(tt::translate("删除此窗口"));
    menu.addSeparator();
    QAction* quit=menu.addAction(tt::translate("退出 TimeTable Next"));
    QAction* selected=menu.exec(anchor);
    if(!selected) return;
    if(selected==details) showDetail(id,anchor);
    else if(selected==planner) openPlanner();
    else if(selected==collapse) toggleCollapsed(id);
    else if(selected==editWindow) edit(id);
    else if(selected==editLayout) editElements(id);
    else if(selected==manage) openConfiguration();
    else if(selected==add) edit();
    else if(selected==duty) editDutyNotes();
    else if(selected==top) { configs_[index].topmost=!c.topmost;save();rebuild(); }
    else if(selected==lock) { configs_[index].locked=!c.locked;save();rebuild(); }
    else if(selected==hide) { temporarilyHiddenWindows_.insert(id);syncVisibility(); }
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
                 nameOrFallback(c)+(c.collapsed?tt::translate(" [折叠]"):tt::translate(" [展开]")),typeName(c.type)));
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
    dlg->setWindowTitle(tt::translate("桌面窗口管理 · TimeTable Next"));
    dlg->setMinimumSize(650,480);
    dlg->setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* layout=new QVBoxLayout(dlg);
    layout->setContentsMargins(22,20,22,20);layout->setSpacing(12);
    auto* title=new QLabel(tt::translate("独立桌面窗口"));
    auto font=title->font();font.setPointSize(17);font.setBold(true);title->setFont(font);
    layout->addWidget(title);
    auto* hint=new QLabel(tt::translate("默认小卡片仅占一行；拖动右下角缩放，右键可折叠或展开。\n左键弹出详情，双击打开课表管理器，位置与两种尺寸分别保存。"));
    hint->setWordWrap(true);layout->addWidget(hint);
    auto* list=new QListWidget;configurationList_=list;
    layout->addWidget(list,1);
    auto* buttons=new QHBoxLayout;
    auto* add=new QPushButton(tt::translate("＋ 新建"));
    auto* importOld=new QPushButton(tt::translate("导入旧版窗口"));
    auto* showAllButton=new QPushButton(tt::translate("全部显示"));
    auto* hideAllButton=new QPushButton(tt::translate("全部隐藏"));
    auto* editButton=new QPushButton(tt::translate("编辑选中"));
    auto* toggle=new QPushButton(tt::translate("启动显示 / 隐藏"));
    auto* removeButton=new QPushButton(tt::translate("删除"));
    auto* collapseButton=new QPushButton(tt::translate("折叠 / 展开"));
    buttons->addWidget(add);buttons->addWidget(editButton);buttons->addWidget(collapseButton);
    buttons->addWidget(toggle);buttons->addWidget(removeButton);
    layout->addLayout(buttons);
    auto* migrationHint=new QLabel(tt::translate("旧版 config.json 可恢复元素布局及窗口参数。全部显示/隐藏只影响当前会话；重启仍按各窗口的“启动时显示”设置。"));
    migrationHint->setWordWrap(true);
    layout->addWidget(migrationHint);
    auto* allActions=new QHBoxLayout;
    allActions->addWidget(importOld);allActions->addWidget(showAllButton);allActions->addWidget(hideAllButton);
    layout->addLayout(allActions);
    auto* pathLabel=new QLabel(configurationPath_+(storageWritable_?QString():
        tt::translate("\n配置文件未成功加载，自动保存已暂停。请恢复有效布局或更改配置路径。")));
    pathLabel->setWordWrap(true);pathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(pathLabel);
    auto* storageActions=new QHBoxLayout;
    auto* exportButton=new QPushButton(tt::translate("备份完整布局"));
    auto* restoreButton=new QPushButton(tt::translate("恢复完整布局"));
    auto* pathButton=new QPushButton(tt::translate("更改配置路径"));
    storageActions->addWidget(exportButton);storageActions->addWidget(restoreButton);storageActions->addWidget(pathButton);
    layout->addLayout(storageActions);
    auto* footer=new QHBoxLayout;
    auto* notes=new QPushButton(tt::translate("每周值日 / 提示"));
    auto* autostart=new QCheckBox(tt::translate("开机自启动"));
#ifdef Q_OS_WIN
    const QString startupPath=QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
    QSettings startup(startupPath,QSettings::NativeFormat);
    autostart->setChecked(startup.contains("TimeTableNext"));
#else
    autostart->setEnabled(false);
    autostart->setToolTip(tt::translate("仅在 Windows 上可用"));
#endif
    auto* full=new QPushButton(tt::translate("进入完整课程管理器"));
    auto* close=new QPushButton(tt::translate("关闭"));
    footer->addWidget(notes);footer->addWidget(autostart);footer->addStretch();
    footer->addWidget(full);footer->addWidget(close);
    layout->addLayout(footer);
    const auto selectedIndex=[list] { return list->currentRow(); };
    connect(add,&QPushButton::clicked,dlg,[this]{edit();});
    connect(importOld,&QPushButton::clicked,dlg,[this]{importLegacyWindows();});
    connect(showAllButton,&QPushButton::clicked,dlg,[this]{showAll();});
    connect(hideAllButton,&QPushButton::clicked,dlg,[this]{hideAll();});
    connect(exportButton,&QPushButton::clicked,dlg,[this,dlg] {
        const QString path=QFileDialog::getSaveFileName(dlg,tt::translate("备份桌面布局（包含背景图片）"),
            QStringLiteral("desktop-backup.json"),tt::translate("JSON 布局 (*.json)"));
        if(path.isEmpty())return;
        QString error;
        if(!exportLayout(path,&error))QMessageBox::warning(dlg,tt::translate("备份失败"),error);
        else QMessageBox::information(dlg,tt::translate("备份完成"),tt::translate("窗口、元素、样式与背景图片已保存，可移动此 JSON 文件后恢复。"));
    });
    connect(restoreButton,&QPushButton::clicked,dlg,[this,dlg] {
        const QString path=QFileDialog::getOpenFileName(dlg,tt::translate("恢复完整桌面布局"),{},tt::translate("JSON 布局 (*.json)"));
        if(path.isEmpty())return;
        QVector<OverlayConfig> verified;QString error;
        if(!DesktopLayoutStore::read(path,&verified,&error)) {
            QMessageBox::warning(dlg,tt::translate("布局无效"),error);return;
        }
        if(QMessageBox::question(dlg,tt::translate("恢复布局"),
            tt::translate("恢复 %1 个窗口并替换当前桌面布局？课程数据不变。").arg(verified.size()))!=QMessageBox::Yes)return;
        if(!restoreLayout(path,&error))QMessageBox::warning(dlg,tt::translate("恢复失败"),error);
    });
    connect(pathButton,&QPushButton::clicked,dlg,[this,dlg,pathLabel] {
        QFileDialog dialog(dlg,tt::translate("选择桌面配置 JSON 路径"),configurationPath_,tt::translate("JSON 布局 (*.json)"));
        dialog.setAcceptMode(QFileDialog::AcceptSave);dialog.setOption(QFileDialog::DontConfirmOverwrite);
        if(dialog.exec()!=QDialog::Accepted || dialog.selectedFiles().isEmpty())return;
        QString error;
        if(!setConfigurationPath(dialog.selectedFiles().first(),&error))QMessageBox::warning(dlg,tt::translate("切换失败"),error);
        else pathLabel->setText(configurationPath_);
    });
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
            setStartupVisible(configs_[index].id,!configs_[index].visible);
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
    if(!configurationDialog_) dlg.setWindowFlag(Qt::WindowStaysOnTopHint,true);
    dlg.setWindowTitle(index<0?tt::translate("创建桌面卡片"):tt::translate("桌面卡片设置"));
    dlg.resize(470,550);
    dlg.setMinimumWidth(450);
    dlg.setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* root=new QVBoxLayout(&dlg);
    auto* tabs=new QTabWidget(&dlg);
    root->addWidget(tabs,1);
    const auto scrollTab=[](QWidget* widget) {
        auto* scroll=new QScrollArea;
        scroll->setFrameShape(QFrame::NoFrame);scroll->setWidgetResizable(true);
        scroll->setWidget(widget);return scroll;
    };

    auto* basics=new QWidget;
    auto* form=new QFormLayout(basics);
    form->setSpacing(12);
    auto* title=new QLineEdit(c.title);
    auto* type=new QComboBox;
    for(int i=0;i<types.size();++i) type->addItem(typeNames()[i],types[i]);
    type->setCurrentIndex(std::max(0,int(types.indexOf(c.type))));
    auto* theme=new QComboBox;
    theme->addItem(tt::translate("柔和浅色"),"light");
    theme->addItem(tt::translate("夜间深色"),"dark");
    theme->setCurrentIndex(c.theme=="dark"?1:0);
    auto* fontSize=new QSpinBox;
    fontSize->setRange(8,35);fontSize->setValue(c.fontSize);
    auto* alpha=new QSpinBox;
    alpha->setRange(0,100);alpha->setSuffix("%");alpha->setValue(c.opacity);
    auto* fps=new QSpinBox;fps->setRange(0,120);fps->setValue(c.fps);
    fps->setSuffix(QStringLiteral(" FPS"));fps->setToolTip(tt::translate("0 停止连续动画帧，课表及其他内容仍会刷新。"));
    auto* inset=new QSpinBox;inset->setRange(0,100);inset->setValue(c.elementInset);
    auto* radius=new QSpinBox;radius->setRange(0,100);radius->setValue(c.cornerRadius);
    auto* debug=new QCheckBox(tt::translate("显示元素调试边框"));debug->setChecked(c.debug);
    auto* showFPS=new QCheckBox(tt::translate("显示实际绘制 FPS"));showFPS->setChecked(c.showFPS);
    auto* acrylic=new QCheckBox(tt::translate("Windows 亚克力模糊背景（可用时启用）"));acrylic->setChecked(c.acrylic);
#ifndef Q_OS_WIN
    acrylic->setEnabled(false);
    acrylic->setToolTip(tt::translate("仅在支持此效果的 Windows 桌面上可用。"));
#endif
    auto* visible=new QCheckBox(tt::translate("启动时显示"));
    visible->setChecked(c.visible);
    auto* top=new QCheckBox(tt::translate("始终置顶"));
    top->setChecked(c.topmost);
    auto* lock=new QCheckBox(tt::translate("锁定位置与大小"));
    lock->setChecked(c.locked);
    auto* details=new QCheckBox(tt::translate("单击弹出详情"));
    details->setChecked(c.showDetailsOnClick);
    form->addRow(tt::translate("窗口标题"),title);
    form->addRow(tt::translate("信息类型"),type);
    form->addRow(tt::translate("窗口主题"),theme);
    form->addRow(tt::translate("字体大小"),fontSize);
    form->addRow(tt::translate("背景不透明度"),alpha);
    form->addRow(tt::translate("动画帧率"),fps);
    form->addRow(tt::translate("元素内边距"),inset);
    form->addRow(tt::translate("窗口圆角"),radius);
    form->addRow(debug);form->addRow(showFPS);form->addRow(acrylic);
    form->addRow(visible);
    form->addRow(top);
    form->addRow(lock);
    form->addRow(details);
    auto* explanation=new QLabel(tt::translate("折叠后的卡片可以小到 125 × 44。"
        "\n桌面直接拖动移动，拖右下角改变尺寸。"));
    explanation->setWordWrap(true);
    form->addRow(explanation);
    tabs->addTab(scrollTab(basics),tt::translate("外观与行为"));

    auto* layoutTab=new QWidget;
    auto* layoutForm=new QFormLayout(layoutTab);
    layoutForm->setSpacing(12);
    auto* collapsed=new QCheckBox(tt::translate("使用紧凑折叠样式"));
    collapsed->setChecked(c.collapsed);
    auto* showTitle=new QCheckBox(tt::translate("紧凑模式显示窗口标题"));
    showTitle->setChecked(c.compactShowTitle);
    auto* compactWidth=new QSpinBox;compactWidth->setRange(1,16384);
    compactWidth->setValue(c.compactGeometry.width());
    auto* compactHeight=new QSpinBox;compactHeight->setRange(1,16384);
    compactHeight->setValue(c.compactGeometry.height());
    auto* expandedWidth=new QSpinBox;expandedWidth->setRange(1,16384);
    expandedWidth->setValue(c.expandedGeometry.width());
    auto* expandedHeight=new QSpinBox;expandedHeight->setRange(1,16384);
    expandedHeight->setValue(c.expandedGeometry.height());
    layoutForm->addRow(collapsed);
    layoutForm->addRow(showTitle);
    layoutForm->addRow(tt::translate("折叠宽度"),compactWidth);
    layoutForm->addRow(tt::translate("折叠高度"),compactHeight);
    layoutForm->addRow(tt::translate("展开宽度"),expandedWidth);
    layoutForm->addRow(tt::translate("展开高度"),expandedHeight);
    auto* hint=new QLabel(tt::translate("折叠只显示关键文字，展开时显示标题、时间、地点及提示。"
        "\n两种尺寸分别记忆，右键即可随时切换。"));
    hint->setWordWrap(true);
    layoutForm->addRow(hint);
    tabs->addTab(scrollTab(layoutTab),tt::translate("折叠与尺寸"));

    auto* dataTab=new QWidget;
    auto* dataForm=new QFormLayout(dataTab);
    auto* note=new QLineEdit(c.note);
    note->setPlaceholderText(tt::translate("便签文本 / 组合卡片自定义备注"));
    auto* items=new QLineEdit(c.combinedItems);
    items->setPlaceholderText("clock,date,current,next,week,duty,countdown,note");
    auto* cycle=new QCheckBox(tt::translate("逐项轮播（兼容旧版按顺序显示多个信息）"));
    cycle->setChecked(c.cycleItems);
    auto* seconds=new QSpinBox;
    seconds->setRange(2,300);seconds->setSuffix(tt::translate(" 秒"));seconds->setValue(c.cycleSeconds);
    seconds->setEnabled(c.cycleItems);
    connect(cycle,&QCheckBox::toggled,seconds,&QWidget::setEnabled);
    auto* tokens=new QLabel(tt::translate("组合信息可按顺序输入，用英文逗号隔开：\n"
        "clock 时间 · date 日期 · current 当前课程 · next 下一节 · week 教学周\n"
        "duty 每日值日 · countdown 倒计日 · note 备注"));
    tokens->setWordWrap(true);
    auto* target=new QDateTimeEdit(c.target.isValid()?c.target:QDateTime::currentDateTime().addDays(30));
    target->setCalendarPopup(true);target->setDisplayFormat("yyyy-MM-dd HH:mm");
    auto* editDuty=new QPushButton(tt::translate("编辑每周值日信息"));
    auto* elementButton=new QPushButton(tt::translate("打开元素拖拽布局编辑器"));
    auto* backgroundPath=new QLineEdit(c.backgroundImage);
    backgroundPath->setPlaceholderText(tt::translate("本地图片文件路径（可留空）"));
    auto* browseBackground=new QPushButton(tt::translate("选择背景图片"));
    auto* useImage=new QCheckBox(tt::translate("显示背景图片"));useImage->setChecked(c.imageBackground);
    auto* backgroundColor=new QPushButton(tt::translate("选择背景颜色"));
    QColor selectedBackground=c.backgroundColor;
    backgroundColor->setStyleSheet("background-color:"+selectedBackground.name()+";");
    auto* autoSize=new QCheckBox(tt::translate("根据元素占用空间自动调整展开尺寸"));
    autoSize->setChecked(c.autoSize);
    dataForm->addRow(tt::translate("备注文字"),note);
    dataForm->addRow(tt::translate("组合信息顺序"),items);
    dataForm->addRow(cycle);
    dataForm->addRow(tt::translate("轮播间隔"),seconds);
    dataForm->addRow(tokens);
    dataForm->addRow(tt::translate("倒计日目标"),target);
    dataForm->addRow(editDuty);
    dataForm->addRow(elementButton);
    dataForm->addRow(tt::translate("背景图片路径"),backgroundPath);
    dataForm->addRow(browseBackground);
    dataForm->addRow(useImage);
    dataForm->addRow(backgroundColor);
    dataForm->addRow(autoSize);
    connect(browseBackground,&QPushButton::clicked,&dlg,[&]{
        const QString image=QFileDialog::getOpenFileName(&dlg,tt::translate("选择桌面卡片背景"),
            backgroundPath->text(),tt::translate("图片 (*.png *.jpg *.jpeg *.bmp *.webp)"));
        if(!image.isEmpty()){backgroundPath->setText(image);useImage->setChecked(true);}
    });
    connect(backgroundColor,&QPushButton::clicked,&dlg,[&]{
        const QColor value=QColorDialog::getColor(selectedBackground,&dlg,
            tt::translate("选择卡片背景色"),QColorDialog::ShowAlphaChannel);
        if(value.isValid()){selectedBackground=value;backgroundColor->setStyleSheet("background-color:"+value.name()+";");}
    });
    QList<ElementDefinition> editElements=c.elements;
    connect(elementButton,&QPushButton::clicked,&dlg,[&]{
        if(ElementEditor::edit(&dlg,editElements,planner_->schedule(),
                              QSize(expandedWidth->value(),expandedHeight->value()),theme->currentData().toString()=="dark")) {
            type->setCurrentIndex(type->findData("elements"));
        }
    });
    connect(editDuty,&QPushButton::clicked,&dlg,[this]{editDutyNotes();});
    tabs->addTab(scrollTab(dataTab),tt::translate("显示内容"));

    // Update generated titles as the template changes; never overwrite a title typed by the user.
    auto automaticTitle=std::make_shared<bool>(c.title==typeName(c.type));
    connect(title,&QLineEdit::textEdited,&dlg,[automaticTitle](const QString&){*automaticTitle=false;});
    connect(type,QOverload<int>::of(&QComboBox::currentIndexChanged),&dlg,
        [title,automaticTitle](int selected) {
            if(*automaticTitle) title->setText(typeName(types.at(selected)));
        });
    const auto syncFields=[type,note,items,cycle,seconds,target]() {
        const QString selected=type->currentData().toString();
        const bool combined=selected=="combined";
        note->setEnabled(combined || selected=="note");
        items->setEnabled(combined);
        cycle->setEnabled(combined);
        seconds->setEnabled(combined && cycle->isChecked());
        target->setEnabled(combined || selected=="countdown");
    };
    connect(type,QOverload<int>::of(&QComboBox::currentIndexChanged),&dlg,
        [syncFields](int){syncFields();});
    connect(cycle,&QCheckBox::toggled,&dlg,[syncFields](bool){syncFields();});
    syncFields();

    auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    actions->button(QDialogButtonBox::Save)->setText(tt::translate("应用并保存"));
    actions->button(QDialogButtonBox::Cancel)->setText(tt::translate("取消"));
    root->addWidget(actions);
    connect(actions,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    connect(actions,&QDialogButtonBox::accepted,&dlg,[&,this] {
        const QStringList allowed={"clock","date","current","next","week","duty","countdown","note"};
        for(const auto& raw:items->text().split(',',Qt::SkipEmptyParts)) {
            if(!allowed.contains(raw.trimmed().toLower())) {
                QMessageBox::warning(&dlg,tt::translate("不支持的组合字段"),
                    tt::translate("无法识别「%1」，请使用“显示内容”中列出的字段。").arg(raw.trimmed()));
                tabs->setCurrentIndex(2);
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
        c.fps=fps->value();c.debug=debug->isChecked();c.showFPS=showFPS->isChecked();
        c.elementInset=inset->value();c.cornerRadius=radius->value();c.acrylic=acrylic->isChecked();
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
        c.elements=editElements;
        const QString selectedPath=backgroundPath->text().trimmed();
        if(selectedPath!=c.backgroundImage)c.backgroundAsset.clear();
        c.backgroundImage=DesktopLayoutStore::resolveBackground(selectedPath,configurationPath_);
        c.imageBackground=useImage->isChecked();
        c.backgroundColor=selectedBackground;
        c.autoSize=autoSize->isChecked();
        c.compactGeometry=QRect(origin,QSize(compactWidth->value(),compactHeight->value()));
        c.expandedGeometry=QRect(origin,QSize(expandedWidth->value(),expandedHeight->value()));
        c.collapsed=collapsed->isChecked();
        if(c.autoSize && c.type=="elements" && !c.elements.isEmpty()) {
            const QRect bounds=ElementLayout::contentBounds(c.elements,planner_->schedule(),QDateTime::currentDateTime());
            c.expandedGeometry.setSize(QSize(std::clamp(bounds.right()+1+2*c.elementInset,1,16384),
                                              std::clamp(bounds.bottom()+1+2*c.elementInset,1,16384)));
        }
        c.geometry=c.collapsed?c.compactGeometry:c.expandedGeometry;
        if(index>=0) configs_[index]=c;
        else configs_.append(c);
        save();rebuild();dlg.accept();
    });
    dlg.exec();
}

void DesktopOverlays::editElements(const QString& id) {
    const int index=indexOf(id);
    if(index<0) return;
    OverlayConfig c=configs_[index];
    QList<ElementDefinition> layout=c.elements;
    if(layout.isEmpty()) {
        ElementDefinition entry;
        TextElement item;
        item.type=c.type=="clock"?"clock":c.type=="current"?"current":"note";
        item.format=item.type=="clock"?"%H:%M:%S":c.note;
        entry.items<<item;
        layout<<entry;
    }
    if(!ElementEditor::edit(configurationDialog_.data(),layout,planner_->schedule(),
                             c.expandedGeometry.size(),c.theme=="dark")) return;
    c.elements=layout;c.type="elements";c.collapsed=false;
    if(c.autoSize && !layout.isEmpty()) {
        const QRect bounds=ElementLayout::contentBounds(layout,planner_->schedule(),QDateTime::currentDateTime());
        c.expandedGeometry.setSize(QSize(std::clamp(bounds.right()+1+2*c.elementInset,1,16384),
                                          std::clamp(bounds.bottom()+1+2*c.elementInset,1,16384)));
    }
    c.geometry=c.expandedGeometry;
    configs_[index]=c;
    save();rebuild();
}
void DesktopOverlays::showAll() {
    temporarilyHidden_=false;temporarilyShowAll_=true;temporarilyHiddenWindows_.clear();syncVisibility();
}
void DesktopOverlays::hideAll() {
    temporarilyHidden_=true;temporarilyShowAll_=false;syncVisibility();
}
void DesktopOverlays::importLegacyWindows() {
    const QString path=QFileDialog::getOpenFileName(configurationDialog_.data(),
        tt::translate("导入旧版 TimeTable-Qt 窗口布局"),
        QString(),tt::translate("旧版配置文件 (*.json)"));
    if(path.isEmpty()) return;
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(configurationDialog_.data(),tt::translate("打开失败"),file.errorString());
        return;
    }
    QByteArray bytes=file.readAll();
    if(bytes.startsWith(QByteArray::fromHex("EFBBBF"))) bytes.remove(0,3);
    QJsonParseError error;
    const auto parsed=QJsonDocument::fromJson(bytes,&error);
    if(!parsed.isObject()) {
        QMessageBox::warning(configurationDialog_.data(),tt::translate("JSON 格式错误"),error.errorString());
        return;
    }
    const auto root=parsed.object();
    QVector<OverlayConfig> imported;
    int skipped=0;
    QString migrationError;
    if(!DesktopLayoutStore::fromLegacy(root,path,&imported,&skipped,&migrationError)) {
        QMessageBox::warning(configurationDialog_.data(),tt::translate("配置无法识别"),migrationError);
        return;
    }
    if(imported.isEmpty()) {
        QMessageBox::information(configurationDialog_.data(),tt::translate("无可迁移窗口"),
            tt::translate("旧配置中的窗口没有可识别的信息元素，原有设置没有变化。"));
        return;
    }
    const QString message=tt::translate("已恢复 %1 个独立窗口的元素坐标、字体、颜色与文本格式。\n"
        "%2 个无效或尚不支持的元素被跳过。导入只追加窗口，不会覆盖课表。继续吗？")
        .arg(imported.size()).arg(skipped);
    if(QMessageBox::question(configurationDialog_.data(),tt::translate("确认导入窗口"),message)
       !=QMessageBox::Yes) return;
    for(const auto& config:imported) configs_.append(config);
    save();rebuild();
}
void DesktopOverlays::editDutyNotes() {
    QDialog dlg(configurationDialog_?configurationDialog_.data():nullptr);
    if(!configurationDialog_) dlg.setWindowFlag(Qt::WindowStaysOnTopHint,true);
    dlg.setWindowTitle(tt::translate("每周值日 / 每日提醒"));
    dlg.setMinimumWidth(430);
    dlg.setStyleSheet(paletteCss(QSettings().value("dark",false).toBool()));
    auto* content=new QVBoxLayout(&dlg);
    auto* intro=new QLabel(tt::translate("为每个星期设置一条值日、值班或日常提醒；留空则不显示。"
        "\n可以使用“每日信息”卡片，也可插入到“组合信息”卡片。"));
    intro->setWordWrap(true);
    content->addWidget(intro);
    auto* form=new QFormLayout;
    const QStringList days={tt::translate("周一"),tt::translate("周二"),tt::translate("周三"),
        tt::translate("周四"),tt::translate("周五"),tt::translate("周六"),tt::translate("周日")};
    QVector<QLineEdit*> fields;
    QSettings settings;
    for(int day=1;day<=7;++day) {
        auto* field=new QLineEdit(planner_->schedule().dayInfos.value(day).join(QStringLiteral(" ; ")));
        if(field->text().isEmpty()) field->setText(settings.value("desktop/duty/"+QString::number(day)).toString());
        field->setPlaceholderText(tt::translate("例如：值日 A 组 / 晚自习"));
        form->addRow(days[day-1],field);fields<<field;
    }
    content->addLayout(form);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    content->addWidget(buttons);
    connect(buttons,&QDialogButtonBox::rejected,&dlg,&QDialog::reject);
    connect(buttons,&QDialogButtonBox::accepted,&dlg,[this,&dlg,fields] {
        QSettings settings;
        for(int day=1;day<=7;++day) {
            const QString raw=fields[day-1]->text().trimmed();
            const auto messages=raw.split(';',Qt::SkipEmptyParts);
            QStringList clean;
            for(const auto& message:messages) if(!message.trimmed().isEmpty()) clean<<message.trimmed();
            planner_->setDayInfos(day,clean);
            settings.setValue("desktop/duty/"+QString::number(day),raw);
        }
        refresh();
        dlg.accept();
    });
    dlg.exec();
}
void DesktopOverlays::remove(const QString& id) {
    const int index=indexOf(id);
    if(index<0) return;
    if(QMessageBox::question(configurationDialog_?configurationDialog_.data():nullptr,
        tt::translate("删除桌面信息窗口"),
        tt::translate("删除「%1」？此操作不会删除课程数据。").arg(nameOrFallback(configs_[index])))!=QMessageBox::Yes)
        return;
    configs_.removeAt(index);
    save();rebuild();
}
} // namespace tt
