﻿#pragma once

#define WIN32_LEAN_AND_MEAN
#define AUTO_RUN_KEY	"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"

#include <string>
#include <locale>
#include <Winsock2.h>
#include <windows.h>
#include <vector>
#include <stdlib.h>
#include <functional>
#include <regex>
#include <format>
#include <algorithm>
#include <ranges>
#include <fstream>
#include <strsafe.h>
#include <ctime>
#include <time.h>
#include <exception>
#include <shellapi.h>
#include <shlobj.h>
#include <memory>
#include <codecvt>
#include <chrono>
#include <thread>
#include <future>

#include "include\json\json.h"
#include "hv.h"
#include "requests.h"

#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <qsettings.h>
#include <QDesktopServices>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include "qrect.h"
#include "qpoint.h"
#include "qpainter.h"
/*
#include "UIElementBase.h"
#include "EmptyUIElement.h"
#include "SingleItemUIElementBase.h"
#include "MultiItemInOrderUIElementBase.h"
#include "MultiItemAllDisplayUIElementBase.h"
#include "MultiTextItem.h"
#include "SingleTextItem.h"
*/

#pragma  comment(lib, "shell32.lib")

#undef max
#undef GetCurrentTime

#ifdef QT_DEBUG
#define DEBUG
#endif // QT_DEBUG

#ifdef DEBUG
#define SHOW_ITEM_RECT true
#else
#define SHOW_ITEM_RECT false
#endif // 

#ifdef DEBUG
#define STRANGE_TEST false
#endif // DEBUG

#define ITEM_SCROLL_SPEED 10

#define DEFAULT_CONFIG_PATH L"config.json"
#define DEFAULT_LESSONINFO_PATH L"Lessons.json"

#if QT_VERSION > 0x060000
#define QT6
#else
#define QT5
#endif
const int minVersion_global[3]{ 3,0,0 };
const int currentVersion_global[3]{ 3,0,0 };

namespace Json {
	bool ChangeValue(const std::wstring& Item,const Json::Value& value,const std::wstring& filePath);
	Json::Value GetRootJsonValue(const std::wstring& TargetPath);
	int SaveJson(const std::wstring& TargetPath, const Json::Value& root);
	int mGetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath);
	std::wstring mGetTextItem(const std::wstring& Item, const std::wstring& filePath, int);
    Json::Value GetJsonValueFromWStr(const std::wstring& str);
    Json::Value GetJsonValueFromStr(const std::string& str);
}

#if EXPERENCE

// convert string to wstring
std::wstring to_wide_string(const std::wstring& input)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
}
// convert wstring to string
std::wstring to_byte_string(const std::wstring& input)
{
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
}

#ifdef UNICODE
#define string wstring
#define asString() asString().c_str()
#endif // UNICODE

#endif //  EXPERENCE

namespace ToolFunctions {
    std::string wstringToU8string(const std::wstring& wstring);
    std::wstring u8stringToWstring(const std::string& u8string);
    std::string wstringToAnsiString(const std::wstring& u8wstring);
}

static auto u8tw = ToolFunctions::u8stringToWstring;
static auto wtu8 = ToolFunctions::wstringToU8string;

enum Days
{
    Null,
    Mon, 
    Tue, 
    Wed, 
    Thu, 
    Fri, 
    Sat, 
    Sun, 
};

std::wstring GetWStrDay(Days day);
//std::string GetStrDay(Days day);
const std::wstring DaysCollectionW[8]{ L"Null",L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
//const std::string DaysCollection[8]{ "Null","Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
