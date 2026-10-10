#include "MainWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFile>
#include <QUuid>
#include <QColor>
#include <QVariant>
#include <QPalette>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTableWidget>
#include <QTimeEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QSystemTrayIcon>
#include <QEasingCurve>
#include <algorithm>

namespace tt {
namespace {
const QStringList kDays={QStringLiteral("周一"),QStringLiteral("周二"),QStringLiteral("周三"),
    QStringLiteral("周四"),QStringLiteral("周五"),QStringLiteral("周六"),QStringLiteral("周日")};

QLabel* label(const QString& text, const QString& name={}, int pointSize=0) {
    auto* w=new QLabel(text);
    w->setTextFormat(Qt::PlainText);
    if(!name.isEmpty()) w->setObjectName(name);
    if(pointSize) { auto f=w->font(); f.setPointSize(pointSize); w->setFont(f); }
    w->setWordWrap(true);
    return w;
}
QPushButton* button(const QString& text, bool primary=false) {
    auto* b=new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    if(primary) b->setObjectName("primaryButton");
    b->setMinimumHeight(38);
    return b;
}
QFrame* card(const QString& objectName="card") {
    auto* w=new QFrame;
    w->setObjectName(objectName);
    w->setFrameShape(QFrame::NoFrame);
    return w;
}
void clear(QLayout* layout) {
    if(!layout) return;
    while(auto* item=layout->takeAt(0)) {
        if(item->widget()) delete item->widget();
        if(item->layout()) { clear(item->layout()); delete item->layout(); }
        delete item;
    }
}
QString timeRange(const Course& c) {
    return c.start.toString("HH:mm")+" – "+c.end.toString("HH:mm");
}
QString remain(int seconds) {
    if(seconds<0) seconds=0;
    const int days=seconds/86400, hours=seconds%86400/3600, minutes=seconds%3600/60;
    if(days) return QStringLiteral("%1 天 %2 小时").arg(days).arg(hours);
    if(hours) return QStringLiteral("%1 小时 %2 分").arg(hours).arg(minutes);
    return QStringLiteral("%1 分 %2 秒").arg(minutes).arg(seconds%60);
}
// Lightweight Fluent-style settings layout based on stock Qt Widgets.
// This stays usable on Windows 7/Qt5: no Windows App SDK dependency.
QWidget* fluentSettingRow(const QString& heading, const QString& description, QWidget* control) {
    auto* row=new QWidget;
    row->setObjectName("settingRow");
    auto* layout=new QHBoxLayout(row);
    layout->setContentsMargins(18,13,18,13);
    layout->setSpacing(18);
    auto* text=new QVBoxLayout;
    text->setSpacing(5);
    auto* h=label(heading,"settingLabel",11);
    auto* d=label(description,"settingDescription",9);
    text->addWidget(h);
    text->addWidget(d);
    layout->addLayout(text,1);
    control->setMinimumWidth(130);
    layout->addWidget(control,0,Qt::AlignRight|Qt::AlignVCenter);
    return row;
}
QFrame* fluentSection() {
    auto* frame=new QFrame;
    frame->setObjectName("fluentSection");
    frame->setFrameShape(QFrame::NoFrame);
    return frame;
}
QLabel* sectionTitle(const QString& text) {
    auto* heading=label(text,"settingsGroupTitle",13);
    return heading;
}
QWidget* scrollWidget(QWidget* content) {
    auto* view=new QScrollArea;
    view->setWidgetResizable(true);
    view->setFrameShape(QFrame::NoFrame);
    view->setWidget(content);
    return view;
}
QString description(const Course& c) {
    QStringList parts;
    if(!c.room.trimmed().isEmpty()) parts << "📍 "+c.room;
    if(!c.teacher.trimmed().isEmpty()) parts << c.teacher;
    return parts.join("  ·  ");
}
} // namespace

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent) {
    setWindowTitle(QStringLiteral("TimeTable Next · 课表与日程"));
    resize(1080,750);
    setMinimumSize(835,580);
    dataPath_=ScheduleStore::defaultPath();
    if(QFile::exists(dataPath_)) {
        QString error;
        if(!store_.load(dataPath_,&error))
            QMessageBox::warning(this,QStringLiteral("读取课表失败"),
                QStringLiteral("原始文件保持不变，请修复后重新启动。\n%1\n%2").arg(dataPath_,error));
    } else {
        // Clearly labelled, removable examples; no private personal information.
        for(int d: {1,3,5}) {
            Course example;
            example.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
            example.name=d==1 ? QStringLiteral("示例 · 高等数学") :
                (d==3 ? QStringLiteral("示例 · 程序设计") : QStringLiteral("示例 · 英语"));
            example.room=d==3 ? QStringLiteral("实验楼 302") : QStringLiteral("教学楼 A201");
            example.weekday=d;
            example.start=QTime(8,0);example.end=QTime(9,40);
            for(int w=1;w<=16;++w) example.weeks << w;
            example.color=d==1?"#6C6AF6":d==3?"#1DAF9C":"#F18E64";
            store_.courses << example;
        }
        store_.save(dataPath_);
    }
    shownWeek_=std::max(1,store_.weekNumber(QDate::currentDate()));
    dark_=QSettings().value("dark",false).toBool();
    buildUi();
    buildTray();
    applyTheme();
    fillSettings();
    saveAndRefresh();
    auto* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this] { updateDashboard(); });
    timer->start(1000);
}

