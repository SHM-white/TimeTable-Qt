QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += \
    c++20\
    c++2a

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BasicWindow.cpp \
    CSVEditor.cpp \
    Lesson.cpp \
    TextFormat.cpp \
    Timetable.cpp \
    UIElementBase.cpp \
    WindowItem.cpp \
    WindowSettings.cpp \
    about.cpp \
    changeconfigpath.cpp \
    importfromfile.cpp \
    include.cpp \
    main.cpp \
    TimeTableQt.cpp \
    mainwindow.cpp \
    settings.cpp \
    showalllessons.cpp \
    showmoreinfo.cpp \
    todayalllessons.cpp

HEADERS += \
    BasicWindow.h \
    CSVEditor.h \
    Lesson.h \
    TextFormat.h \
    TimeTable-Qt.aps \
    Timetable.h \
    ToWString.h \
    UIElementBase.h \
    WindowItem.h \
    WindowSettings.h \
    about.h \
    changeconfigpath.h \
    framework.h \
    TimeTableQt.h \
    importfromfile.h \
    include.h \
    mainwindow.h \
    resource.h \
    resource1.h \
    settings.h \
    showalllessons.h \
    showmoreinfo.h \
    targetver.h \
    todayalllessons.h

FORMS += \
    TimeTableQt.ui \
    about.ui \
    changeconfigpath.ui \
    importfromfile.ui \
    mainwindow.ui \
    settings.ui \
    showalllessons.ui \
    showmoreinfo.ui \
    todayalllessons.ui

TRANSLATIONS += \
    TimeTable-Qt_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore \
    1.png \
    123.ico \
    AcrylicMaterial.dll \
    BackGround.png \
    Lessons.json \
    Lessons1.json \
    README.md \
    TimeTable-Qt.pri \
    TimeTable-Qt.pro.user \
    TimeTable-Qt.rc \
    TimeTable-Qt.vcxproj \
    TimeTable-Qt.vcxproj.filters \
    TimeTable-Qt.vcxproj.user \
    TimeTable.ico \
    TimeTable2.0.ico \
    TimeTable2.0.rc \
    alpha_test.png \
    config.json \
    small.ico



INCLUDEPATH += ./include
DEPENDPATH += ./include

RESOURCES += \
    TimeTableQt.qrc \
    TimeTableQt.qrc

RC_FILE += TimeTable-Qt.rc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/jsoncpp/lib/Release/ -ljsoncpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/jsoncpp/lib/Debug/ -ljsoncpp

INCLUDEPATH += $$PWD/jsoncpp/include
DEPENDPATH += $$PWD/jsoncpp/include

INCLUDEPATH += $$PWD/jsoncpp/include/jsoncpp
DEPENDPATH += $$PWD/jsoncpp/include/jsoncpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/hv/lib/Release/ -lhv
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/hv/lib/Debug/ -lhv

INCLUDEPATH += $$PWD/hv/include/hv
DEPENDPATH += $$PWD/hv/include/hv
