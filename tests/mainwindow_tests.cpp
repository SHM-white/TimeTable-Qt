#include "MainWindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryDir>
#include <QTableWidget>
#include <QTimer>
#include <QDebug>

using namespace tt;
namespace {
int failures=0;
void check(bool condition,const char* message) {
    if(!condition) { qCritical()<<"FAILED:"<<message; ++failures; }
}
QByteArray readFile(const QString& path) {
    QFile file(path);
    check(file.open(QIODevice::ReadOnly),"schedule remains readable");
    return file.readAll();
}
void writeFile(const QString& path,const QByteArray& bytes) {
    QFile file(path);
    check(file.open(QIODevice::WriteOnly),"test schedule writable");
    check(file.write(bytes)==bytes.size(),"complete test schedule written");
}
QPushButton* findButton(MainWindow& window,const QString& text) {
    for(auto* button:window.findChildren<QPushButton*>())
        if(button->text()==text) return button;
    check(false,"requested MainWindow action exists");
    return nullptr;
}

// Exercise the actual sidebar action, file picker and replacement confirmation.
// A bounded timer keeps modal warnings from hanging a failed regression test.
QStringList runFileAction(MainWindow& window,const QString& action,const QString& path,
                          int* confirmations=nullptr,QMessageBox::StandardButton importMode=QMessageBox::Yes) {
    QStringList warnings;
    bool selected=false;
    QElapsedTimer elapsed;
    elapsed.start();
    QTimer timer;
    QObject::connect(&timer,&QTimer::timeout,[&] {
        for(auto* widget:QApplication::topLevelWidgets()) {
            if(!widget->isVisible()) continue;
            if(auto* dialog=qobject_cast<QFileDialog*>(widget)) {
                if(elapsed.elapsed()>5000) {
                    check(false,"file dialog automation completed");
                    dialog->reject();
                } else {
                    if(!selected) {
                        selected=true;
                        dialog->setDirectory(QFileInfo(path).absolutePath());
                    }
                    // Existing-file dialogs populate their model asynchronously.
                    // selectFile alone can leave a visible dialog's selection empty;
                    // enter the filename as a user would and retry after model loads.
                    dialog->selectFile(QFileInfo(path).fileName());
                    if(auto* name=dialog->findChild<QLineEdit*>("fileNameEdit"))
                        name->setText(QFileInfo(path).fileName());
                    QMetaObject::invokeMethod(dialog,"accept",Qt::QueuedConnection);
                }
            } else if(auto* message=qobject_cast<QMessageBox*>(widget)) {
                if(message->windowTitle()==QStringLiteral("确认导入")) {
                    if(confirmations) ++*confirmations;
                    message->button(importMode)->click();
                } else if(message->windowTitle()==QStringLiteral("确认覆盖")) {
                    message->button(QMessageBox::Yes)->click();
                } else {
                    warnings<<message->text();
                    message->accept();
                }
            }
        }
    });
    timer.start(10);
    if(auto* button=findButton(window,action)) button->click();
    timer.stop();
    check(selected,"file picker was opened");
    return warnings;
}
void cancelFileAction(MainWindow& window,const QString& action) {
    bool cancelled=false;
    QTimer timer;
    QObject::connect(&timer,&QTimer::timeout,[&] {
        for(auto* widget:QApplication::topLevelWidgets())
            if(auto* dialog=qobject_cast<QFileDialog*>(widget);dialog&&dialog->isVisible()) {
                cancelled=true; dialog->reject();
            }
    });
    timer.start(10);
    if(auto* button=findButton(window,action)) button->click();
    timer.stop(); check(cancelled,"path file picker cancellation exercised");
}
} // namespace

