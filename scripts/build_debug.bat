@echo off

set QT_DIR=c:\Qt\6.4.3\msvc2019_64
set SRC_DIR=%cd%
set BUILD_DIR=%cd%\build

if not exist %QT_DIR% exit
if not exist %SRC_DIR% exit
if not exist %BUILD_DIR% md %BUILD_DIR%

cd build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

%QT_DIR%\bin\qmake.exe %SRC_DIR%\TimeTable-Qt\TimeTable-Qt.pro -spec win32-msvc  "CONFIG+=debug"
if exist %BUILD_DIR%\debug\TimeTable-Qt.exe del %BUILD_DIR%\debug\TimeTable-Qt.exe
nmake Debug
if not exist %BUILD_DIR%\debug\Qt6Cored.dll (
  %QT_DIR%\bin\windeployqt.exe %BUILD_DIR%\debug\TimeTable-Qt.exe
)
if not exist %BUILD_DIR%\debug\hv.dll (y
  xcopy %SRC_DIR%\TimeTable-Qt\hv\bin\Debug\hv.dll %BUILD_DIR%\debug\
)
if not exist %BUILD_DIR%\debug\jsoncpp.dll (
  xcopy %SRC_DIR%\TimeTable-Qt\jsoncpp\bin\Debug\jsoncpp.dll %BUILD_DIR%\debug\
)