#include "MainWindow.h"
#include "Translation.h"

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
#include <QFileInfo>
#include <QDir>
#include <QInputDialog>
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
#include <QRegularExpression>
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
QStringList dayNames() { return {tt::translate("周一"),tt::translate("周二"),tt::translate("周三"),
    tt::translate("周四"),tt::translate("周五"),tt::translate("周六"),tt::translate("周日")}; }

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
    if(days) return tt::translate("%1 天 %2 小时").arg(days).arg(hours);
    if(hours) return tt::translate("%1 小时 %2 分").arg(hours).arg(minutes);
    return tt::translate("%1 分 %2 秒").arg(minutes).arg(seconds%60);
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
QString absolutePath(const QString& path) { return QDir::cleanPath(QFileInfo(path).absoluteFilePath()); }
bool samePath(const QString& first,const QString& second) {
    const QFileInfo a(first),b(second);
    if(!a.canonicalFilePath().isEmpty()&&!b.canonicalFilePath().isEmpty())
        return a.canonicalFilePath()==b.canonicalFilePath();
#ifdef Q_OS_WIN
    return absolutePath(first).compare(absolutePath(second),Qt::CaseInsensitive)==0;
#else
    return absolutePath(first)==absolutePath(second);
#endif
}
} // namespace

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent) {
    setWindowTitle(tt::translate("TimeTable Next · 课表与日程"));
    resize(1080,750);
    setMinimumSize(835,580);
    const QString defaultPath=ScheduleStore::defaultPath();
    dataPath_=QSettings().value("schedule/path",defaultPath).toString();
    if(dataPath_.isEmpty()) dataPath_=defaultPath;
    dataPath_=QFileInfo(dataPath_).absoluteFilePath();
    if(QFile::exists(dataPath_)) {
        QString error;
        if(!store_.load(dataPath_,&error)) {
            canSaveSchedule_=false;
            QMessageBox::warning(this,tt::translate("读取课表失败"),
                tt::translate("原始文件保持不变，课表自动保存已暂停。后续修改仅保留在内存中，可导出备份。\n"
                               "请修复文件后重新启动，或明确导入有效课表以替换原始文件。\n%1\n%2")
                    .arg(dataPath_,error));
        }
    } else if(dataPath_!=QFileInfo(defaultPath).absoluteFilePath()) {
        canSaveSchedule_=false;
        QMessageBox::warning(this,tt::translate("读取课表失败"),
            tt::translate("自定义课表文件不存在，课表自动保存已暂停。请选择有效课表或另存为新文件。\n%1").arg(dataPath_));
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
    remind_=QSettings().value("reminders/enabled",false).toBool();
    minutesBefore_=std::clamp(QSettings().value("reminders/minutesBefore",10).toInt(),0,1440);
    if(QSettings().value("reminders/notifiedDate").toDate()==QDate::currentDate())
        for(const auto& key:QSettings().value("reminders/notified").toStringList()) notifiedReminders_.insert(key);
    buildUi();
    buildTray();
    applyTheme();
    fillSettings();
    saveAndRefresh();
    auto* timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this] { updateDashboard(); checkReminders(); });
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
    side->addWidget(label(tt::translate("桌面课程与日程"),"sidebarSubtitle",9));
    side->addSpacing(30);
    side->addWidget(label(tt::translate("工作区"),"sidebarGroup",9));
    const QStringList tabs={tt::translate("  今日"),tt::translate("  周课表"),
        tt::translate("  课程"),tt::translate("  特殊日期"),
        tt::translate("  设置")};
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
    side->addWidget(label(tt::translate("桌面窗口"),"sidebarGroup",9));
    auto* overlaysBtn=button(tt::translate("管理浮动组件"));
    side->addWidget(overlaysBtn);
    connect(overlaysBtn,&QPushButton::clicked,this,[this] { if(onOpenOverlaySettings_) onOpenOverlaySettings_(); });
    auto* importBtn=button(tt::translate("导入数据"));
    auto* exportBtn=button(tt::translate("导出与备份"));
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
    titles->addWidget(label(tt::translate("今日"),"pageTitle",22));
    titles->addWidget(label(tt::translate("当前课程与近期安排"),"pageSubtitle",10));
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
    heroEyebrow_=label(tt::translate("今天的安排"),"heroSmall",13);
    heroCourse_=label(tt::translate("准备就绪"),"heroTitle",27);
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
    dayLayout->addWidget(label(tt::translate("今日课程"),"sectionHeading",17));
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
    nextLayout->addWidget(label(tt::translate("接下来"),"sectionHeading",17));
    nextLabel_=label("", "upcoming",15);
    nextLayout->addWidget(nextLabel_,1,Qt::AlignTop);
    auto* weekBtn=button(tt::translate("查看完整周课表  →"),true);
    auto* changeBtn=button(tt::translate("安排临时调休  →"));
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
    auto* title=label(tt::translate("一周的全貌"),"sectionHeading",19);
    bar->addWidget(title);bar->addStretch();
    auto* prev=button(tt::translate("← 上一周"));
    auto* next=button(tt::translate("下一周 →"));
    auto* current=button(tt::translate("回到本周"),true);
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
    v->addWidget(label(tt::translate("提示：课程卡片按实际日期显示；调休和教学周规则会自动生效。"),"muted"));
    return page;
}