void MainWindow::buildUi() {
    auto* root=new QWidget;
    root->setObjectName("root");
    auto* horizontal=new QHBoxLayout(root);
    horizontal->setContentsMargins(0,0,0,0);
    horizontal->setSpacing(0);

    auto* sidebar=card("sidebar");
    sidebar->setFixedWidth(222);
    auto* side=new QVBoxLayout(sidebar);
    side->setContentsMargins(16,30,16,18);
    side->setSpacing(6);
    side->addWidget(label(QStringLiteral("TimeTable"),"brand",19));
    side->addWidget(label(QStringLiteral("桌面课程与日程"),"sidebarSubtitle",9));
    side->addSpacing(30);
    side->addWidget(label(QStringLiteral("工作区"),"sidebarGroup",9));
    const QStringList tabs={QStringLiteral("  今日"),QStringLiteral("  周课表"),
        QStringLiteral("  课程"),QStringLiteral("  特殊日期"),
        QStringLiteral("  设置")};
    for(int i=0;i<tabs.size();++i) {
        auto* nav=button(tabs.at(i));
        nav->setProperty("navigation",true);
        nav->setCheckable(true);
        nav->setMinimumHeight(43);
        side->addWidget(nav);
        navigation_ << nav;
        connect(nav,&QPushButton::clicked,this,[this,i]{switchPage(i);});
    }
    side->addStretch();
    side->addWidget(label(QStringLiteral("桌面窗口"),"sidebarGroup",9));
    auto* overlaysBtn=button(QStringLiteral("管理浮动组件"));
    side->addWidget(overlaysBtn);
    connect(overlaysBtn,&QPushButton::clicked,this,[this] { if(onOpenOverlaySettings_) onOpenOverlaySettings_(); });
    auto* importBtn=button(QStringLiteral("导入数据"));
    auto* exportBtn=button(QStringLiteral("导出与备份"));
    side->addWidget(importBtn);
    side->addWidget(exportBtn);
    connect(importBtn,&QPushButton::clicked,this,[this]{importSchedule();});
    connect(exportBtn,&QPushButton::clicked,this,[this]{exportSchedule();});
    horizontal->addWidget(sidebar);

    auto* right=new QWidget;
    auto* rightLayout=new QVBoxLayout(right);
    rightLayout->setContentsMargins(30,22,30,18);
    rightLayout->setSpacing(16);
    auto* header=new QHBoxLayout;
    auto* titles=new QVBoxLayout;
    titles->addWidget(label(QStringLiteral("今日"),"pageTitle",22));
    titles->addWidget(label(QStringLiteral("当前课程与近期安排"),"pageSubtitle",10));
    header->addLayout(titles,1);
    clockLabel_=label("", "clock",13);
    header->addWidget(clockLabel_,0,Qt::AlignRight|Qt::AlignVCenter);
    rightLayout->addLayout(header);

    stack_=new QStackedWidget;
    stack_->addWidget(createDashboard());
    stack_->addWidget(createWeekPage());
    stack_->addWidget(createCoursesPage());
    stack_->addWidget(createChangesPage());
    stack_->addWidget(createSettingsPage());
    rightLayout->addWidget(stack_,1);
    horizontal->addWidget(right,1);
    setCentralWidget(root);
    statusBar()->setSizeGripEnabled(false);
    switchPage(0);
}

QWidget* MainWindow::createDashboard() {
    auto* page=new QWidget;
    auto* col=new QVBoxLayout(page);
    col->setContentsMargins(0,0,0,0);
    col->setSpacing(18);

    auto* hero=card("hero");
    hero->setMinimumHeight(238);
    auto* inside=new QVBoxLayout(hero);
    inside->setContentsMargins(32,26,32,26);
    inside->setSpacing(10);
    heroEyebrow_=label(QStringLiteral("今天的安排"),"heroSmall",13);
    heroCourse_=label(QStringLiteral("准备就绪"),"heroTitle",27);
    heroDetail_=label(QString(),"heroSub",13);
    heroCountdown_=label(QString(),"heroCountdown",17);
    auto* progress=new QProgressBar;
    progress->setObjectName("lessonProgress");
    progress->setTextVisible(false);
    progress->setMaximumHeight(8);
    progress->setValue(0);
    inside->addWidget(heroEyebrow_);
    inside->addWidget(heroCourse_);
    inside->addWidget(heroDetail_);
    inside->addStretch();
    inside->addWidget(heroCountdown_);
    inside->addWidget(progress);
    col->addWidget(hero,0);

    auto* lower=new QHBoxLayout; lower->setSpacing(18);
    auto* today=card();
    auto* dayLayout=new QVBoxLayout(today);
    dayLayout->setContentsMargins(20,18,20,18);
    dayLayout->addWidget(label(QStringLiteral("今日课程"),"sectionHeading",17));
    auto* itemsHost=new QWidget;
    todayList_=new QVBoxLayout(itemsHost);
    todayList_->setContentsMargins(0,0,0,0);
    todayList_->setSpacing(9);
    dayLayout->addWidget(scrollWidget(itemsHost),1);
    lower->addWidget(today,5);

    auto* upcoming=card();
    auto* nextLayout=new QVBoxLayout(upcoming);
    nextLayout->setContentsMargins(23,20,23,20);
    nextLayout->setSpacing(15);
    nextLayout->addWidget(label(QStringLiteral("接下来"),"sectionHeading",17));
    nextLabel_=label("", "upcoming",15);
    nextLayout->addWidget(nextLabel_,1,Qt::AlignTop);
    auto* weekBtn=button(QStringLiteral("查看完整周课表  →"),true);
    auto* changeBtn=button(QStringLiteral("安排临时调休  →"));
    nextLayout->addWidget(weekBtn);
    nextLayout->addWidget(changeBtn);
    connect(weekBtn,&QPushButton::clicked,this,[this]{switchPage(1);});
    connect(changeBtn,&QPushButton::clicked,this,[this]{switchPage(3);});
    lower->addWidget(upcoming,3);
    col->addLayout(lower,1);
    return page;
}

