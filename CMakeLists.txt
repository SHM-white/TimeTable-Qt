cmake_minimum_required(VERSION 3.21)
project(TimeTableNext VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_WIN32_EXECUTABLE ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

add_library(timetable_core STATIC
    modern/ScheduleStore.h
    modern/ScheduleStore.cpp
)
target_link_libraries(timetable_core PUBLIC Qt6::Core)
target_include_directories(timetable_core PUBLIC modern)

qt_add_executable(TimeTableNext WIN32
    modern/main.cpp
    modern/MainWindow.h
    modern/MainWindow.cpp
    modern/DesktopOverlays.h
    modern/DesktopOverlays.cpp
)
target_link_libraries(TimeTableNext PRIVATE timetable_core Qt6::Widgets)
set_target_properties(TimeTableNext PROPERTIES
    OUTPUT_NAME "TimeTableNext"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
if(WIN32)
    set_target_properties(TimeTableNext PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
include(CTest)
if(BUILD_TESTING)
    add_executable(schedule_tests tests/schedule_tests.cpp)
    target_link_libraries(schedule_tests PRIVATE timetable_core Qt6::Core)
    add_test(NAME schedule_tests COMMAND schedule_tests)
endif()
install(TARGETS TimeTableNext RUNTIME DESTINATION .)
