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
    EditUIElements.cpp \
    EmptyUIElement.cpp \
    Lesson.cpp \
    MultiItemAllDisplayUIElementBase.cpp \
    MultiItemInOrderUIElementBase.cpp \
    MultiTextItem.cpp \
    Settings_New.cpp \
    SingleItemUIElementBase.cpp \
    SingleTextItem.cpp \
    SubWindow.cpp \
    Timetable.cpp \
    TodayAllLessons.cpp \
    UIElementBase.cpp \
    importfromfile.cpp \
    include.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    BasicWindow.h \
    CSVEditor.h \
    EditUIElements.h \
    EmptyUIElement.h \
    Lesson.h \
    MultiItemAllDisplayUIElementBase.h \
    MultiItemInOrderUIElementBase.h \
    MultiTextItem.h \
    Settings_New.h \
    SingleItemUIElementBase.h \
    SingleTextItem.h \
    SubWindow.h \
    TimeTable-Qt.aps \
    Timetable.h \
    TodayAllLessons.h \
    UIElementBase.h \
    importfromfile.h \
    include.h \
    mainwindow.h

FORMS += \
    EditUIElements.ui \
    Settings_New.ui \
    SubWindow.ui \
    importfromfile.ui \
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

LIBS+= -luser32
