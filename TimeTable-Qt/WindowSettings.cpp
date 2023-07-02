#include "WindowSettings.h"
#include "Timetable.h"
#include "include.h"
#include "include\json\json.h"
#include<format>


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
	Json::Value version;
	if (reader.parse(in, root)) {
		version = root["version"];
	}
	bool versionIsEnough{ true };
	{
		for (int i = 0; i < 3; i++) {
			versionIsEnough &= (version[i] >= minVersion[i]);
		}
	}
	if (versionIsEnough) {
		Json::Value Settings = root["Settings"]["Window"];
		miWindowHeight = Settings["WindowSize"][1].asInt();
		miWindowWeight = Settings["WindowSize"][0].asInt();
		miWindowX = Settings["WindowLocation"][0].asInt();
		miWindowY = Settings["WindowLocation"][1].asInt();
		miAllLessonWindowX = Settings["WindowLocation"][2].asInt();
		miAllLessonWindowY = Settings["WindowLocation"][3].asInt();
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
			TextFormat textformat(Settings["TextFormat"][i]);
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
	Settings["WindowSize"].append(miWindowWeight);
	Settings["WindowSize"].append(miWindowHeight);
	Settings["WindowLocation"].clear();
	Settings["WindowLocation"].append(miWindowX);
	Settings["WindowLocation"].append(miWindowY);
	Settings["WindowLocation"].append(miAllLessonWindowX);
	Settings["WindowLocation"].append(miAllLessonWindowY);
	Settings["LessonInLine"] = miLessonInLine;
	Settings["LessonNull"] = msLessonNull;
	//Settings["LessonInfoFile"] = msLessonInfoFile;
	Settings["BackGroundColor"].clear();
	for (auto color : miBackGroundColor) {
		Settings["BackGroundColor"].append(color);
	}
	Settings["BackGroundImg"] = msBackGroundImg;
	Settings["TextFormat"].clear();
	for (const auto& format : msTextFormat) {
		Settings["TextFormat"].append(format.GetJsonValue());
	}
	Settings["AcrylicEffect"] = bAcrylicEffect;
	Settings["CountDownDayInLine"] = miCountDownDayInLine;
	Settings["CountDownDay"].clear();
	Settings["CountDownDay"].append(mCountDownDay.tm_year+1900);
	Settings["CountDownDay"].append(mCountDownDay.tm_mon+1);
	Settings["CountDownDay"].append(mCountDownDay.tm_mday);
	Settings["CountDownDay"].append(mCountDownDay.tm_hour);
	Settings["CountDownDay"].append(mCountDownDay.tm_min);
	Settings["CountDownDay"].append(mCountDownDay.tm_sec);
	
	os<<sw.write(root);
	os.close();
	return 0;
}
//获取配置文件中某一项文本
int WindowSettings::mGetTextItem(const std::string& Item, std::string& input)
{
	return mGetTextItem(Item,input,msSettingPath);
}

int WindowSettings::mGetTextItem(const std::string& Item, std::string& input, const std::string& filePath)
{
	std::ifstream in(filePath, std::ios::in);
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
	return 0;
}

std::string WindowSettings::mGetTextItem(const std::string& Item, const std::string& filePath, int)
{
	std::string value;
	mGetTextItem(Item, value, filePath);
	return value;
}

const std::string WindowSettings::mChangeConfigPath(const std::string& path)
{
	if (path.size() == 0) {
		return "";
	}
	std::string origin{ msSettingPath };
	Json::ChangeValue("ConfigFile", path, DEFAULT_CONFIG_PATH);
	msSettingPath = path;
	return origin;
}
