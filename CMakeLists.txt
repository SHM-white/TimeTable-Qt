cmake_minimum_required(VERSION 3.21)
project(TimeTableNext VERSION 4.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# The legacy Qt5 toolchain does not add /utf-8 automatically. Our Chinese
# JSON fixtures and UI source files must compile as UTF-8 on MSVC.
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_AUTOMOC ON)

option(TIMETABLE_FORCE_QT5 "Build with Qt 5.15, for Windows 7 legacy compatibility" OFF)
if(TIMETABLE_FORCE_QT5)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)
    set(TT_QT_MAJOR 5)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
    set(TT_QT_MAJOR ${QT_VERSION_MAJOR})
    find_package(Qt${TT_QT_MAJOR} REQUIRED COMPONENTS Core Widgets)
endif()

if(TT_QT_MAJOR EQUAL 6)
    qt_standard_project_setup()
endif()

add_library(timetable_core STATIC modern/ScheduleStore.h modern/ScheduleStore.cpp)
target_include_directories(timetable_core PUBLIC modern)
target_link_libraries(timetable_core PUBLIC Qt${TT_QT_MAJOR}::Core)

set(TT_DESKTOP_SOURCES
    modern/main.cpp
    modern/MainWindow.h modern/MainWindow.cpp
    modern/DesktopOverlays.h modern/DesktopOverlays.cpp
    modern/QtCompat.h
)
if(TT_QT_MAJOR EQUAL 6)
    qt_add_executable(TimeTableNext WIN32 ${TT_DESKTOP_SOURCES})
else()
    add_executable(TimeTableNext WIN32 ${TT_DESKTOP_SOURCES})
endif()
target_link_libraries(TimeTableNext PRIVATE timetable_core Qt${TT_QT_MAJOR}::Widgets)
target_compile_definitions(TimeTableNext PRIVATE TIMETABLE_QT_MAJOR=${TT_QT_MAJOR})
set_target_properties(TimeTableNext PROPERTIES
    OUTPUT_NAME "TimeTableNext"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS TimeTableNext RUNTIME DESTINATION .)

include(CTest)
if(BUILD_TESTING)
    add_executable(schedule_tests tests/schedule_tests.cpp)
    target_link_libraries(schedule_tests PRIVATE timetable_core Qt${TT_QT_MAJOR}::Core)
    add_test(NAME schedule_tests COMMAND schedule_tests)
    message(STATUS "Building TimeTableNext with Qt ${TT_QT_MAJOR}")
endif()
