#pragma once

#include "include.h"
#include<string>
#include<string_view>
#include<format>
#include<vector>
#include<Windows.h>
#include"TextFormat.h"
#include "Timetable.h"
#include "include.h"

//#include "ToWString.h"

//WindowSettings类，用于保存设置，包含与窗口绘图有关函数
class WindowSettings {
public:
	const int* minVersion = minVersion_global;
	const int* currentVersion = currentVersion_global;
	WindowSettings() = default;
	WindowSettings(const WindowSettings& another) = default;
	WindowSettings(WindowSettings& another) = default;
	WindowSettings(const std::wstring path)
		:msSettingPath{ path }
	{
		msTextFormat.push_back(TextFormat());
		mGetWindowSettings();
	};
	WindowSettings& operator=(const WindowSettings& another) = default;
	int mGetWindowSettings();
	int save();
	int save(const std::wstring& ConfigPath);
	int mGetTextItem(const std::wstring& Item, std::wstring& input);	
	static int mGetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath);
	static std::wstring mGetTextItem(const std::wstring& Item, const std::wstring& filePath, int);
	const std::wstring mChangeConfigPath(const std::wstring& path);
	int miWindowHeight{ 120 };
	int miWindowWeight{ 250 };
	int miAllLessonWindowX{ 300 };
	int miAllLessonWindowY{ 20 };
	int miWindowX{ 20 };
	int miWindowY{ 20 };
	int miLessonInLine{ 1 };
	int miCountDownDayInLine{ 10 };
	std::vector<TextFormat> msTextFormat;
	std::wstring msLessonNull{ L"" };
	std::wstring msLessonInfoFile{ L".\\Lessons.json" };
	std::wstring msBackGroundImg{ L"" };
	int miBackGroundColor[4]{ 255,255,255,1 };
	bool mAutoOpenAllLessons {false};
	bool bAcrylicEffect{ false };
	bool mUseImgAsBackGround{ true };
	bool Utf8{ true };
	tm mCountDownDay;
	std::wstring msSettingPath{ L".\\Config.json" };
	std::vector<int> changedItems;
};
