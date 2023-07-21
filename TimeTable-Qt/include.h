#pragma once
#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <string>
//#include <windef.h>

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
#pragma  comment(lib, "shell32.lib")

#define DEFAULT_CONFIG_PATH "config.json"
#define DEFAULT_LESSONINFO_PATH "Lessons.json"

//#define DEBUG 1

namespace Json {
	bool ChangeValue(const std::string& Item,const Json::Value& value,const std::string& filePath);
	Json::Value GetRootJsonValue(const std::string& TargetPath);
	int SaveJson(const std::string& TargetPath, const Json::Value& root);
	int mGetTextItem(const std::string& Item, std::string& input, const std::string& filePath);
	std::string mGetTextItem(const std::string& Item, const std::string& filePath, int);
	std::string utf8ToGB2312(const std::string &str);
}