#include "WindowSettings.h"
#include "Timetable.h"
#include <Windows.h>
#include <fstream>
#include <vector>
#include "include\json\json.h"
#include<format>
#include<strsafe.h>

//WindowSettings类的实现函数
//初始化时获取设置
int WindowSettings::mGetWindowSettings()
{
	std::ifstream in(msSettingPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	//root["emitUTF8"] = true;
	if (reader.parse(in, root)) {
		Json::Value Settings = root["Settings"]["Window"];
		miWindowHeight = Settings["WindowSize"][1].asInt();
		miWindowWeight = Settings["WindowSize"][0].asInt();
		miWindowX = Settings["WindowLocation"][0].asInt();
		miWindowY = Settings["WindowLocation"][1].asInt();
		miLessonInLine = Settings["LessonInLine"].asInt();
		msLessonNull = Settings["LessonNull"].asString();
		msLessonInfoFile = Settings["LessonInfoFile"].asString();
		mcBackGroundColor = RGB(Settings["BackGroundColor"][0].asInt(), Settings["BackGroundColor"][1].asInt(), Settings["BackGroundColor"][2].asInt());
		for (int i = 0; i < Settings["BackGroundColor"].size(); i++) {
			miBackGroundColor[i] = Settings["BackGroundColor"][i].asInt();
		}
		msBackGroundImg = Settings["BackGroundImg"].asString();
		msTextFormat.clear();
		for (int i = 0; i < (int)Settings["TextFormat"].size(); i++) {
			Json::Value Format = Settings["TextFormat"][i];
			TextFormat textformat(Format[2][0].asInt(), Format[2][1].asInt(), Format[3].asInt(), Format[1].asString(), Format[0].asString(), RGB(Format[4][0].asInt(), Format[4][1].asInt(), Format[4][2].asInt()));
			msTextFormat.push_back(textformat);
		}
		miCountDownDayInLine = Settings["CountDownDayInLine"].asInt();
		mCountDownDay.tm_year = Settings["CountDownDay"][0].asInt() - 1900;
		mCountDownDay.tm_mon = Settings["CountDownDay"][1].asInt() - 1;
		mCountDownDay.tm_mday = Settings["CountDownDay"][2].asInt();
		mCountDownDay.tm_hour = Settings["CountDownDay"][3].asInt();
		mCountDownDay.tm_min = Settings["CountDownDay"][4].asInt();
		mCountDownDay.tm_sec = Settings["CountDownDay"][5].asInt();
		bAcrylicEffect = Settings["AcrylicEffect"].asBool();
	}
	return 1;
}
int WindowSettings::save()
{
	return save(msSettingPath);
}
int WindowSettings::save(const std::string& ConfigPath)
{
	Json::StyledWriter sw;
	Json::Value root;
	Json::Reader reader;
	std::ifstream in(msSettingPath, std::ios::in);
	if (in.is_open()) {
		reader.parse(in, root);
		in.close();
	}
	std::fstream os(ConfigPath, std::ios::out | std::ios::trunc);
	Json::Value& Settings = root["Settings"]["Window"];
	//fill settings
	Settings["WindowSize"].clear();
	Settings["WindowSize"]
		.append(miWindowWeight)
		.append(miWindowHeight);
	Settings["WindowLocation"].clear();
	Settings["WindowLocation"]
		.append(miWindowX)
		.append(miWindowY);
	Settings["LessonInLine"] = miLessonInLine;
	Settings["LessonNull"] = msLessonNull;
	Settings["LessonInfoFile"] = msLessonInfoFile;
	Settings["BackGroundColor"].clear();
	for (auto color : miBackGroundColor) {
		Settings["BackGroundColor"].append(color);
	}
	Settings["BackGroundImg"] = msBackGroundImg;
	Settings["TextFormat"].clear();
	for (auto format : msTextFormat) {
		Json::Value formatItem;
		Json::Value color;
		color
			.append(GetRValue(format.color))
			.append(GetBValue(format.color))
			.append(GetGValue(format.color));
		formatItem
			.append(format.msFontName)
			.append(format.msFontName)
			.append(Json::Value()
				.append((int)format.mpTextLocation.x)
				.append((int)format.mpTextLocation.y)
				.append(format.miTextSize)
				.append(color));
		Settings["TextFormat"].append(formatItem);
	}
	Settings["AcrylicEffect"] = bAcrylicEffect;
	Settings["CountDownDayInLine"] = miCountDownDayInLine;
	Settings["CountDownDay"].clear();
	Settings["CountDownDay"]
		.append(mCountDownDay.tm_year)
		.append(mCountDownDay.tm_mon)
		.append(mCountDownDay.tm_mday)
		.append(mCountDownDay.tm_hour)
		.append(mCountDownDay.tm_min)
		.append(mCountDownDay.tm_sec);
	/*
	miWindowHeight = Settings["WindowSize"][1].asInt();
		miWindowWeight = Settings["WindowSize"][0].asInt();
		miWindowX = Settings["WindowLocation"][0].asInt();
		miWindowY = Settings["WindowLocation"][1].asInt();
		miLessonInLine = Settings["LessonInLine"].asInt();
		msLessonNull = Settings["LessonNull"].asString();
		msLessonInfoFile = Settings["LessonInfoFile"].asString();
		mcBackGroundColor = RGB(Settings["BackGroundColor"][0].asInt(), Settings["BackGroundColor"][1].asInt(), Settings["BackGroundColor"][2].asInt());
		for (int i = 0; i < Settings["BackGroundColor"].size(); i++) {
			miBackGroundColor[i] = Settings["BackGroundColor"][i].asInt();
		}
		msBackGroundImg = Settings["BackGroundImg"].asString();
		msTextFormat.clear();
		for (int i = 0; i < (int)Settings["TextFormat"].size(); i++) {
			Json::Value Format = Settings["TextFormat"][i];
			TextFormat textformat(Format[2][0].asInt(), Format[2][1].asInt(), Format[3].asInt(), Format[1].asString(), Format[0].asString(), RGB(Format[4][0].asInt(), Format[4][1].asInt(), Format[4][2].asInt()));
			msTextFormat.push_back(textformat);
		}
		miCountDownDayInLine = Settings["CountDownDayInLine"].asInt();
		mCountDownDay.tm_year = Settings["CountDownDay"][0].asInt() - 1900;
		mCountDownDay.tm_mon = Settings["CountDownDay"][1].asInt() - 1;
		mCountDownDay.tm_mday = Settings["CountDownDay"][2].asInt();
		mCountDownDay.tm_hour = Settings["CountDownDay"][3].asInt();
		mCountDownDay.tm_min = Settings["CountDownDay"][4].asInt();
		mCountDownDay.tm_sec = Settings["CountDownDay"][5].asInt();
		bAcrylicEffect = Settings["AcrylicEffect"].asBool();*/
	os<<sw.write(root);
	os.close();
	return 0;
}
//获取配置文件中某一项文本
int WindowSettings::mGetTextItem(const std::string& Item, std::string& input)
{
	std::ifstream in(msSettingPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(in, root)) {
		const Json::Value Text = root[Item];
		input = Text.asString();
	}
	in.close();
	return 1;
}
//绘制主窗口文字
//int WindowSettings::mPrintText(QPainter& painter, TimeTable& timetable) {
//	int i = 1;
//	for (TextFormat a : msTextFormat) {
//		QFont font;
//		painter.setPen(a.color);
//		font.setFamily(a.msFontName.c_str());
//		font.setPointSize(a.miTextSize);
//		painter.setFont(font);
//		std::string Text;
//		if (i == miLessonInLine) {
//			Text = timetable.mGetCurrentTime(a.msTextFormat) + timetable.mGetCurrentLesson(msLessonNull);
//		}
//		else if (i == miCountDownDayInLine) {
//			Text = timetable.mGetCountDown(mCountDownDay, a.msTextFormat);
//		}
//		else {
//			Text = timetable.mGetCurrentTime(a.msTextFormat);
//		}
//		painter.drawText(a.mpTextLocation.x, a.mpTextLocation.y, QString::fromStdString(Text));
//		i++;
//	}
//	return 0;
//}

bool WindowSettings::mChangeConfigPath(const std::string& path)
{
	if (path.size() == 0) {
		return false;
	}
	msSettingPath = path;
	return true;
}
/*int WindowSettings::mPrintText(HDC& hdc, TimeTable& timetable)
{
	  int i = 1;
	  for (TextFormat a : msTextFormat) {
		  HFONT hFont = CreateFont(a.miTextSize, 0, 0, 0,0, 0, 0, 0, GB2312_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, (LPCWSTR)a.msFontName.c_str());
		  SelectObject(hdc, hFont);
		  SetTextColor(hdc, a.color);
		  std::string Text;
		  if (i == miLessonInLine) {
			  Text = timetable.mGetCurrentTime(a.msTextFormat) + timetable.mGetCurrentLesson(msLessonNull);
		  }
		  else if (i == miCountDownDayInLine) {
			  Text = timetable.mGetCountDown(mCountDownDay, a.msTextFormat);
		  }
		  else {
			  Text = timetable.mGetCurrentTime(a.msTextFormat);
		  }
		  TextOut(hdc, a.mpTextLocation.x, a.mpTextLocation.y, (LPCWSTR)Text.c_str(), (int)Text.length());
		  SetTextColor(hdc, RGB(0, 0, 0));
		  DeleteObject(hFont);
		  i++;
	  }

	return 0;
}*/
//切换菜单项勾选状态
/*LPMENUITEMINFO WindowSettings::mSwitchMenuItemCheck(HWND& hWnd, HMENU& hMenu, DWORD MenuItem)
{
	static MENUITEMINFO MenuItemInfo;
	LPMENUITEMINFO lpMenuItemInfo = &MenuItemInfo;
	lpMenuItemInfo->cbSize = sizeof(MENUITEMINFO);
	lpMenuItemInfo->fMask = MIIM_STATE;//只获取菜单项状态（应该）
	GetMenuItemInfo(hMenu, MenuItem, FALSE, lpMenuItemInfo);//获取当前状态
	if (lpMenuItemInfo->fState & MFS_CHECKED) {//菜单项checked
		lpMenuItemInfo->fState = MFS_UNCHECKED;
	}
	else {//菜单项unchecked
		lpMenuItemInfo->fState = MFS_CHECKED;
	}
	SetMenuItemInfo(hMenu, MenuItem, FALSE, lpMenuItemInfo);
	return lpMenuItemInfo;
}*/
