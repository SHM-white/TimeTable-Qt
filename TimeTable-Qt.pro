QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CSVEditor.cpp \
    TextFormat.cpp \
    Timetable.cpp \
    WindowSettings.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CSVEditor.h \
    Lesson.h \
    TextFormat.h \
    Timetable.h \
    WindowSettings.h \
    framework.h \
    mainwindow.h \
    resource.h \
    targetver.h

FORMS += \
    mainwindow.ui

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
    Lessons.json \
    README.md \
    TimeTable-Qt.pro.user \
    TimeTable2.0.ico \
    TimeTable2.0.rc \
    config.json \
    small.ico

win32:CONFIG(release, debug|release): LIBS += -LC:/libraries/jsoncpp/lib/release/ -ljsoncpp
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/libraries/jsoncpp/lib/debug/ -ljsoncpp

INCLUDEPATH += C:/libraries/jsoncpp/include
DEPENDPATH += C:/libraries/jsoncpp/include
