@echo off

set QT_DIR=c:\Qt\5.15.2\msvc2019_64
set SRC_DIR=%cd%
set BUILD_DIR=%cd%\build_qt5

if not exist %QT_DIR% exit
if not exist %SRC_DIR% exit
if not exist %BUILD_DIR% md %BUILD_DIR%

cd build_qt5

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

%QT_DIR%\bin\qmake.exe %SRC_DIR%\TimeTable-Qt\TimeTable-Qt.pro -spec win32-msvc  "CONFIG+=release"
if exist %BUILD_DIR%\release\TimeTable-Qt.exe del %BUILD_DIR%\release\TimeTable-Qt.exe
nmake Release
if not exist %BUILD_DIR%\release\Qt5Core.dll (
  %QT_DIR%\bin\windeployqt.exe %BUILD_DIR%\release\TimeTable-Qt.exe
)
if not exist %BUILD_DIR%\release\hv.dll (
  xcopy %SRC_DIR%\TimeTable-Qt\hv\bin\Release\hv.dll %BUILD_DIR%\release\
)
if not exist %BUILD_DIR%\release\jsoncpp.dll (
  xcopy %SRC_DIR%\TimeTable-Qt\jsoncpp\bin\Release\jsoncpp.dll %BUILD_DIR%\release\
)
del %BUILD_DIR%\release\*.obj
del %BUILD_DIR%\release\*.cpp