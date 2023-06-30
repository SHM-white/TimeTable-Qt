#pragma once
#include <QtWidgets/QWidget>
#include <qtimer.h>
#include <qpixmap.h>
#include <string>
#include <windef.h>
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
#include <Windows.h>
#include <fstream>
#include <vector>
#include <strsafe.h>
#include <ctime>
#include <QDesktopServices>
#include <qmessagebox.h>
#include <exception>
#include "include\json\json.h"


#define DEFAULT_CONFIG_PATH "config.json"
#define DEFAULT_LESSONINFO_PATH "Lessons.json"

namespace Json {
	bool ChangeValue(const std::string& Item,const Json::Value& value,const std::string& filePath);
	Json::Value GetRootJsonValue(const std::string& TargetPath);
	int SaveJson(const std::string& TargetPath, const Json::Value& root);
}