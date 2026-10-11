#include "Translation.h"
#include "MainWindow.h"
#include "DesktopOverlays.h"
#include "ElementEditor.h"
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>

int main(int argc,char** argv) {
    QTemporaryDir isolated;
    if(!isolated.isValid())return 1;
    qputenv("XDG_DATA_HOME",(isolated.path()+"/data").toUtf8());
    qputenv("XDG_CONFIG_HOME",(isolated.path()+"/settings").toUtf8());
    QApplication app(argc,argv);
    app.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName("TimeTableNextTests");
    QCoreApplication::setApplicationName("translations-"+QFileInfo(isolated.path()).fileName());
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat,QSettings::UserScope,isolated.path());
    QSettings settings;
    settings.setValue("ui/language","en");
    tt::ScheduleStore schedule;
    const QString schedulePath=isolated.path()+"/schedule.json";
    if(!schedule.save(schedulePath))return 2;
    settings.setValue("schedule/path",schedulePath);
    settings.setValue("desktop/configurationPath",isolated.path()+"/layout.json");
    tt::AppTranslator translator;
    if(!translator.activate("en") ||
       tt::translate("保存更改")!="Save changes" ||
       tt::translate("导入数据")!="Import data" ||
       tt::translate("user-defined text")!="user-defined text")return 3;
    {
        tt::MainWindow window;
        bool save=false,import=false;
        for(auto* button:window.findChildren<QPushButton*>()) {
            save|=button->text()=="Save changes";
            import|=button->text()=="Import data";
        }
        if(!save||!import||window.windowTitle()!=tt::translate("TimeTable Next · 课表与日程"))return 4;
        window.openPage(1);
        bool monday=false;
        for(auto* label:window.findChildren<QLabel*>())monday|=label->text().startsWith(tt::translate("周一")+"\n");
        if(!monday)return 5; // Day labels are generated after translator activation.
        bool editorChecked=false;
        tt::ElementDefinition element; element.items={tt::TextElement{}};
        QList<tt::ElementDefinition> elements={element};
        QTimer::singleShot(0,[&] {
            auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());
            if(!dialog)return;
            auto* kind=dialog->findChild<QComboBox*>("editorKind");
            auto* undo=dialog->findChild<QPushButton*>("editorUndo");
            editorChecked=kind&&undo&&kind->itemText(0)==tt::translate("时间")&&
                undo->text()=="Undo"&&dialog->windowTitle()==tt::translate("信息元素布局编辑器");
            dialog->reject();
        });
        if(tt::ElementEditor::edit(&window,elements,schedule,QSize(400,150),false)||!editorChecked)return 6;
        tt::DesktopOverlays overlays(&window);
        overlays.openConfiguration();
        bool layoutChecked=false;
        for(auto* widget:QApplication::topLevelWidgets()) {
            if(auto* dialog=qobject_cast<QDialog*>(widget);
               dialog&&dialog->windowTitle()==tt::translate("桌面窗口管理 · TimeTable Next")) {
                for(auto* button:dialog->findChildren<QPushButton*>())
                    layoutChecked|=button->text()==tt::translate("备份完整布局");
                dialog->close();
            }
        }
        if(!layoutChecked)return 7;
    }
    if(translator.activate("unsupported"))return 8;
    if(!translator.activate("zh_CN") ||
       tt::translate("保存更改")!=QStringLiteral("保存更改"))return 9;
    qInfo()<<"Bundled English, actual window/editor labels and Chinese fallback passed";
    return 0;
}