QWidget* MainWindow::createWeekPage() {
    auto* page=new QWidget;
    auto* v=new QVBoxLayout(page);v->setContentsMargins(0,0,0,0);v->setSpacing(15);
    auto* bar=new QHBoxLayout;
    auto* title=label(QStringLiteral("一周的全貌"),"sectionHeading",19);
    bar->addWidget(title);bar->addStretch();
    auto* prev=button(QStringLiteral("← 上一周"));
    auto* next=button(QStringLiteral("下一周 →"));
    auto* current=button(QStringLiteral("回到本周"),true);
    weekLabel_=label("", "weekCounter",13);
    bar->addWidget(prev);bar->addWidget(weekLabel_);bar->addWidget(next);bar->addWidget(current);
    connect(prev,&QPushButton::clicked,this,[this]{shownWeek_=std::max(1,shownWeek_-1);updateWeek();});
    connect(next,&QPushButton::clicked,this,[this]{shownWeek_=std::min(store_.totalWeeks,shownWeek_+1);updateWeek();});
    connect(current,&QPushButton::clicked,this,[this]{
        shownWeek_=std::max(1,store_.weekNumber(QDate::currentDate()));updateWeek();
    });
    v->addLayout(bar);
    weekContent_=new QWidget;
    auto* horizontal=new QHBoxLayout(weekContent_);
    horizontal->setSpacing(10);horizontal->setContentsMargins(4,6,4,8);
    v->addWidget(scrollWidget(weekContent_),1);
    v->addWidget(label(QStringLiteral("提示：课程卡片按实际日期显示；调休和教学周规则会自动生效。"),"muted"));
    return page;
}

QWidget* MainWindow::createCoursesPage() {
    auto* page=new QWidget;
    auto* v=new QVBoxLayout(page);v->setContentsMargins(0,0,0,0);v->setSpacing(12);
    auto* bar=new QHBoxLayout;
    bar->addWidget(label(QStringLiteral("课程管理"),"sectionHeading",19));
    bar->addStretch();
    auto* add=button(QStringLiteral("＋ 新建课程"),true);
    auto* edit=button(QStringLiteral("编辑"));
    auto* copy=button(QStringLiteral("复制"));
    auto* remove=button(QStringLiteral("删除"));
    bar->addWidget(add);bar->addWidget(edit);bar->addWidget(copy);bar->addWidget(remove);
    v->addLayout(bar);
    courseTable_=new QTableWidget;
    courseTable_->setColumnCount(7);
    courseTable_->setHorizontalHeaderLabels({QStringLiteral("课程名称"),QStringLiteral("星期"),
        QStringLiteral("上课时间"),QStringLiteral("教学周"),QStringLiteral("地点"),
        QStringLiteral("教师"),QStringLiteral("配色")});
    courseTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    courseTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    courseTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    courseTable_->setAlternatingRowColors(false);
    courseTable_->verticalHeader()->hide();
    courseTable_->verticalHeader()->setDefaultSectionSize(48);
    courseTable_->horizontalHeader()->setStretchLastSection(true);
    courseTable_->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    courseTable_->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Stretch);
    v->addWidget(courseTable_,1);
    v->addWidget(label(QStringLiteral("教学周示例：全部 / 1-16 / 1-16单 / 2-18双 / 1-8,10,12"),"muted"));
    connect(add,&QPushButton::clicked,this,[this]{editCourse();});
    connect(edit,&QPushButton::clicked,this,[this]{editCourse(courseTable_->currentRow());});
    connect(copy,&QPushButton::clicked,this,[this]{duplicateCourse();});
    connect(remove,&QPushButton::clicked,this,[this]{removeCourse();});
    connect(courseTable_,&QTableWidget::cellDoubleClicked,this,[this](int row,int){editCourse(row);});
    return page;
}

QWidget* MainWindow::createChangesPage() {
    auto* page=new QWidget;
    auto* v=new QVBoxLayout(page);v->setContentsMargins(0,0,0,0);v->setSpacing(15);
    v->addWidget(label(QStringLiteral("调休与特殊日期"),"sectionHeading",19));
    v->addWidget(label(QStringLiteral("快速将某一天设为休息日，或临时按照另一日期的课表上课。\n适合节假日补课、周末调课、考试周。"),"muted"));
    auto* settings=card();
    auto* form=new QFormLayout(settings);
    form->setContentsMargins(24,24,24,24);
    form->setSpacing(16);
    changeDate_=new QDateEdit(QDate::currentDate());
    changeDate_->setCalendarPopup(true);
    changeDate_->setDisplayFormat("yyyy-MM-dd dddd");
    sourceDate_=new QDateEdit(QDate::currentDate());
    sourceDate_->setCalendarPopup(true);
    sourceDate_->setDisplayFormat("yyyy-MM-dd dddd");
    holiday_=new QCheckBox(QStringLiteral("休息日（当天不上课）"));
    holiday_->setChecked(true);
    sourceDate_->setEnabled(false);
    connect(holiday_,&QCheckBox::toggled,sourceDate_,[this](bool checked){sourceDate_->setEnabled(!checked);});
    form->addRow(QStringLiteral("要调整的日期"),changeDate_);
    form->addRow(QStringLiteral("安排类型"),holiday_);
    form->addRow(QStringLiteral("改按哪天上课"),sourceDate_);
    auto* add=button(QStringLiteral("保存日期调整"),true);
    auto* quick=button(QStringLiteral("一键将今天设为休息日"));
    auto* row=new QHBoxLayout;
    row->addWidget(add);row->addWidget(quick);
    form->addRow(row);
    v->addWidget(settings);
    v->addWidget(label(QStringLiteral("已设置的特殊日期"),"sectionHeading",16));
    exceptionList_=new QListWidget;
    exceptionList_->setAlternatingRowColors(false);
    v->addWidget(exceptionList_,1);
    auto* remove=button(QStringLiteral("删除选中的日期调整"));
    v->addWidget(remove);
    connect(add,&QPushButton::clicked,this,[this]{addDayChange();});
    connect(quick,&QPushButton::clicked,this,[this] {
        changeDate_->setDate(QDate::currentDate());holiday_->setChecked(true);addDayChange();
    });
    connect(remove,&QPushButton::clicked,this,[this]{removeDayChange();});
    return page;
}

