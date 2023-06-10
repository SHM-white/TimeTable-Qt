#pragma once

#include<string>
#include<string_view>
#include<format>
#include<vector>
#include<Windows.h>
#include"TextFormat.h"
#include "Timetable.h"

#include "ToWString.h"

//WindowSettings类，用于保存设置，包含与窗口绘图有关函数
class WindowSettings {
public:
	const int minVersion[3]{ 2,3,0 };
	const int currentVersion[3]{ 2,4,0 };
	WindowSettings(const std::string path)
		:msSettingPath{ path }
	{
		msLessonNull = std::format(TEXT("找不到{}"), msSettingPath);
		msTextFormat.push_back(TextFormat());
		mGetWindowSettings();
	};
	WindowSettings() = delete;
	int mGetWindowSettings();
	int save();
	int save(const std::string& ConfigPath);
	int mGetTextItem(const std::string& Item, std::string& input);	
	const std::string mChangeConfigPath(const std::string& path);
	int miWindowHeight{ 120 };
	int miWindowWeight{ 250 };
	int miWindowX{ 20 };
	int miWindowY{ 20 };
	int miLessonInLine{ 1 };
	int miCountDownDayInLine{ 10 };
	std::vector<TextFormat> msTextFormat;
	std::string msLessonNull{ TEXT("") };
	std::string msLessonInfoFile{ TEXT(".\\Lessons.json") };
	std::string msBackGroundImg{ TEXT("") };
	COLORREF mcBackGroundColor{ 0x00ffffff };
	int miBackGroundColor[4]{ 255,255,255,1 };
	bool bAcrylicEffect{ false };
	bool Utf8{ true };
	tm mCountDownDay;
	std::string msSettingPath{ TEXT(".\\Config.json") };
	std::vector<int> changedItems;
};
