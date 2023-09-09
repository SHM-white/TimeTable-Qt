#pragma once
#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <string>
//#include <windef.h>
#include <locale>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <qsettings.h>
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
#include <QDesktopServices>
#include <qmessagebox.h>
#include <exception>
#include "include\json\json.h"
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <shellapi.h>
#include <shlobj.h>
#include <memory>
#include <codecvt>
//#include "ToWString.h"
#pragma  comment(lib, "shell32.lib")

#define DEBUG

#ifdef DEBUG
#define SHOW_ITEM_RECT true
#else
#define SHOW_ITEM_RECT false
#endif // 

#define DEFAULT_CONFIG_PATH L"config.json"
#define DEFAULT_LESSONINFO_PATH L"Lessons.json"


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