QWidget* MainWindow::createSettingsPage() {
    auto* page=new QWidget;
    auto* outer=new QVBoxLayout(page);
    outer->setContentsMargins(0,0,0,0);
    outer->setSpacing(12);

    auto* scroller=new QScrollArea;
    scroller->setObjectName("settingsScroll");
    scroller->setWidgetResizable(true);
    scroller->setFrameShape(QFrame::NoFrame);
    auto* host=new QWidget;
    host->setObjectName("settingsBody");
    auto* column=new QVBoxLayout(host);
    column->setContentsMargins(1,5,12,14);
    column->setSpacing(13);

    auto* intro=label(QStringLiteral("管理课表周期、显示偏好和每日作息。"),"settingDescription",10);
    column->addWidget(intro);
    column->addWidget(sectionTitle(QStringLiteral("学期")));

    auto* semester=fluentSection();
    auto* semesterRows=new QVBoxLayout(semester);
    semesterRows->setContentsMargins(0,0,0,0);
    semesterRows->setSpacing(0);
    semesterDate_=new QDateEdit;
    semesterDate_->setCalendarPopup(true);
    semesterDate_->setDisplayFormat("yyyy-MM-dd");
    semesterDate_->setMinimumWidth(170);
    semesterRows->addWidget(fluentSettingRow(QStringLiteral("学期开始日期"),
        QStringLiteral("以该日期所在的星期一作为第一周"),semesterDate_));
    auto* line1=new QFrame;line1->setObjectName("settingDivider");
    line1->setFixedHeight(1);
    semesterRows->addWidget(line1);
    weeksSpin_=new QSpinBox;
    weeksSpin_->setRange(1,60);
    weeksSpin_->setSuffix(QStringLiteral(" 周"));
    semesterRows->addWidget(fluentSettingRow(QStringLiteral("教学周数"),
        QStringLiteral("课程的单双周与指定周以此为范围"),weeksSpin_));
    column->addWidget(semester);

    column->addWidget(sectionTitle(QStringLiteral("个性化")));
    auto* appearance=fluentSection();
    auto* appearanceRows=new QVBoxLayout(appearance);
    appearanceRows->setContentsMargins(0,0,0,0);
    appearanceRows->setSpacing(0);
    darkMode_=new QCheckBox;
    darkMode_->setText(QStringLiteral("使用深色主题"));
    appearanceRows->addWidget(fluentSettingRow(QStringLiteral("应用主题"),
        QStringLiteral("明确指定浅色或深色，不受 Windows 主题影响"),darkMode_));
    auto* line2=new QFrame;line2->setObjectName("settingDivider");line2->setFixedHeight(1);
    appearanceRows->addWidget(line2);
    alwaysOnTop_=new QCheckBox;
    alwaysOnTop_->setText(QStringLiteral("管理器置顶"));
    appearanceRows->addWidget(fluentSettingRow(QStringLiteral("窗口层级"),
        QStringLiteral("常驻信息卡片的置顶策略请在浮动组件中设置"),alwaysOnTop_));
    column->addWidget(appearance);

    column->addWidget(sectionTitle(QStringLiteral("节次时间")));
    auto* periods=fluentSection();
    auto* periodsLayout=new QVBoxLayout(periods);
    periodsLayout->setContentsMargins(18,16,18,18);
    periodsLayout->setSpacing(10);
    periodsLayout->addWidget(label(QStringLiteral("设置每一节课的起止时间，用于将第三方课程表节次转换为实际时刻。"),
        "settingDescription",9));
    periodsTable_=new QTableWidget;
    periodsTable_->setObjectName("periodsTable");
    periodsTable_->setColumnCount(3);
    periodsTable_->setHorizontalHeaderLabels({QStringLiteral("节次"),QStringLiteral("开始"),QStringLiteral("结束")});
    periodsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    periodsTable_->verticalHeader()->hide();
    periodsTable_->setAlternatingRowColors(false);
    periodsTable_->setMinimumHeight(305);
    periodsTable_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    periodsLayout->addWidget(periodsTable_);
    column->addWidget(periods);
    column->addStretch();

    scroller->setWidget(host);
    outer->addWidget(scroller,1);
    auto* actions=new QHBoxLayout;
    actions->addWidget(label(QStringLiteral("修改后需要点击保存才会写入本地配置"),"settingDescription",9),1);
    auto* save=button(QStringLiteral("保存更改"),true);
    save->setMinimumWidth(125);
    actions->addWidget(save);
    outer->addLayout(actions);
    connect(save,&QPushButton::clicked,this,[this]{saveSettings();});
    return page;
}

