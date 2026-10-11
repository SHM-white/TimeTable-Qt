cmake_minimum_required(VERSION 3.21)
project(TimeTableNext VERSION 4.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# The legacy Qt5 toolchain does not add /utf-8 automatically. Our Chinese
# JSON fixtures and UI source files must compile as UTF-8 on MSVC.
if(MSVC)
    add_compile_options(/utf-8)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

option(TIMETABLE_FORCE_QT5 "Build with Qt 5.15, for Windows 7 legacy compatibility" OFF)
if(TIMETABLE_FORCE_QT5)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Network)
    set(TT_QT_MAJOR 5)
else()
    find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
    set(TT_QT_MAJOR ${QT_VERSION_MAJOR})
    find_package(Qt${TT_QT_MAJOR} REQUIRED COMPONENTS Core Widgets Network)
endif()

if(TT_QT_MAJOR EQUAL 6)
    qt_standard_project_setup()
endif()

add_library(timetable_core STATIC modern/ScheduleStore.h modern/ScheduleStore.cpp)
target_include_directories(timetable_core PUBLIC modern)
target_link_libraries(timetable_core PUBLIC Qt${TT_QT_MAJOR}::Core)

add_library(timetable_elements STATIC
    modern/ElementLayout.h modern/ElementLayout.cpp
    modern/WeatherClient.h modern/WeatherClient.cpp
    modern/ContentClient.h modern/ContentClient.cpp)
target_link_libraries(timetable_elements PUBLIC timetable_core Qt${TT_QT_MAJOR}::Gui Qt${TT_QT_MAJOR}::Network)
target_include_directories(timetable_elements PUBLIC modern)

add_library(timetable_desktop STATIC
    modern/MainWindow.h modern/MainWindow.cpp
    modern/DesktopOverlays.h modern/DesktopOverlays.cpp
    modern/QtCompat.h
    modern/ElementEditor.h modern/ElementEditor.cpp
    modern/ElementSelection.h
    modern/DesktopLayoutStore.h modern/DesktopLayoutStore.cpp
    modern/WindowEffects.h modern/WindowEffects.cpp
    modern/Translation.h modern/Translation.cpp modern/translations.qrc
)
target_link_libraries(timetable_desktop PUBLIC timetable_elements Qt${TT_QT_MAJOR}::Widgets)
target_include_directories(timetable_desktop PUBLIC modern)
target_compile_definitions(timetable_desktop PUBLIC
    TIMETABLE_QT_MAJOR=${TT_QT_MAJOR}
    TIMETABLE_VERSION="${PROJECT_VERSION}")
if(WIN32)
    target_link_libraries(timetable_desktop PRIVATE user32)
endif()
if(TT_QT_MAJOR EQUAL 6)
    qt_add_executable(TimeTableNext WIN32 modern/main.cpp)
else()
    add_executable(TimeTableNext WIN32 modern/main.cpp)
endif()
target_link_libraries(TimeTableNext PRIVATE timetable_desktop)
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
    add_executable(element_tests tests/element_tests.cpp)
    target_link_libraries(element_tests PRIVATE timetable_elements Qt${TT_QT_MAJOR}::Widgets)
    add_test(NAME element_tests COMMAND element_tests)
    set_tests_properties(element_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_executable(overlay_tests tests/overlay_tests.cpp)
    target_link_libraries(overlay_tests PRIVATE timetable_desktop)
    add_test(NAME overlay_tests COMMAND overlay_tests)
    add_executable(mainwindow_tests tests/mainwindow_tests.cpp)
    target_link_libraries(mainwindow_tests PRIVATE timetable_desktop)
    add_test(NAME mainwindow_tests COMMAND mainwindow_tests)
    set_tests_properties(overlay_tests mainwindow_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    add_executable(content_tests tests/content_tests.cpp)
    target_link_libraries(content_tests PRIVATE timetable_elements Qt${TT_QT_MAJOR}::Widgets)
    add_test(NAME content_tests COMMAND content_tests)
    add_executable(editor_tests tests/editor_tests.cpp)
    target_link_libraries(editor_tests PRIVATE timetable_desktop)
    add_test(NAME editor_tests COMMAND editor_tests)
    add_executable(layout_tests tests/layout_tests.cpp)
    target_link_libraries(layout_tests PRIVATE timetable_desktop)
    add_test(NAME layout_tests COMMAND layout_tests)
    add_executable(translation_tests tests/translation_tests.cpp)
    target_link_libraries(translation_tests PRIVATE timetable_desktop)
    add_test(NAME translation_tests COMMAND translation_tests)
    set_tests_properties(content_tests editor_tests layout_tests translation_tests PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    message(STATUS "Building TimeTableNext with Qt ${TT_QT_MAJOR}")
endif()
