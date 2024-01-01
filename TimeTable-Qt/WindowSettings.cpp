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
        msLessonNull = u8tw(Settings["LessonNull"].asString());
		for (int i = 0; i < Settings["BackGroundColor"].size(); i++) {
			miBackGroundColor[i] = Settings["BackGroundColor"][i].asInt();
		}
        msBackGroundImg = u8tw(Settings["BackGroundImg"].asString());
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
        mAutoOpenAllLessons = Settings["AutoOpenWindow"][0].asBool();
		bAcrylicEffect = Settings["AcrylicEffect"].asBool();
        mUseImgAsBackGround= Settings["ImgAsBackGround"].asBool();
	}
	return 1;
}
int WindowSettings::save()
{
	return save(msSettingPath);
}
int WindowSettings::save(const std::wstring& ConfigPath)
{
    // Create a Json writer
    Json::StyledWriter sw;
    // Create a Json root object
    Json::Value root;
    // Create a Json reader
    Json::Reader reader;
    // Open the input file stream
    std::ifstream in(msSettingPath, std::ios::in);
    // If the input file stream is open
    if (in.is_open()) {
        // Parse the input file stream and store the result in the root object
        reader.parse(in, root);
        // Close the input file stream
        in.close();
    }
    // Open the output file stream
    std::fstream os(ConfigPath, std::ios::out | std::ios::trunc);
    // Get the "Settings" object from the root object and then get the "Window" object from the "Settings" object
    Json::Value& Settings = root["Settings"]["Window"];
    // Clear the "WindowSize" array and append the window weight and height
    Settings["WindowSize"].clear();
    Settings["WindowSize"].append(miWindowWeight);
    Settings["WindowSize"].append(miWindowHeight);
    // Clear the "WindowLocation" array and append the window position and all lesson window position
    Settings["WindowLocation"].clear();
    Settings["WindowLocation"].append(miWindowX);
    Settings["WindowLocation"].append(miWindowY);
    Settings["WindowLocation"].append(miAllLessonWindowX);
    Settings["WindowLocation"].append(miAllLessonWindowY);
    // Set the "LessonInLine" and "LessonNull" properties
    Settings["LessonInLine"] = miLessonInLine;
    Settings["LessonNull"] = wtu8(msLessonNull);
    // Clear the "BackGroundColor" array and append each color
    Settings["BackGroundColor"].clear();
    for (auto color : miBackGroundColor) {
        Settings["BackGroundColor"].append(color);
    }
    // Set the "BackGroundImg" property
    Settings["BackGroundImg"] = wtu8(msBackGroundImg);
    // Clear the "TextFormat" array and append each text format
    Settings["TextFormat"].clear();
    for (const auto& format : msTextFormat) {
        Settings["TextFormat"].append(format.GetJsonValue());
    }
    // Set the "AcrylicEffect" and "UseImgBackGround" properties
    Settings["AcrylicEffect"] = bAcrylicEffect;
    Settings["ImgAsBackGround"] = mUseImgAsBackGround;
    // Set the "CountDownDayInLine" property
    Settings["CountDownDayInLine"] = miCountDownDayInLine;
    // Clear the "CountDownDay" array and append the count down day values
    Settings["CountDownDay"].clear();
    Settings["CountDownDay"].append(mCountDownDay.tm_year+1900);
    Settings["CountDownDay"].append(mCountDownDay.tm_mon+1);
    Settings["CountDownDay"].append(mCountDownDay.tm_mday);
    Settings["CountDownDay"].append(mCountDownDay.tm_hour);
    Settings["CountDownDay"].append(mCountDownDay.tm_min);
    Settings["CountDownDay"].append(mCountDownDay.tm_sec);
    
    Settings["AutoOpenWindow"].clear();
    Settings["AutoOpenWindow"].append(mAutoOpenAllLessons);
    // Write the root object to the output file stream
    os<<sw.write(root);
    // Close the output file stream
    os.close();
    
    // Return 0 to indicate success
    return 0;
}
//获取配置文件中某一项文本
int WindowSettings::mGetTextItem(const std::wstring& Item, std::wstring& input)
{
	return mGetTextItem(Item,input,msSettingPath);
}

int WindowSettings::mGetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath)
{
	return Json::mGetTextItem(Item, input, filePath);
}

std::wstring WindowSettings::mGetTextItem(const std::wstring& Item, const std::wstring& filePath, int)
{
	return Json::mGetTextItem(Item, filePath, 0);
}

const std::wstring WindowSettings::mChangeConfigPath(const std::wstring& path)
{
	if (path.size() == 0) {
		return L"";
	}
	std::wstring origin{ msSettingPath };
    Json::ChangeValue(L"ConfigFile", wtu8(path), DEFAULT_CONFIG_PATH);
	msSettingPath = path;
	return origin;
}