void MainWindow::buildTray() {
    if(!QSystemTrayIcon::isSystemTrayAvailable()) return;
    tray_=new QSystemTrayIcon(style()->standardIcon(QStyle::SP_DesktopIcon),this);
    auto* menu=new QMenu(this);
    auto* manageWindows=menu->addAction(QStringLiteral("管理桌面信息窗口"));
    auto* show=menu->addAction(QStringLiteral("打开课程管理器"));
    auto* holiday=menu->addAction(QStringLiteral("今天临时休息"));
    menu->addSeparator();
    auto* quit=menu->addAction(QStringLiteral("退出程序"));
    connect(manageWindows,&QAction::triggered,this,[this]{
        if(onOpenOverlaySettings_) onOpenOverlaySettings_();
    });
    connect(show,&QAction::triggered,this,[this]{showNormal();raise();activateWindow();});
    connect(holiday,&QAction::triggered,this,[this]{
        changeDate_->setDate(QDate::currentDate());holiday_->setChecked(true);addDayChange();
    });
    connect(quit,&QAction::triggered,this,[this]{reallyQuit_=true;tray_->hide();QApplication::quit();});
    connect(tray_,&QSystemTrayIcon::activated,this,[this](QSystemTrayIcon::ActivationReason reason){
        if(reason==QSystemTrayIcon::Trigger || reason==QSystemTrayIcon::DoubleClick) {
            showNormal();raise();activateWindow();
        }
    });
    tray_->setToolTip(QStringLiteral("TimeTable Next"));
    tray_->setContextMenu(menu);
    tray_->show();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if(!reallyQuit_ && tray_ && tray_->isVisible()) {
        hide();event->ignore();
        tray_->showMessage(QStringLiteral("TimeTable Next"),
            QStringLiteral("已最小化到系统托盘，右键托盘图标可退出。"),
            QSystemTrayIcon::Information,2500);
    } else {
        reallyQuit_=true;
        QApplication::quit();
        event->accept();
    }
}
void MainWindow::openPage(int index) {
    switchPage(std::clamp(index,0,stack_->count()-1));
}
void MainWindow::switchPage(int index) {
    if(!stack_ || index<0 || index>=stack_->count()) return;
    // QGraphicsOpacityEffect on a QStackedWidget page causes native Qt Widgets
    // child surfaces (tables/scroll areas) to display stale pixels on Windows.
    // Never attach an effect to the page; a deterministic stack switch is faster.
    if(stack_->currentIndex()!=index) stack_->setCurrentIndex(index);
    for(int i=0;i<navigation_.size();++i) {
        navigation_[i]->setChecked(i==index);
        navigation_[i]->style()->unpolish(navigation_[i]);
        navigation_[i]->style()->polish(navigation_[i]);
    }
    const QStringList heading={QStringLiteral("今日"),QStringLiteral("周课表"),
        QStringLiteral("课程管理"),QStringLiteral("特殊日期"),QStringLiteral("设置")};
    const QStringList subheading={QStringLiteral("当前课程与近期安排"),
        QStringLiteral("按教学周查看课程时间"),QStringLiteral("添加和编辑课程"),
        QStringLiteral("调休、补课和节假日"),QStringLiteral("外观、学期和节次")};
    if(auto* h=findChild<QLabel*>("pageTitle")) h->setText(heading.value(index));
    if(auto* h=findChild<QLabel*>("pageSubtitle")) h->setText(subheading.value(index));
    stack_->update();
}