int main(int argc,char** argv) {
    QTemporaryDir isolated;
    if(!isolated.isValid()) return 1;
    const QString dataDirectory=isolated.path()+"/data";
    const QString settingsDirectory=isolated.path()+"/settings";
    QDir().mkpath(dataDirectory);
    QDir().mkpath(settingsDirectory);
    qputenv("XDG_DATA_HOME",dataDirectory.toUtf8());
    qputenv("XDG_CONFIG_HOME",settingsDirectory.toUtf8());
    qputenv("APPDATA",dataDirectory.toUtf8());
    qputenv("LOCALAPPDATA",dataDirectory.toUtf8());
    QStandardPaths::setTestModeEnabled(true);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc,argv);
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName("TimeTableNextTests");
    const QString applicationName="mainwindow-tests-"+QFileInfo(isolated.path()).fileName();
    QCoreApplication::setApplicationName(applicationName);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,settingsDirectory);
    QSettings().clear();

    // Windows and macOS may ignore XDG paths. Use a unique application directory
    // in Qt's test location and refuse existing directories before writing files.
    const QString appDataDirectory=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    check(!appDataDirectory.isEmpty()&&QFileInfo(appDataDirectory).fileName()==applicationName,
          "MainWindow data directory belongs to this unique test application");
    check(!QFileInfo::exists(appDataDirectory),"test application data directory is exclusive");
    if(failures) return 1;
    struct AppDataCleanup {
        QString directory;
        ~AppDataCleanup() { QDir(directory).removeRecursively(); }
    } cleanup{appDataDirectory};
    const QString dataPath=ScheduleStore::defaultPath();
    check(QFileInfo(dataPath).absolutePath()==QDir(appDataDirectory).absolutePath(),
          "schedule remains inside the exclusive test data directory");
    if(failures) return 1;
    const QByteArray damaged=QByteArrayLiteral("{ damaged JSON\n  original bytes must survive\n");
    writeFile(dataPath,damaged);

    QStringList startupWarnings;
    QTimer closeWarnings;
    QObject::connect(&closeWarnings,&QTimer::timeout,[&] {
        for(auto* widget:QApplication::topLevelWidgets())
            if(auto* message=qobject_cast<QMessageBox*>(widget);message&&message->isVisible()) {
                startupWarnings<<message->text();
                message->accept();
            }
    });
    closeWarnings.start(10);
    {
        MainWindow window;
        closeWarnings.stop();
        check(startupWarnings.size()==1,"failed startup reports a warning");
        check(startupWarnings.join(" ").contains(QStringLiteral("自动保存已暂停")),
              "warning explains persistent save protection");
        check(readFile(dataPath)==damaged,"startup preserves damaged schedule bytes");
        QString protectedError;
        check(!window.saveScheduleAs(dataPath,&protectedError)&&readFile(dataPath)==damaged,
              "save-as cannot bypass failed-load protection for same original file");

        window.setDayInfos(1,{QStringLiteral("仅在内存中修改")});
        check(window.schedule().dayInfos.value(1)==QStringList{QStringLiteral("仅在内存中修改")},
              "daily message edit is available in memory");
        check(readFile(dataPath)==damaged,"daily message edit preserves damaged schedule bytes");
        check(window.statusBar()->currentMessage().contains(QStringLiteral("自动保存已暂停")),
              "daily message edit keeps save protection visible");

        window.openPage(4);
        for(auto* box:window.findChildren<QCheckBox*>())
            if(box->text()==QStringLiteral("使用深色主题")) box->setChecked(true);
        if(auto* save=findButton(window,QStringLiteral("保存更改"))) save->click();
        check(QSettings().value("dark").toBool(),"appearance preference remains saveable");
        check(readFile(dataPath)==damaged,"appearance save preserves damaged schedule bytes");

        const QString backup=isolated.path()+"/backup.json";
        check(runFileAction(window,QStringLiteral("导出与备份"),backup).isEmpty(),
              "export remains available under protection");
        ScheduleStore exported;
        QString error;
        check(exported.load(backup,&error),"protected in-memory schedule can be exported");
        check(exported.dayInfos.value(1)==window.schedule().dayInfos.value(1),
              "export contains current in-memory messages");
        check(readFile(dataPath)==damaged,"export preserves damaged original");

        const QString invalid=isolated.path()+"/invalid.json";
        writeFile(invalid,damaged);
        int confirmations=0;
        check(!runFileAction(window,QStringLiteral("导入数据"),invalid,&confirmations).isEmpty(),
              "invalid explicit import reports failure");
        window.setDayInfos(2,{QStringLiteral("无效导入后仍保护")});
        check(readFile(dataPath)==damaged,"failed import does not lift save protection");

        ScheduleStore replacement;
        replacement.dayInfos[3]=QStringList{QStringLiteral("有效导入")};
        const QString valid=isolated.path()+"/valid.json";
        check(replacement.save(valid,&error),"valid replacement fixture saved");
        check(runFileAction(window,QStringLiteral("导入数据"),valid,&confirmations).isEmpty(),
              "valid explicit import succeeds");
        check(confirmations==2,"each import requires explicit replacement confirmation");
        ScheduleStore saved;
        check(saved.load(dataPath,&error),"confirmed valid import replaces damaged original");
        check(saved.dayInfos.value(3)==replacement.dayInfos.value(3),"imported data was persisted");
        window.setDayInfos(4,{QStringLiteral("导入后恢复保存")});
        check(saved.load(dataPath,&error)&&saved.dayInfos.value(4)==window.schedule().dayInfos.value(4),
              "successful import restores subsequent automatic saves");
    }

    const int warningCount=startupWarnings.size();
    closeWarnings.start(10);
    {
        MainWindow window;
        closeWarnings.stop();
        check(startupWarnings.size()==warningCount,"normal startup reports no load warning");
        QString error;
        ScheduleStore saved;
        check(window.schedule().dayInfos.value(4)==QStringList{QStringLiteral("导入后恢复保存")},
              "normal startup loads previously saved data");
        window.setDayInfos(5,{QStringLiteral("正常启动后修改")});
        check(saved.load(dataPath,&error)&&saved.dayInfos.value(5)==window.schedule().dayInfos.value(5),
              "normal startup and edits still persist");
    }

    QString activeCustomPath;
    {
        MainWindow window;
        window.openPage(4);
        QString error;
        const QString originalPath=window.dataPath();
        const QByteArray originalBytes=readFile(originalPath);
        const QJsonObject originalModel=window.schedule().toJson();
        const QString invalid=isolated.path()+"/invalid.json";
        check(!window.switchSchedulePath(invalid,&error)&&window.dataPath()==originalPath&&window.schedule().toJson()==originalModel,
              "failed path switch preserves active model and path");
        check(!window.switchSchedulePath(isolated.path()+"/missing.json",&error)&&window.dataPath()==originalPath,
              "missing path switch preserves active path");
        cancelFileAction(window,QStringLiteral("选择课表文件"));
        check(window.dataPath()==originalPath&&window.schedule().toJson()==originalModel,"cancelled picker preserves path and model");

        ScheduleStore custom;
        custom.courses.clear(); custom.semesterMonday=QDate(2026,9,7);
        Course a; a.id="alice"; a.name="Math"; a.teacher="Alice"; a.room="Lab";
        a.weekday=2; a.start=QTime(8,0); a.end=QTime(9,0); a.weeks={2,7};
        Course b=a; b.id="bob"; b.name="Science"; b.teacher="Bob"; b.room="Room"; b.weekday=3;
        custom.courses<<a<<b;
        custom.dayInfos[6]=QStringList{QStringLiteral("独立学期")};
        const QString customPath=isolated.path()+"/custom.json";
        check(custom.save(customPath,&error),"custom path fixture saved");
        const QByteArray beforeSwitch=readFile(customPath);
        check(window.switchSchedulePath(customPath,&error)&&window.dataPath()==customPath,"valid custom path switch commits");
        check(readFile(customPath)==beforeSwitch&&readFile(originalPath)==originalBytes,"switch is read-only for both schedule files");
        window.setDayInfos(7,{QStringLiteral("自定义路径保存")});
        ScheduleStore persisted;
        check(persisted.load(customPath,&error)&&persisted.dayInfos.value(7)==window.schedule().dayInfos.value(7),"edits persist to active custom path");
        check(readFile(originalPath)==originalBytes,"custom edits leave previous schedule intact");
        check(!window.saveScheduleAs(isolated.path()+"/missing-parent/new.json",&error)&&window.dataPath()==customPath,
              "failed save-as leaves active path unchanged");
        activeCustomPath=isolated.path()+"/saved-as.json";
        check(window.saveScheduleAs(activeCustomPath,&error)&&QSettings().value("schedule/path").toString()==activeCustomPath,
              "atomic save-as switches and persists target path");
        check(persisted.load(activeCustomPath,&error)&&persisted.toJson()==window.schedule().toJson(),"save-as contains current model");

        auto* search=window.findChild<QLineEdit*>("courseSearch");
        auto* table=window.findChild<QTableWidget*>("courseTable");
        check(search&&table,"course search widgets exist");
        if(search&&table) {
            search->setText(QStringLiteral("Alice Lab 周二 7"));
            check(table->rowCount()==1&&table->item(0,0)->text()=="Math","multi-condition search matches teacher room weekday and active week");
            search->setText("Bob"); table->selectRow(0);
            if(auto* copy=findButton(window,QStringLiteral("复制"))) copy->click();
            check(window.schedule().courses.size()==3&&window.schedule().courses.last().teacher=="Bob"&&
                  window.schedule().courses.first().name=="Math","filtered selection resolves correct underlying course");
            search->clear();
        }

        const QString importSource=isolated.path()+"/legacy-conversion.json";
        writeFile(importSource,QByteArrayLiteral("{\"Mon\":{\"Lessons\":[[\"Converted\",\"0800\",\"0900\"]],\"Infos\":[\"Legacy note\"]}}"));
        const QString converted=isolated.path()+"/converted.json";
        const QJsonObject beforeConvert=window.schedule().toJson();
        const QByteArray activeBeforeConvert=readFile(activeCustomPath);
        check(window.convertImportToJson(importSource,converted,&error),"legacy file converts to independent JSON");
        check(persisted.load(converted,&error)&&persisted.courses.size()==1&&persisted.courses.first().name=="Converted","converted JSON loads as native schedule");
        check(window.schedule().toJson()==beforeConvert&&window.dataPath()==activeCustomPath&&readFile(activeCustomPath)==activeBeforeConvert,
              "conversion preserves active model and persistence file");
        check(!window.convertImportToJson(importSource,activeCustomPath,&error)&&readFile(activeCustomPath)==activeBeforeConvert,
              "conversion rejects active persistence target");

        check(runFileAction(window,QStringLiteral("导入数据"),importSource,nullptr,QMessageBox::No).isEmpty(),"append UI action succeeds");
        check(window.schedule().courses.size()==4&&window.schedule().dayInfos.value(7).contains(QStringLiteral("自定义路径保存")),
              "append UI retains current courses and Infos");
        auto* reminderToggle=window.findChild<QCheckBox*>("remindersEnabled");
        auto* reminderMinutes=window.findChild<QSpinBox*>("reminderMinutes");
        check(reminderToggle&&!reminderToggle->isChecked(),"course reminders default disabled");
        if(reminderToggle&&reminderMinutes) { reminderToggle->setChecked(true); reminderMinutes->setValue(5); }
        if(auto* save=findButton(window,QStringLiteral("保存更改"))) save->click();
        check(QSettings().value("reminders/enabled").toBool()&&QSettings().value("reminders/minutesBefore").toInt()==5,
              "reminder preferences persist after explicit settings save");

        QTimer profileTimer;
        QObject::connect(&profileTimer,&QTimer::timeout,[&]{
            for(auto* widget:QApplication::topLevelWidgets())
                if(auto* dialog=qobject_cast<QInputDialog*>(widget);dialog&&dialog->isVisible()) {
                    dialog->setTextValue("2026 Fall"); dialog->accept();
                }
        });
        profileTimer.start(10);
        if(auto* record=findButton(window,QStringLiteral("命名当前档案"))) record->click();
        profileTimer.stop();
        check(QSettings().value("schedule/profiles").toMap().value("2026 Fall").toString()==activeCustomPath,
              "named semester profile records persistent path");
    }
    {
        MainWindow window;
        check(window.dataPath()==activeCustomPath&&window.schedule().courses.size()==4,
              "restart loads selected custom semester path");
        check(window.findChild<QCheckBox*>("remindersEnabled")->isChecked(),"restart restores reminder preferences");
    }
    {
        const QString malformedPath=isolated.path()+"/malformed-native.json";
        const QByteArray malformed=QByteArrayLiteral("{\"schema\":\"timetable-next/v1\",\"semesterStart\":\"2026-09-07\",\"totalWeeks\":16,\"courses\":\"damaged\"}");
        writeFile(malformedPath,malformed);
        QSettings().setValue("schedule/path",malformedPath);
        const int warningsBefore=startupWarnings.size();
        closeWarnings.start(10);
        MainWindow window;
        closeWarnings.stop();
        check(startupWarnings.size()==warningsBefore+1,"malformed native member type triggers protected startup");
        window.setDayInfos(1,{QStringLiteral("内存变更")});
        check(readFile(malformedPath)==malformed,"native type damage cannot silently clear original schedule on save");
        const QString protectedBackup=isolated.path()+"/protected-new-path.json";
        QString error;
        check(window.saveScheduleAs(protectedBackup,&error)&&readFile(malformedPath)==malformed,
              "protected in-memory schedule can save to new path while original stays intact");
    }
    if(!failures) qInfo()<<"All MainWindow protection tests passed";
    return failures?1:0;
}