QWidget* MainWindow::createCoursesPage() {
    auto* page=new QWidget;
    auto* v=new QVBoxLayout(page);v->setContentsMargins(0,0,0,0);v->setSpacing(12);
    auto* bar=new QHBoxLayout;
    bar->addWidget(label(tt::translate("课程管理"),"sectionHeading",19));
    bar->addStretch();
    auto* add=button(tt::translate("＋ 新建课程"),true);
    auto* edit=button(tt::translate("编辑"));
    auto* copy=button(tt::translate("复制"));
    auto* remove=button(tt::translate("删除"));
    auto* conflicts=button(tt::translate("检查冲突"));
    bar->addWidget(add);bar->addWidget(edit);bar->addWidget(copy);bar->addWidget(remove);bar->addWidget(conflicts);
    v->addLayout(bar);
    courseSearch_=new QLineEdit;
    courseSearch_->setObjectName("courseSearch");
    courseSearch_->setPlaceholderText(tt::translate("搜索课程、教师、地点、星期或教学周（空格分隔多个条件）"));
    v->addWidget(courseSearch_);
    conflictSummary_=label(QString(),"muted");
    v->addWidget(conflictSummary_);
    courseTable_=new QTableWidget;
    courseTable_->setObjectName("courseTable");
    courseTable_->setColumnCount(7);
    courseTable_->setHorizontalHeaderLabels({tt::translate("课程名称"),tt::translate("星期"),
        tt::translate("上课时间"),tt::translate("教学周"),tt::translate("地点"),
        tt::translate("教师"),tt::translate("配色")});
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
    v->addWidget(label(tt::translate("教学周示例：全部 / 1-16 / 1-16单 / 2-18双 / 1-8,10,12"),"muted"));
    connect(add,&QPushButton::clicked,this,[this]{editCourse();});
    connect(edit,&QPushButton::clicked,this,[this]{const int index=selectedCourseIndex(); if(index>=0) editCourse(index);});
    connect(copy,&QPushButton::clicked,this,[this]{duplicateCourse();});
    connect(remove,&QPushButton::clicked,this,[this]{removeCourse();});
    connect(courseSearch_,&QLineEdit::textChanged,this,[this]{updateCourseTable();});
    connect(conflicts,&QPushButton::clicked,this,[this]{showConflicts();});
    connect(courseTable_,&QTableWidget::cellDoubleClicked,this,[this](int,int){editCourse(selectedCourseIndex());});
    return page;
}