void MainWindow::applyTheme() {
    // Qt adopts the host OS dark palette by default. Force a complete, explicit
    // application palette so light windows remain readable on dark-mode Windows.
    QPalette palette;
    const QColor background(dark_?"#10121E":"#F3F5FB");
    const QColor paletteSurface(dark_?"#1C2030":"#FFFFFF");
    const QColor foreground(dark_?"#F1F2FA":"#1E2334");
    const QColor muted(dark_?"#AFB6CF":"#59677F");
    palette.setColor(QPalette::Window,background);
    palette.setColor(QPalette::WindowText,foreground);
    palette.setColor(QPalette::Base,paletteSurface);
    palette.setColor(QPalette::AlternateBase,background);
    palette.setColor(QPalette::Text,foreground);
    palette.setColor(QPalette::Button,paletteSurface);
    palette.setColor(QPalette::ButtonText,foreground);
    palette.setColor(QPalette::BrightText,QColor("#FFFFFF"));
    palette.setColor(QPalette::ToolTipBase,paletteSurface);
    palette.setColor(QPalette::ToolTipText,foreground);
    palette.setColor(QPalette::Highlight,QColor("#6763DE"));
    palette.setColor(QPalette::HighlightedText,QColor("#FFFFFF"));
    palette.setColor(QPalette::Link,QColor("#7774F0"));
    palette.setColor(QPalette::PlaceholderText,muted);
    palette.setColor(QPalette::Disabled,QPalette::Text,muted);
    palette.setColor(QPalette::Disabled,QPalette::WindowText,muted);
    palette.setColor(QPalette::Disabled,QPalette::ButtonText,muted);
    qApp->setPalette(palette);
    const QString bg=dark_?"#10121E":"#F3F5FB";
    const QString surface=dark_?"#1C2030":"#FFFFFF";
    const QString text=dark_?"#F1F2FA":"#1E2334";
    const QString subdued=dark_?"#AFB6CF":"#737E95";
    const QString border=dark_?"#363C53":"#E7EAF3";
    const QString hover=dark_?"#323952":"#F1F2FC";
    setStyleSheet(QString(R"(
        QWidget#root { background: %1; color: %3; }
        QWidget { color: %3; }
        QFrame#sidebar { background: %2; border-right: 1px solid %5; }
        QFrame#card { background: %2; border: 1px solid %5; border-radius: 18px; }
        QFrame#hero { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #5654D8,stop:0.58 #726BEB,stop:1 #A68CFA); border-radius: 22px; }
        QLabel#heroTitle { color: white; font-weight: 700; }
        QLabel#heroSmall, QLabel#heroSub, QLabel#heroCountdown { color: #F6F3FF; }
        QLabel#pageTitle, QLabel#sectionHeading, QLabel#brand { font-weight: 700; }
        QLabel#brand { color: #726BEB; letter-spacing: 2px; }
        QLabel#muted { color: %4; }
        QLabel#clock { color: %4; font-weight: 600; }
        QLabel#upcoming { line-height: 130%%; }
        QPushButton { background: %2; border: 1px solid %5; border-radius: 11px; padding: 7px 14px; font-weight: 600; }
        QPushButton:hover { background: %6; border: 1px solid #9997F6; }
        QPushButton:pressed { background: #DAD7FF; color: #33335D; }
        QPushButton#primaryButton { color: white; background: #6763DE; border: 1px solid #6763DE; }
        QPushButton#primaryButton:hover { background: #7774F0; }
        QPushButton[navigation="true"] { text-align: left; border: none; background: transparent; padding: 10px 14px; border-radius: 12px; }
        QPushButton[navigation="true"]:hover { background: %6; }
        QPushButton[navigation="true"]:checked { background: %6; color: #726BEB; font-weight: 700; }
        QLineEdit, QDateEdit, QTimeEdit, QSpinBox, QComboBox { background: %2; color: %3; border: 1px solid %5; border-radius: 8px; padding: 7px; min-height: 23px; }
        QTableWidget, QListWidget { background: %2; color: %3; border: 1px solid %5; border-radius: 12px; gridline-color: %5; selection-background-color: #7774E6; selection-color: white; }
        QTableWidget::item, QListWidget::item { padding: 5px; }
        QHeaderView::section { background: %6; color: %3; border: none; padding: 9px; font-weight: 700; }
        QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; border: none; }
        QProgressBar { background: #AAA7E9; border: none; border-radius: 4px; }
        QProgressBar::chunk { background: white; border-radius: 4px; }
        QStatusBar { background: %2; color: %4; border-top: 1px solid %5; }
        QMenu { background: %2; color: %3; border: 1px solid %5; padding: 5px; }
        QMenu::item:selected { background: %6; }
    )").arg(bg,surface,text,subdued,border,hover));
}

void MainWindow::updateDashboard() {
    const auto now=QDateTime::currentDateTime();
    clockLabel_->setText(now.toString("yyyy/MM/dd  dddd\nHH:mm:ss"));
    const int week=store_.weekNumber(now.date());
    const Course* current=store_.currentCourse(now);
    if(current) {
        heroEyebrow_->setText(QStringLiteral("正在进行 · 第 %1 周").arg(week));
        heroCourse_->setText(current->name);
        heroDetail_->setText(timeRange(*current)+"   "+description(*current));
        const int total=current->start.secsTo(current->end);
        const int spent=current->start.secsTo(now.time());
        heroCountdown_->setText(QStringLiteral("距离下课  %1").arg(remain(total-spent)));
        if(auto* progress=findChild<QProgressBar*>("lessonProgress"))
            progress->setValue(total>0 ? std::clamp(spent*100/total,0,100) : 0);
    } else {
        heroEyebrow_->setText(week ? QStringLiteral("第 %1 周 · 轻松掌控节奏").arg(week) :
            QStringLiteral("学期外 · 可在设置中调整开学日期"));
        heroCourse_->setText(QStringLiteral("现在没有正在进行的课程"));
        heroDetail_->setText(QStringLiteral("合理规划空闲时间，也是一种进步。"));
        heroCountdown_->setText(QStringLiteral("今天是 %1").arg(now.date().toString("MM 月 dd 日 dddd")));
        if(auto* progress=findChild<QProgressBar*>("lessonProgress")) progress->setValue(0);
    }

    Course next;QDateTime when;
    if(store_.nextCourse(now,&next,&when)) {
        nextLabel_->setText(QStringLiteral("%1\n\n%2\n%3\n\n还有 %4")
            .arg(next.name,when.toString("MM/dd dddd HH:mm"),description(next),
                remain(now.secsTo(when))));
    } else nextLabel_->setText(QStringLiteral("未来两周没有待上的课程。\n可以先添加一节课，或者调整学期起始日期。"));

    // Refresh today's cards only when the date or timetable changes, not every second.
    if (todayList_->property("renderedDate").toDate()==now.date()) return;
    todayList_->setProperty("renderedDate",now.date());
    clear(todayList_);
    const auto dayCourses=store_.coursesForDate(now.date());
    if(dayCourses.isEmpty()) todayList_->addWidget(label(QStringLiteral("今天没有课程，享受自由时间。"),"muted"));
    else for(const auto& c:dayCourses) {
        auto* row=card();
        auto* h=new QHBoxLayout(row);
        h->setContentsMargins(13,10,13,10);
        h->setSpacing(12);
        auto* dot=label(QStringLiteral("●"),"",17);
        dot->setStyleSheet("color:"+c.color+";");
        h->addWidget(dot);
        auto* words=new QVBoxLayout;
        words->addWidget(label(c.name,"",12));
        words->addWidget(label(description(c),"muted"));
        h->addLayout(words,1);
        h->addWidget(label(timeRange(c)));
        if(c.start<=now.time() && now.time()<c.end)
            row->setStyleSheet("QFrame#card{border: 2px solid #817AF7;}");
        todayList_->addWidget(row);
    }
    todayList_->addStretch();
}

void MainWindow::updateWeek() {
    shownWeek_=std::clamp(shownWeek_,1,store_.totalWeeks);
    const QDate monday=store_.semesterMonday.addDays((shownWeek_-1)*7);
    weekLabel_->setText(QStringLiteral("第 %1 / %2 周").arg(shownWeek_).arg(store_.totalWeeks));
    auto* outer=static_cast<QHBoxLayout*>(weekContent_->layout());
    clear(outer);
    for(int i=0;i<7;++i) {
        const QDate date=monday.addDays(i);
        const auto lessons=store_.coursesForDate(date);
        auto* day=card();
        day->setMinimumWidth(161);
        day->setMaximumWidth(240);
        auto* vertical=new QVBoxLayout(day);
        vertical->setContentsMargins(11,13,11,13);vertical->setSpacing(9);
        auto* heading=label(kDays.at(i)+"\n"+date.toString("MM/dd"),"sectionHeading",13);
        heading->setAlignment(Qt::AlignCenter);
        if(date==QDate::currentDate()) heading->setStyleSheet("color:#746FF1;");
        vertical->addWidget(heading);
        if(lessons.isEmpty()) vertical->addWidget(label(QStringLiteral("— 暂无课程 —"),"muted"));
        for(const auto& c:lessons) {
            auto* block=card();
            block->setStyleSheet(QString("QFrame#card { background:%1; border: none; border-radius: 11px; }"
                "QLabel {color:white;}").arg(c.color));
            auto* inside=new QVBoxLayout(block);
            inside->setContentsMargins(10,12,10,12);inside->setSpacing(6);
            auto* name=label(c.name,"",11);
            auto f=name->font();f.setBold(true);name->setFont(f);
            inside->addWidget(name);
            inside->addWidget(label(timeRange(c)));
            if(!c.room.isEmpty()) inside->addWidget(label(c.room));
            vertical->addWidget(block);
        }
        vertical->addStretch();
        outer->addWidget(day);
    }
    outer->addStretch();
}

void MainWindow::updateCourseTable() {
    courseTable_->setRowCount(store_.courses.size());
    for(int i=0;i<store_.courses.size();++i) {
        const auto& c=store_.courses.at(i);
        QStringList values={c.name,kDays.value(c.weekday-1),timeRange(c),
            ScheduleStore::formatWeeks(c.weeks),c.room,c.teacher,c.color};
        for(int j=0;j<values.size();++j) {
            auto* item=new QTableWidgetItem(values.at(j));
            if(j==6) item->setForeground(QColor(c.color));
            courseTable_->setItem(i,j,item);
        }
    }
}
void MainWindow::updateChanges() {
    exceptionList_->clear();
    for(const auto& c:store_.changes) {
        exceptionList_->addItem(c.date.toString("yyyy-MM-dd dddd") +
            (c.holiday ? QStringLiteral("   ·   休息日") :
                QStringLiteral("   ·   改按 %1 上课").arg(c.sourceDate.toString("yyyy-MM-dd dddd"))));
    }
}
void MainWindow::fillSettings() {
    semesterDate_->setDate(store_.semesterMonday);
    weeksSpin_->setValue(store_.totalWeeks);
    alwaysOnTop_->setChecked(windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    darkMode_->setChecked(dark_);
    periodsTable_->setRowCount(store_.periods.size());
    int i=0;
    for(auto it=store_.periods.cbegin();it!=store_.periods.cend();++it,++i) {
        auto* number=new QTableWidgetItem(QString::number(it.key()));
        number->setFlags(number->flags() & ~Qt::ItemIsEditable);
        periodsTable_->setItem(i,0,number);
        periodsTable_->setItem(i,1,new QTableWidgetItem(it.value().first.toString("HH:mm")));
        periodsTable_->setItem(i,2,new QTableWidgetItem(it.value().second.toString("HH:mm")));
    }
}
void MainWindow::saveAndRefresh() {
    // Invalidate the cached list after any lesson / holiday edit.
    todayList_->setProperty("renderedDate",QVariant());
    QString error;
    if(!store_.save(dataPath_,&error))
        QMessageBox::warning(this,QStringLiteral("保存失败"),error);
    updateDashboard();
    updateWeek();
    updateCourseTable();
    updateChanges();
    if(onScheduleChanged_) onScheduleChanged_();
}
void MainWindow::showMessage(const QString& text) {statusBar()->showMessage(text,7000);}

void MainWindow::editCourse(int index) {
    if(index>=store_.courses.size()) return;
    QDialog dialog(this);
    dialog.setWindowTitle(index<0?QStringLiteral("添加课程"):QStringLiteral("编辑课程"));
    dialog.setMinimumWidth(440);
    auto* box=new QVBoxLayout(&dialog);
    auto* form=new QFormLayout;form->setSpacing(12);
    Course course=index>=0?store_.courses.at(index):Course{};
    auto* name=new QLineEdit(course.name);
    name->setPlaceholderText(QStringLiteral("课程或活动名称"));
    auto* weekday=new QComboBox;
    weekday->addItems(kDays);
    weekday->setCurrentIndex(course.weekday-1);
    auto* start=new QTimeEdit(course.start.isValid()?course.start:QTime(8,0));
    start->setDisplayFormat("HH:mm");
    auto* end=new QTimeEdit(course.end.isValid()?course.end:QTime(9,40));
    end->setDisplayFormat("HH:mm");
    auto* weeks=new QLineEdit(course.weeks.isEmpty()?QStringLiteral("全部"):
        ScheduleStore::formatWeeks(course.weeks));
    auto* teacher=new QLineEdit(course.teacher);
    auto* room=new QLineEdit(course.room);
    auto* color=new QComboBox;
    const QStringList colors={"#6C6AF6","#1DAF9C","#F18E64","#3F91DB","#E369A9","#A06BD6","#DAA237"};
    for(const auto& c:colors) color->addItem(c,c);
    int selected=colors.indexOf(course.color);
    color->setCurrentIndex(selected<0?0:selected);
    form->addRow(QStringLiteral("课程名称"),name);
    form->addRow(QStringLiteral("星期"),weekday);
    form->addRow(QStringLiteral("上课时间"),start);
    form->addRow(QStringLiteral("下课时间"),end);
    form->addRow(QStringLiteral("教学周数"),weeks);
    form->addRow(QStringLiteral("授课教师（可选）"),teacher);
    form->addRow(QStringLiteral("地点（可选）"),room);
    form->addRow(QStringLiteral("卡片配色"),color);
    box->addLayout(form);
    box->addWidget(label(QStringLiteral("可用「1-16单」「2-18双」或「1-8,10,12」设置持续周。"),"muted"));
    auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    actions->button(QDialogButtonBox::Save)->setText(QStringLiteral("保存"));
    actions->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    box->addWidget(actions);
    connect(actions,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    connect(actions,&QDialogButtonBox::accepted,&dialog,[&,this]{
        Course edited=course;
        if(edited.id.isEmpty()) edited.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
        edited.name=name->text().trimmed();
        edited.weekday=weekday->currentIndex()+1;
        edited.start=start->time();edited.end=end->time();
        QString error;
        edited.weeks=ScheduleStore::parseWeeks(weeks->text(),store_.totalWeeks,&error);
        edited.teacher=teacher->text().trimmed();
        edited.room=room->text().trimmed();
        edited.color=color->currentData().toString();
        if(!ScheduleStore::validCourse(edited,&error)) {
            QMessageBox::warning(&dialog,QStringLiteral("课程信息不完整"),error);
            return;
        }
        if(index<0) store_.courses<<edited;
        else store_.courses[index]=edited;
        saveAndRefresh();
        showMessage(QStringLiteral("课程已保存"));
        dialog.accept();
    });
    dialog.exec();
}

void MainWindow::removeCourse() {
    const int row=courseTable_->currentRow();
    if(row<0 || row>=store_.courses.size()) return;
    if(QMessageBox::question(this,QStringLiteral("删除课程"),
        QStringLiteral("确认删除「%1」？").arg(store_.courses[row].name))!=QMessageBox::Yes) return;
    store_.courses.removeAt(row);
    saveAndRefresh();
}
void MainWindow::duplicateCourse() {
    const int row=courseTable_->currentRow();
    if(row<0 || row>=store_.courses.size()) return;
    Course c=store_.courses.at(row);
    c.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    c.name+=QStringLiteral("（副本）");
    store_.courses<<c;
    saveAndRefresh();
    courseTable_->selectRow(store_.courses.size()-1);
}
void MainWindow::addDayChange() {
    DayChange change;
    change.date=changeDate_->date();
    change.holiday=holiday_->isChecked();
    change.sourceDate=sourceDate_->date();
    for(int i=0;i<store_.changes.size();++i)
        if(store_.changes[i].date==change.date) {
            store_.changes[i]=change;
            saveAndRefresh();showMessage(QStringLiteral("已更新特殊日期"));
            return;
        }
    store_.changes<<change;
    std::sort(store_.changes.begin(),store_.changes.end(),
        [](const DayChange&a,const DayChange&b){return a.date<b.date;});
    saveAndRefresh();showMessage(QStringLiteral("日期调整已保存"));
}
void MainWindow::removeDayChange() {
    const int row=exceptionList_->currentRow();
    if(row<0 || row>=store_.changes.size()) return;
    store_.changes.removeAt(row);
    saveAndRefresh();
}
void MainWindow::importSchedule() {
    const QString path=QFileDialog::getOpenFileName(this,QStringLiteral("导入课表"),
        QString(),QStringLiteral("课表文件 (*.json *.csv);;JSON (*.json);;CSV (*.csv)"));
    if(path.isEmpty()) return;
    if(QMessageBox::question(this,QStringLiteral("确认导入"),
        QStringLiteral("导入将替换当前课表；建议先导出备份。继续吗？"))!=QMessageBox::Yes) return;
    QString error;
    if(!store_.importFile(path,&error)) {
        QMessageBox::warning(this,QStringLiteral("导入失败"),error);
        return;
    }
    shownWeek_=std::max(1,store_.weekNumber(QDate::currentDate()));
    fillSettings();saveAndRefresh();
    showMessage(QStringLiteral("导入成功：%1 门课程").arg(store_.courses.size()));
}
void MainWindow::exportSchedule() {
    QString filter;
    QString path=QFileDialog::getSaveFileName(this,QStringLiteral("导出课表"),
        QStringLiteral("TimeTableNext-Backup.json"),
        QStringLiteral("TimeTable Next JSON (*.json);;CSV 表格 (*.csv);;日历文件 (*.ics);;小爱结构兼容 JSON (*.xiaomi.json)"),
        &filter);
    if(path.isEmpty()) return;
    QString error;bool ok=false;
    if(filter.startsWith(QStringLiteral("CSV"))) {
        if(!path.endsWith(".csv",Qt::CaseInsensitive)) path+=".csv";
        ok=store_.exportCsv(path,&error);
    } else if(filter.startsWith(QStringLiteral("日历"))) {
        if(!path.endsWith(".ics",Qt::CaseInsensitive)) path+=".ics";
        ok=store_.exportIcs(path,&error);
    } else if(filter.startsWith(QStringLiteral("小爱"))) {
        if(!path.endsWith(".json",Qt::CaseInsensitive)) path+=".xiaomi.json";
        ok=store_.exportXiaoAiJson(path,&error);
    } else {
        if(!path.endsWith(".json",Qt::CaseInsensitive)) path+=".json";
        ok=store_.save(path,&error);
    }
    if(!ok) QMessageBox::warning(this,QStringLiteral("导出失败"),error);
    else showMessage(QStringLiteral("已导出：%1").arg(path));
}
void MainWindow::saveSettings() {
    const int newTotal=weeksSpin_->value();
    for(const auto& c:store_.courses) for(int w:c.weeks) if(w>newTotal) {
        QMessageBox::warning(this,QStringLiteral("周数冲突"),
            QStringLiteral("课程「%1」使用第 %2 周，无法将学期缩短为 %3 周。")
                .arg(c.name).arg(w).arg(newTotal));
        return;
    }
    QMap<int,QPair<QTime,QTime>> newPeriods;
    for(int row=0;row<periodsTable_->rowCount();++row) {
        const int section=periodsTable_->item(row,0)->text().toInt();
        const QTime start=QTime::fromString(periodsTable_->item(row,1)->text().trimmed(),"HH:mm");
        const QTime end=QTime::fromString(periodsTable_->item(row,2)->text().trimmed(),"HH:mm");
        if(!start.isValid() || !end.isValid() || start>=end) {
            QMessageBox::warning(this,QStringLiteral("时间格式错误"),
                QStringLiteral("第 %1 节时间无效，格式应为 HH:mm 且结束晚于开始").arg(section));
            return;
        }
        newPeriods[section]={start,end};
    }
    store_.semesterMonday=ScheduleStore::mondayOf(semesterDate_->date());
    store_.totalWeeks=newTotal;
    store_.periods=newPeriods;
    const bool top=alwaysOnTop_->isChecked();
    if(windowFlags().testFlag(Qt::WindowStaysOnTopHint)!=top) {
        setWindowFlag(Qt::WindowStaysOnTopHint,top);show();
    }
    dark_=darkMode_->isChecked();
    QSettings().setValue("dark",dark_);
    applyTheme();
    shownWeek_=std::clamp(shownWeek_,1,newTotal);
    saveAndRefresh();
    fillSettings();
    showMessage(QStringLiteral("设置已保存"));
}
} // namespace tt
