#pragma once

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

#pragma  comment(lib, "shell32.lib")

#undef max

#ifdef QT_DEBUG
#define DEBUG
#endif // QT_DEBUG

#ifdef DEBUG
#define SHOW_ITEM_RECT true
#else
#define SHOW_ITEM_RECT false
#endif // 

#define DEFAULT_CONFIG_PATH L"config.json"
#define DEFAULT_LESSONINFO_PATH L"Lessons.json"

#if QT_VERSION > 0x060000
#define QT6
#else
#define QT5
#endif
const int minVersion_global[3]{ 2,5,0 };
const int currentVersion_global[3]{ 3,0,0 };

namespace Json {
	bool ChangeValue(const std::wstring& Item,const Json::Value& value,const std::wstring& filePath);
	Json::Value GetRootJsonValue(const std::wstring& TargetPath);
	int SaveJson(const std::wstring& TargetPath, const Json::Value& root);
	int mGetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath);
	std::wstring mGetTextItem(const std::wstring& Item, const std::wstring& filePath, int);
	
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
}

static auto u8tw = ToolFunctions::u8stringToWstring;
static auto wtu8 = ToolFunctions::wstringToU8string;