QWidget* MainWindow::createChangesPage() {
    auto* page=new QWidget;
    auto* v=new QVBoxLayout(page);v->setContentsMargins(0,0,0,0);v->setSpacing(15);
    v->addWidget(label(tt::translate("调休与特殊日期"),"sectionHeading",19));
    v->addWidget(label(tt::translate("快速将某一天设为休息日，或临时按照另一日期的课表上课。\n适合节假日补课、周末调课、考试周。"),"muted"));
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
    holiday_=new QCheckBox(tt::translate("休息日（当天不上课）"));
    holiday_->setChecked(true);
    sourceDate_->setEnabled(false);
    connect(holiday_,&QCheckBox::toggled,sourceDate_,[this](bool checked){sourceDate_->setEnabled(!checked);});
    form->addRow(tt::translate("要调整的日期"),changeDate_);
    form->addRow(tt::translate("安排类型"),holiday_);
    form->addRow(tt::translate("改按哪天上课"),sourceDate_);
    auto* add=button(tt::translate("保存日期调整"),true);
    auto* quick=button(tt::translate("一键将今天设为休息日"));
    auto* row=new QHBoxLayout;
    row->addWidget(add);row->addWidget(quick);
    form->addRow(row);
    v->addWidget(settings);
    v->addWidget(label(tt::translate("已设置的特殊日期"),"sectionHeading",16));
    exceptionList_=new QListWidget;
    exceptionList_->setAlternatingRowColors(false);
    v->addWidget(exceptionList_,1);
    auto* remove=button(tt::translate("删除选中的日期调整"));
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

    auto* intro=label(tt::translate("管理课表周期、显示偏好和每日作息。"),"settingDescription",10);
    column->addWidget(intro);
    column->addWidget(sectionTitle(tt::translate("学期")));

    auto* semester=fluentSection();
    auto* semesterRows=new QVBoxLayout(semester);
    semesterRows->setContentsMargins(0,0,0,0);
    semesterRows->setSpacing(0);
    semesterDate_=new QDateEdit;
    semesterDate_->setCalendarPopup(true);
    semesterDate_->setDisplayFormat("yyyy-MM-dd");
    semesterDate_->setMinimumWidth(170);
    semesterRows->addWidget(fluentSettingRow(tt::translate("学期开始日期"),
        tt::translate("以该日期所在的星期一作为第一周"),semesterDate_));
    auto* line1=new QFrame;line1->setObjectName("settingDivider");
    line1->setFixedHeight(1);
    semesterRows->addWidget(line1);
    weeksSpin_=new QSpinBox;
    weeksSpin_->setRange(1,60);
    weeksSpin_->setSuffix(tt::translate(" 周"));
    semesterRows->addWidget(fluentSettingRow(tt::translate("教学周数"),
        tt::translate("课程的单双周与指定周以此为范围"),weeksSpin_));
    column->addWidget(semester);

    column->addWidget(sectionTitle(tt::translate("课表文件与学期档案")));
    auto* files=fluentSection();
    auto* fileRows=new QVBoxLayout(files);
    fileRows->setContentsMargins(18,16,18,18);
    schedulePathLabel_=label(QString(),"settingDescription",9);
    schedulePathLabel_->setObjectName("schedulePath");
    schedulePathLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    fileRows->addWidget(schedulePathLabel_);
    auto* fileActions=new QHBoxLayout;
    auto* choose=button(tt::translate("选择课表文件"));
    auto* saveAs=button(tt::translate("课表另存为"));
    auto* resetPath=button(tt::translate("恢复默认课表"));
    fileActions->addWidget(choose); fileActions->addWidget(saveAs); fileActions->addWidget(resetPath);
    fileRows->addLayout(fileActions);
    connect(choose,&QPushButton::clicked,this,[this]{chooseSchedulePath();});
    connect(saveAs,&QPushButton::clicked,this,[this]{chooseSaveScheduleAs();});
    connect(resetPath,&QPushButton::clicked,this,[this]{
        QString error; const QString path=ScheduleStore::defaultPath();
        const bool ok=QFile::exists(path)?switchSchedulePath(path,&error):saveScheduleAs(path,&error);
        if(!ok) QMessageBox::warning(this,tt::translate("切换课表失败"),error);
        else showMessage(tt::translate("已恢复默认课表路径"));
    });
    fileRows->addWidget(label(tt::translate("每个学期可以使用独立 JSON 文件；切换前先完整校验，失败会保留当前课表。"),"settingDescription",9));
    auto* profileActions=new QHBoxLayout;
    profiles_=new QComboBox;
    profiles_->setObjectName("scheduleProfiles");
    profileActions->addWidget(profiles_,1);
    auto* switchProfile=button(tt::translate("切换学期"));
    auto* recordProfile=button(tt::translate("命名当前档案"));
    auto* removeProfile=button(tt::translate("移除档案记录"));
    profileActions->addWidget(switchProfile); profileActions->addWidget(recordProfile); profileActions->addWidget(removeProfile);
    fileRows->addLayout(profileActions);
    connect(recordProfile,&QPushButton::clicked,this,[this]{registerProfile();});
    connect(switchProfile,&QPushButton::clicked,this,[this]{
        const QString path=profiles_->currentData().toString();
        if(path.isEmpty()) return;
        QString error;
        if(!switchSchedulePath(path,&error)) QMessageBox::warning(this,tt::translate("切换课表失败"),error);
        else showMessage(tt::translate("已切换学期档案"));
    });
    connect(removeProfile,&QPushButton::clicked,this,[this]{
        if(profiles_->currentData().toString().isEmpty()) return;
        auto records=QSettings().value("schedule/profiles").toMap();
        records.remove(profiles_->currentText());
        QSettings().setValue("schedule/profiles",records); fillPathSettings();
        showMessage(tt::translate("档案记录已移除，课表文件仍保留"));
    });
    auto* convert=button(tt::translate("将导入文件转换为独立 JSON"));
    fileRows->addWidget(convert);
    connect(convert,&QPushButton::clicked,this,[this]{convertImport();});
    column->addWidget(files);

    column->addWidget(sectionTitle(tt::translate("课程提醒")));
    auto* reminders=fluentSection();
    auto* reminderRows=new QVBoxLayout(reminders);
    reminderRows->setContentsMargins(0,0,0,0);
    remindersEnabled_=new QCheckBox(tt::translate("启用上课提醒"));
    remindersEnabled_->setObjectName("remindersEnabled");
    reminderRows->addWidget(fluentSettingRow(tt::translate("系统通知"),
        tt::translate("默认关闭；启用后按实际日期和调休发送一次提醒，无系统托盘时显示状态栏提示"),remindersEnabled_));
    reminderMinutes_=new QSpinBox;
    reminderMinutes_->setObjectName("reminderMinutes");
    reminderMinutes_->setRange(0,1440); reminderMinutes_->setSuffix(tt::translate(" 分钟"));
    reminderRows->addWidget(fluentSettingRow(tt::translate("提前时间"),
        tt::translate("0 表示上课时提醒；设置保存后生效"),reminderMinutes_));
    column->addWidget(reminders);

    column->addWidget(sectionTitle(tt::translate("个性化")));
    auto* appearance=fluentSection();
    auto* appearanceRows=new QVBoxLayout(appearance);
    appearanceRows->setContentsMargins(0,0,0,0);
    appearanceRows->setSpacing(0);
    darkMode_=new QCheckBox;
    darkMode_->setText(tt::translate("使用深色主题"));
    appearanceRows->addWidget(fluentSettingRow(tt::translate("应用主题"),
        tt::translate("明确指定浅色或深色，不受 Windows 主题影响"),darkMode_));
    auto* line2=new QFrame;line2->setObjectName("settingDivider");line2->setFixedHeight(1);
    appearanceRows->addWidget(line2);
    alwaysOnTop_=new QCheckBox;
    alwaysOnTop_->setText(tt::translate("管理器置顶"));
    appearanceRows->addWidget(fluentSettingRow(tt::translate("窗口层级"),
        tt::translate("常驻信息卡片的置顶策略请在浮动组件中设置"),alwaysOnTop_));
    language_=new QComboBox;
    language_->addItem(tt::translate("中文"),"zh_CN");
    language_->addItem("English","en");
    appearanceRows->addWidget(fluentSettingRow(tt::translate("界面语言"),
        tt::translate("保存后重新启动应用生效"),language_));
    column->addWidget(appearance);

    column->addWidget(sectionTitle(tt::translate("节次时间")));
    auto* periods=fluentSection();
    auto* periodsLayout=new QVBoxLayout(periods);
    periodsLayout->setContentsMargins(18,16,18,18);
    periodsLayout->setSpacing(10);
    periodsLayout->addWidget(label(tt::translate("设置每一节课的起止时间，用于将第三方课程表节次转换为实际时刻。"),
        "settingDescription",9));
    periodsTable_=new QTableWidget;
    periodsTable_->setObjectName("periodsTable");
    periodsTable_->setColumnCount(3);
    periodsTable_->setHorizontalHeaderLabels({tt::translate("节次"),tt::translate("开始"),tt::translate("结束")});
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
    actions->addWidget(label(tt::translate("修改后需要点击保存才会写入本地配置"),"settingDescription",9),1);
    auto* save=button(tt::translate("保存更改"),true);
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
    auto* manageWindows=menu->addAction(tt::translate("管理桌面信息窗口"));
    auto* show=menu->addAction(tt::translate("打开课程管理器"));
    auto* holiday=menu->addAction(tt::translate("今天临时休息"));
    menu->addSeparator();
    auto* quit=menu->addAction(tt::translate("退出程序"));
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
            tt::translate("已最小化到系统托盘，右键托盘图标可退出。"),
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
    const QStringList heading={tt::translate("今日"),tt::translate("周课表"),
        tt::translate("课程管理"),tt::translate("特殊日期"),tt::translate("设置")};
    const QStringList subheading={tt::translate("当前课程与近期安排"),
        tt::translate("按教学周查看课程时间"),tt::translate("添加和编辑课程"),
        tt::translate("调休、补课和节假日"),tt::translate("外观、学期和节次")};
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
    const QString bg=dark_?"#111319":"#F5F5F7";
    const QString surface=dark_?"#25262B":"#FFFFFF";
    const QString text=dark_?"#F6F6F6":"#202124";
    const QString subdued=dark_?"#B9BBC2":"#62656D";
    const QString border=dark_?"#3D3E44":"#E1E3E7";
    const QString hover=dark_?"#343840":"#EAECF1";
    setStyleSheet(QString(R"(
        QWidget#root { background: %1; color: %3; }
        QWidget { color: %3; }
        QFrame#sidebar { background: %2; border-right: 1px solid %5; }
        QLabel#sidebarSubtitle { color:%4; }
        QLabel#sidebarGroup { color:%4; font-weight:600; padding-left: 10px; margin-top:5px; }
        QLabel#pageSubtitle {color:%4;}
        QLabel#settingLabel { color:%3; font-weight:600; }
        QLabel#settingDescription {color:%4;}
        QLabel#settingsGroupTitle {color:%3; font-weight:700; }
        QFrame#fluentSection {background:%2; border:1px solid %5; border-radius:10px;}
        QWidget#settingRow {background:transparent;}
        QFrame#settingDivider {background:%5; border:none;}
        QWidget#settingsBody {background:%1;}
        QScrollArea#settingsScroll {background:%1; border:none;}
        QTableWidget#periodsTable {border: 1px solid %5; border-radius:7px; }
        QFrame#card { background: %2; border: 1px solid %5; border-radius: 18px; }
        QFrame#hero { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #5654D8,stop:0.58 #726BEB,stop:1 #A68CFA); border-radius: 22px; }
        QLabel#heroTitle { color: white; font-weight: 700; }
        QLabel#heroSmall, QLabel#heroSub, QLabel#heroCountdown { color: #F6F3FF; }
        QLabel#pageTitle, QLabel#sectionHeading, QLabel#brand { font-weight: 700; }
        QLabel#brand { color: %3; font-weight: 700; letter-spacing:0; }
        QLabel#muted { color: %4; }
        QLabel#clock { color: %4; font-weight: 600; }
        QPushButton { background: %2; border: 1px solid %5; border-radius: 11px; padding: 7px 14px; font-weight: 600; }
        QPushButton:hover { background: %6; border: 1px solid #9997F6; }
        QPushButton:pressed { background: #DAD7FF; color: #33335D; }
        QPushButton#primaryButton { color:white; background:#625CDC; border:1px solid #625CDC; }
        QPushButton#primaryButton:hover { background:#7570EB; }
        QPushButton[navigation="true"] { color:%3; text-align:left; border:none; background:transparent; padding:10px 14px; border-radius:7px; font-weight:500; }
        QPushButton[navigation="true"]:hover { background: %6; }
        QPushButton[navigation="true"]:checked { background:%6; color:%3; border-left: 3px solid #625CDC; font-weight:600; }
        QLineEdit, QDateEdit, QTimeEdit, QSpinBox, QComboBox { background:%2; color:%3; border:1px solid %5; border-bottom:2px solid %4; border-radius:5px; padding:6px; min-height:22px; }
        QTableWidget, QListWidget { background:%2; color:%3; border:1px solid %5; border-radius:8px; gridline-color:%5; selection-background-color:#DBD9FF; selection-color:#292B42; }
        QTableWidget::item, QListWidget::item { padding: 5px; }
        QHeaderView::section {background:%6; color:%3; border:none; padding:9px; font-weight:600; }
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
        heroEyebrow_->setText(tt::translate("正在进行 · 第 %1 周").arg(week));
        heroCourse_->setText(current->name);
        heroDetail_->setText(timeRange(*current)+"   "+description(*current));
        const int total=current->start.secsTo(current->end);
        const int spent=current->start.secsTo(now.time());
        heroCountdown_->setText(tt::translate("距离下课  %1").arg(remain(total-spent)));
        if(auto* progress=findChild<QProgressBar*>("lessonProgress"))
            progress->setValue(total>0 ? std::clamp(spent*100/total,0,100) : 0);
    } else {
        heroEyebrow_->setText(week ? tt::translate("第 %1 周 · 轻松掌控节奏").arg(week) :
            tt::translate("学期外 · 可在设置中调整开学日期"));
        heroCourse_->setText(tt::translate("现在没有正在进行的课程"));
        heroDetail_->setText(tt::translate("合理规划空闲时间，也是一种进步。"));
        heroCountdown_->setText(tt::translate("今天是 %1").arg(now.date().toString(tt::translate("MM 月 dd 日 dddd"))));
        if(auto* progress=findChild<QProgressBar*>("lessonProgress")) progress->setValue(0);
    }

    Course next;QDateTime when;
    if(store_.nextCourse(now,&next,&when)) {
        nextLabel_->setText(tt::translate("%1\n\n%2\n%3\n\n还有 %4")
            .arg(next.name,when.toString("MM/dd dddd HH:mm"),description(next),
                remain(now.secsTo(when))));
    } else nextLabel_->setText(tt::translate("未来两周没有待上的课程。\n可以先添加一节课，或者调整学期起始日期。"));

    // Refresh today's cards only when the date or timetable changes, not every second.
    if (todayList_->property("renderedDate").toDate()==now.date()) return;
    todayList_->setProperty("renderedDate",now.date());
    clear(todayList_);
    const auto dayCourses=store_.coursesForDate(now.date());
    if(dayCourses.isEmpty()) todayList_->addWidget(label(tt::translate("今天没有课程，享受自由时间。"),"muted"));
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
    weekLabel_->setText(tt::translate("第 %1 / %2 周").arg(shownWeek_).arg(store_.totalWeeks));
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
        auto* heading=label(dayNames().at(i)+"\n"+date.toString("MM/dd"),"sectionHeading",13);
        heading->setAlignment(Qt::AlignCenter);
        if(date==QDate::currentDate()) heading->setStyleSheet("color:#746FF1;");
        vertical->addWidget(heading);
        if(lessons.isEmpty()) vertical->addWidget(label(tt::translate("— 暂无课程 —"),"muted"));
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
    const int selected=selectedCourseIndex();
    const auto terms=courseSearch_->text().trimmed().split(QRegularExpression("\\s+"),Qt::SkipEmptyParts);
    const QStringList englishDays={"Monday Mon","Tuesday Tue","Wednesday Wed","Thursday Thu","Friday Fri","Saturday Sat","Sunday Sun"};
    QList<int> visible;
    for(int i=0;i<store_.courses.size();++i) {
        const auto& c=store_.courses.at(i);
        const QString haystack=QStringList{c.name,c.teacher,c.room,dayNames().value(c.weekday-1),
            QStringLiteral("星期")+QStringLiteral("一二三四五六日").mid(c.weekday-1,1),englishDays.value(c.weekday-1),
            QString::number(c.weekday),ScheduleStore::formatWeeks(c.weeks),timeRange(c)}.join(" ");
        bool matches=true;
        for(const auto& term:terms) {
            bool numeric=false; const int week=term.toInt(&numeric);
            if(!haystack.contains(term,Qt::CaseInsensitive)&&!(numeric&&c.weeks.contains(week))) { matches=false; break; }
        }
        if(matches) visible<<i;
    }
    courseTable_->setRowCount(visible.size());
    for(int row=0;row<visible.size();++row) {
        const int i=visible.at(row); const auto& c=store_.courses.at(i);
        QStringList values={c.name,dayNames().value(c.weekday-1),timeRange(c),
            ScheduleStore::formatWeeks(c.weeks),c.room,c.teacher,c.color};
        for(int j=0;j<values.size();++j) {
            auto* item=new QTableWidgetItem(values.at(j));
            item->setData(Qt::UserRole,i);
            if(j==6) item->setForeground(QColor(c.color));
            courseTable_->setItem(row,j,item);
        }
        if(i==selected) courseTable_->selectRow(row);
    }
    const int conflictCount=store_.conflicts().size();
    conflictSummary_->setText(conflictCount?tt::translate("检测到 %1 组时间冲突；可点击「检查冲突」查看，重叠课程仍可保存。").arg(conflictCount):
        tt::translate("没有教学周与时间同时重叠的课程"));
}
int MainWindow::selectedCourseIndex() const {
    if(!courseTable_) return -1;
    const auto* item=courseTable_->item(courseTable_->currentRow(),0);
    return item?item->data(Qt::UserRole).toInt():-1;
}
void MainWindow::showConflicts() {
    const auto conflicts=store_.conflicts();
    if(conflicts.isEmpty()) { QMessageBox::information(this,tt::translate("课程冲突检查"),tt::translate("没有发现时间冲突。")); return; }
    QStringList lines;
    for(const auto& conflict:conflicts) {
        const auto& a=store_.courses.at(conflict.firstIndex); const auto& b=store_.courses.at(conflict.secondIndex);
        lines<<tt::translate("%1 · 第 %2 周\n「%3」%4\n「%5」%6")
            .arg(dayNames().value(a.weekday-1),ScheduleStore::formatWeeks(conflict.weeks),a.name,timeRange(a),b.name,timeRange(b));
    }
    QMessageBox report(QMessageBox::Information,tt::translate("课程冲突检查"),
        tt::translate("发现 %1 组重叠课程。此报告仅供检查，不会阻止保存。\n\n%2").arg(conflicts.size()).arg(lines.join("\n\n")),QMessageBox::Ok,this);
    report.setTextFormat(Qt::PlainText); report.exec();
}
void MainWindow::updateChanges() {
    exceptionList_->clear();
    for(const auto& c:store_.changes) {
        exceptionList_->addItem(c.date.toString("yyyy-MM-dd dddd") +
            (c.holiday ? tt::translate("   ·   休息日") :
                tt::translate("   ·   改按 %1 上课").arg(c.sourceDate.toString("yyyy-MM-dd dddd"))));
    }
}
void MainWindow::fillSettings() {
    semesterDate_->setDate(store_.semesterMonday);
    weeksSpin_->setValue(store_.totalWeeks);
    alwaysOnTop_->setChecked(windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    darkMode_->setChecked(dark_);
    remindersEnabled_->setChecked(remind_);
    reminderMinutes_->setValue(minutesBefore_);
    language_->setCurrentIndex(std::max(0,language_->findData(QSettings().value("ui/language","zh_CN").toString())));
    fillPathSettings();
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
void MainWindow::saveAndRefresh(bool persist) {
    // Invalidate the cached list after any lesson / holiday edit.
    todayList_->setProperty("renderedDate",QVariant());
    QString error;
    if(persist && canSaveSchedule_ && !store_.save(dataPath_,&error))
        QMessageBox::warning(this,tt::translate("保存失败"),error);
    if(!canSaveSchedule_) showMessage(tt::translate("原始课表读取失败"));
    updateDashboard();
    updateWeek();
    updateCourseTable();
    updateChanges();
    if(onScheduleChanged_) onScheduleChanged_();
}
void MainWindow::showMessage(const QString& text) {
    if(!canSaveSchedule_)
        statusBar()->showMessage(text+tt::translate(" · 课表自动保存已暂停，修改仅在内存中，可导出备份。"));
    else
        statusBar()->showMessage(text,7000);
}

void MainWindow::editCourse(int index) {
    if(index>=store_.courses.size()) return;
    QDialog dialog(this);
    dialog.setWindowTitle(index<0?tt::translate("添加课程"):tt::translate("编辑课程"));
    dialog.setMinimumWidth(440);
    auto* box=new QVBoxLayout(&dialog);
    auto* form=new QFormLayout;form->setSpacing(12);
    Course course=index>=0?store_.courses.at(index):Course{};
    auto* name=new QLineEdit(course.name);
    name->setPlaceholderText(tt::translate("课程或活动名称"));
    auto* weekday=new QComboBox;
    weekday->addItems(dayNames());
    weekday->setCurrentIndex(course.weekday-1);
    auto* start=new QTimeEdit(course.start.isValid()?course.start:QTime(8,0));
    start->setDisplayFormat("HH:mm");
    auto* end=new QTimeEdit(course.end.isValid()?course.end:QTime(9,40));
    end->setDisplayFormat("HH:mm");
    auto* weeks=new QLineEdit(course.weeks.isEmpty()?tt::translate("全部"):
        ScheduleStore::formatWeeks(course.weeks));
    auto* teacher=new QLineEdit(course.teacher);
    auto* room=new QLineEdit(course.room);
    auto* color=new QComboBox;
    const QStringList colors={"#6C6AF6","#1DAF9C","#F18E64","#3F91DB","#E369A9","#A06BD6","#DAA237"};
    for(const auto& c:colors) color->addItem(c,c);
    int selected=colors.indexOf(course.color);
    color->setCurrentIndex(selected<0?0:selected);
    form->addRow(tt::translate("课程名称"),name);
    form->addRow(tt::translate("星期"),weekday);
    form->addRow(tt::translate("上课时间"),start);
    form->addRow(tt::translate("下课时间"),end);
    form->addRow(tt::translate("教学周数"),weeks);
    form->addRow(tt::translate("授课教师（可选）"),teacher);
    form->addRow(tt::translate("地点（可选）"),room);
    form->addRow(tt::translate("卡片配色"),color);
    box->addLayout(form);
    box->addWidget(label(tt::translate("可用「1-16单」「2-18双」或「1-8,10,12」设置持续周。"),"muted"));
    auto* actions=new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel);
    actions->button(QDialogButtonBox::Save)->setText(tt::translate("保存"));
    actions->button(QDialogButtonBox::Cancel)->setText(tt::translate("取消"));
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
            QMessageBox::warning(&dialog,tt::translate("课程信息不完整"),error);
            return;
        }
        if(index<0) store_.courses<<edited;
        else store_.courses[index]=edited;
        saveAndRefresh();
        showMessage(canSaveSchedule_?tt::translate("课程已保存"):tt::translate("课程已更新"));
        dialog.accept();
    });
    dialog.exec();
}

void MainWindow::removeCourse() {
    const int row=selectedCourseIndex();
    if(row<0 || row>=store_.courses.size()) return;
    if(QMessageBox::question(this,tt::translate("删除课程"),
        tt::translate("确认删除「%1」？").arg(store_.courses[row].name))!=QMessageBox::Yes) return;
    store_.courses.removeAt(row);
    saveAndRefresh();
}
void MainWindow::duplicateCourse() {
    const int row=selectedCourseIndex();
    if(row<0 || row>=store_.courses.size()) return;
    Course c=store_.courses.at(row);
    c.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    c.name+=tt::translate("（副本）");
    store_.courses<<c;
    saveAndRefresh();
    for(int i=0;i<courseTable_->rowCount();++i)
        if(courseTable_->item(i,0)->data(Qt::UserRole).toInt()==store_.courses.size()-1) courseTable_->selectRow(i);
}
void MainWindow::addDayChange() {
    DayChange change;
    change.date=changeDate_->date();
    change.holiday=holiday_->isChecked();
    change.sourceDate=sourceDate_->date();
    for(int i=0;i<store_.changes.size();++i)
        if(store_.changes[i].date==change.date) {
            store_.changes[i]=change;
            saveAndRefresh();showMessage(tt::translate("已更新特殊日期"));
            return;
        }
    store_.changes<<change;
    std::sort(store_.changes.begin(),store_.changes.end(),
        [](const DayChange&a,const DayChange&b){return a.date<b.date;});
    saveAndRefresh();showMessage(canSaveSchedule_?tt::translate("日期调整已保存"):tt::translate("日期调整已更新"));
}
void MainWindow::removeDayChange() {
    const int row=exceptionList_->currentRow();
    if(row<0 || row>=store_.changes.size()) return;
    store_.changes.removeAt(row);
    saveAndRefresh();
}
void MainWindow::importSchedule() {
    const QString path=QFileDialog::getOpenFileName(this,tt::translate("导入课表"),
        QString(),tt::translate("课表文件 (*.json *.csv);;JSON (*.json);;CSV (*.csv)"));
    if(path.isEmpty()) return;
    QMessageBox choice(QMessageBox::Question,tt::translate("确认导入"),
        tt::translate("请选择导入方式。追加会保留已有课程、每日附加信息和调休，同内容课程会自动去重；相同日期的调休以现有安排为准。替换会清空当前课表，建议先导出备份。"),
        QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,this);
    choice.button(QMessageBox::Yes)->setText(tt::translate("替换当前课表"));
    choice.button(QMessageBox::No)->setText(tt::translate("追加合并"));
    choice.button(QMessageBox::Cancel)->setText(tt::translate("取消"));
    choice.setDefaultButton(QMessageBox::Cancel);
    const int answer=choice.exec();
    if(answer!=QMessageBox::Yes&&answer!=QMessageBox::No) return;
    if(answer==QMessageBox::No&&!canSaveSchedule_) {
        QMessageBox::warning(this,tt::translate("导入失败"),tt::translate("原课表读取失败时不能追加覆盖。请先另存为新文件，或明确选择替换。"));
        return;
    }
    QString error;
    ScheduleStore imported=store_;
    if(!imported.importFile(path,answer==QMessageBox::No?ScheduleStore::ImportMode::Append:ScheduleStore::ImportMode::Replace,&error) ||
       !imported.save(dataPath_,&error)) {
        QMessageBox::warning(this,tt::translate("导入失败"),error);
        return;
    }
    // The user confirmed replacement and the imported data passed validation.
    canSaveSchedule_=true;
    store_=imported;
    shownWeek_=std::max(1,store_.weekNumber(QDate::currentDate()));
    fillSettings();saveAndRefresh(false);
    showMessage(tt::translate("导入成功：%1 门课程").arg(store_.courses.size()));
}

bool MainWindow::switchSchedulePath(const QString& path,QString* error) {
    if(path.trimmed().isEmpty()) { if(error) *error=tt::translate("请选择课表文件"); return false; }
    ScheduleStore loaded;
    if(!loaded.load(path,error)) return false;
    // Commit path, settings and model only after the complete target validates.
    store_=loaded; dataPath_=absolutePath(path); canSaveSchedule_=true;
    QSettings().setValue("schedule/path",dataPath_);
    shownWeek_=std::max(1,store_.weekNumber(QDate::currentDate()));
    fillSettings(); saveAndRefresh(false);
    return true;
}
bool MainWindow::saveScheduleAs(const QString& path,QString* error) {
    if(path.trimmed().isEmpty()) { if(error) *error=tt::translate("请选择保存路径"); return false; }
    if(!canSaveSchedule_&&samePath(path,dataPath_)) {
        if(error) *error=tt::translate("原课表已受保护，请选择另一个文件路径；明确导入有效课表才能替换原始文件。");
        return false;
    }
    if(!store_.save(path,error)) return false;
    dataPath_=absolutePath(path); canSaveSchedule_=true;
    QSettings().setValue("schedule/path",dataPath_);
    fillPathSettings(); saveAndRefresh(false);
    return true;
}
bool MainWindow::convertImportToJson(const QString& input,const QString& output,QString* error) const {
    if(samePath(output,dataPath_)||samePath(input,output)) {
        if(error) *error=tt::translate("转换目标必须是独立文件，请选择不同于当前课表及导入源的路径。");
        return false;
    }
    ScheduleStore imported=store_;
    return imported.importFile(input,error)&&imported.save(output,error);
}
void MainWindow::fillPathSettings() {
    if(!schedulePathLabel_) return;
    schedulePathLabel_->setText(tt::translate("当前课表：%1").arg(dataPath_));
    const QString selected=profiles_->currentText();
    profiles_->clear();
    const auto records=QSettings().value("schedule/profiles").toMap();
    profiles_->addItem(tt::translate("选择已命名的学期档案"),QString());
    int matching=0;
    for(auto it=records.cbegin();it!=records.cend();++it) {
        profiles_->addItem(it.key(),it.value().toString());
        if(samePath(it.value().toString(),dataPath_)) matching=profiles_->count()-1;
    }
    const int oldSelection=profiles_->findText(selected);
    profiles_->setCurrentIndex(matching?matching:std::max(0,oldSelection));
}
void MainWindow::chooseSchedulePath() {
    const QString path=QFileDialog::getOpenFileName(this,tt::translate("选择持久课表文件"),
        QFileInfo(dataPath_).absolutePath(),QStringLiteral("TimeTable Next JSON (*.json)"));
    if(path.isEmpty()) return;
    QString error;
    if(!switchSchedulePath(path,&error)) QMessageBox::warning(this,tt::translate("切换课表失败"),error);
    else showMessage(tt::translate("已切换课表：%1").arg(dataPath_));
}
void MainWindow::chooseSaveScheduleAs() {
    QString path=QFileDialog::getSaveFileName(this,tt::translate("另存为持久课表"),
        QFileInfo(dataPath_).absolutePath()+"/NewSemester.json",QStringLiteral("TimeTable Next JSON (*.json)"));
    if(path.isEmpty()) return;
    if(!path.endsWith(".json",Qt::CaseInsensitive)) path+=".json";
    QString error;
    if(!saveScheduleAs(path,&error)) QMessageBox::warning(this,tt::translate("另存为失败"),error);
    else showMessage(tt::translate("已另存为并切换：%1").arg(dataPath_));
}
void MainWindow::registerProfile() {
    if(!canSaveSchedule_) {
        QMessageBox::warning(this,tt::translate("档案尚未保存"),tt::translate("请先另存为有效课表，再命名学期档案。")); return;
    }
    bool ok=false;
    const QString name=QInputDialog::getText(this,tt::translate("命名学期档案"),
        tt::translate("档案名称（例如 2026 秋季学期）"),QLineEdit::Normal,QString(),&ok).trimmed();
    if(!ok||name.isEmpty()) return;
    auto records=QSettings().value("schedule/profiles").toMap();
    if(records.contains(name)&&records.value(name).toString()!=dataPath_ &&
       QMessageBox::question(this,tt::translate("替换档案记录"),tt::translate("该名称已使用，是否改为关联当前课表？"))!=QMessageBox::Yes) return;
    records[name]=dataPath_; QSettings().setValue("schedule/profiles",records);
    fillPathSettings(); showMessage(tt::translate("学期档案已记录"));
}
void MainWindow::convertImport() {
    const QString input=QFileDialog::getOpenFileName(this,tt::translate("选择待转换课表"),QString(),
        tt::translate("课表文件 (*.json *.csv)"));
    if(input.isEmpty()) return;
    QString output=QFileDialog::getSaveFileName(this,tt::translate("保存转换后的独立课表"),
        QFileInfo(input).absolutePath()+"/"+QFileInfo(input).completeBaseName()+"-converted.json",
        QStringLiteral("TimeTable Next JSON (*.json)"));
    if(output.isEmpty()) return;
    if(!output.endsWith(".json",Qt::CaseInsensitive)) output+=".json";
    QString error;
    if(!convertImportToJson(input,output,&error)) QMessageBox::warning(this,tt::translate("转换失败"),error);
    else showMessage(tt::translate("已生成独立课表，当前课表保持不变：%1").arg(output));
}
void MainWindow::checkReminders() {
    if(!remind_) return;
    const auto now=QDateTime::currentDateTime();
    bool changed=false;
    if(QSettings().value("reminders/notifiedDate").toDate()!=now.date()) {
        notifiedReminders_.clear();
        QSettings().setValue("reminders/notifiedDate",now.date());
        changed=true;
    }
    for(const auto& reminder:store_.remindersDue(now,minutesBefore_)) {
        const QString key=dataPath_+"|"+reminder.key();
        if(notifiedReminders_.contains(key)) continue;
        notifiedReminders_.insert(key);
        changed=true;
        const QString text=tt::translate("%1 · %2 上课%3").arg(reminder.course.name,
            reminder.startsAt.toString("HH:mm"),reminder.course.room.isEmpty()?QString():QStringLiteral(" · ")+reminder.course.room);
        if(tray_&&tray_->isVisible()&&QSystemTrayIcon::supportsMessages())
            tray_->showMessage(tt::translate("课程提醒"),text,QSystemTrayIcon::Information,10000);
        else showMessage(tt::translate("课程提醒：%1").arg(text));
    }
    if(changed) {
        QStringList keys; for(const auto& key:notifiedReminders_) keys<<key;
        QSettings().setValue("reminders/notified",keys);
    }
}
void MainWindow::exportSchedule() {
    QString filter;
    QString path=QFileDialog::getSaveFileName(this,tt::translate("导出课表"),
        QStringLiteral("TimeTableNext-Backup.json"),
        tt::translate("TimeTable Next JSON (*.json);;CSV 表格 (*.csv);;日历文件 (*.ics);;小爱结构兼容 JSON (*.xiaomi.json)"),
        &filter);
    if(path.isEmpty()) return;
    QString error;bool ok=false;
    if(filter.startsWith(QStringLiteral("CSV"))) {
        if(!path.endsWith(".csv",Qt::CaseInsensitive)) path+=".csv";
        ok=store_.exportCsv(path,&error);
    } else if(filter.startsWith(tt::translate("日历"))) {
        if(!path.endsWith(".ics",Qt::CaseInsensitive)) path+=".ics";
        ok=store_.exportIcs(path,&error);
    } else if(filter.startsWith(tt::translate("小爱"))) {
        if(!path.endsWith(".json",Qt::CaseInsensitive)) path+=".xiaomi.json";
        ok=store_.exportXiaoAiJson(path,&error);
    } else {
        if(!path.endsWith(".json",Qt::CaseInsensitive)) path+=".json";
        ok=store_.save(path,&error);
    }
    if(!ok) QMessageBox::warning(this,tt::translate("导出失败"),error);
    else showMessage(tt::translate("已导出：%1").arg(path));
}
void MainWindow::saveSettings() {
    const int newTotal=weeksSpin_->value();
    for(const auto& c:store_.courses) for(int w:c.weeks) if(w>newTotal) {
        QMessageBox::warning(this,tt::translate("周数冲突"),
            tt::translate("课程「%1」使用第 %2 周，无法将学期缩短为 %3 周。")
                .arg(c.name).arg(w).arg(newTotal));
        return;
    }
    QMap<int,QPair<QTime,QTime>> newPeriods;
    for(int row=0;row<periodsTable_->rowCount();++row) {
        const int section=periodsTable_->item(row,0)->text().toInt();
        const QTime start=QTime::fromString(periodsTable_->item(row,1)->text().trimmed(),"HH:mm");
        const QTime end=QTime::fromString(periodsTable_->item(row,2)->text().trimmed(),"HH:mm");
        if(!start.isValid() || !end.isValid() || start>=end) {
            QMessageBox::warning(this,tt::translate("时间格式错误"),
                tt::translate("第 %1 节时间无效，格式应为 HH:mm 且结束晚于开始").arg(section));
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
    remind_=remindersEnabled_->isChecked(); minutesBefore_=reminderMinutes_->value();
    QSettings().setValue("reminders/enabled",remind_);
    QSettings().setValue("reminders/minutesBefore",minutesBefore_);
    const QString language=language_->currentData().toString();
    const bool languageChanged=QSettings().value("ui/language","zh_CN").toString()!=language;
    QSettings().setValue("ui/language",language);
    applyTheme();
    shownWeek_=std::clamp(shownWeek_,1,newTotal);
    saveAndRefresh();
    fillSettings();
    showMessage(languageChanged?tt::translate("设置已保存，界面语言将在重新启动后生效"):
        (canSaveSchedule_?tt::translate("设置已保存"):tt::translate("外观设置已保存")));
}
} // namespace tt
