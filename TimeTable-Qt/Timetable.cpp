﻿#include "include.h"
#include "Timetable.h"
#include "include\json\json.h"
#include "CSVEditor.h"
#include "Lesson.h"

// This function is an overloaded version of the mAddLesson function in the TimeTable class.
// It adds a lesson with four parameters: week, Lesson, sBegin, sEnd.
// If the fifth parameter, mLessonInfoPath, is not provided, it defaults to the value of the member variable mLessonInfoPath.
// Returns the result of calling the mAddLesson function with the provided parameters.
int TimeTable::AddLesson(const std::wstring &week, const std::wstring &Lesson, const std::wstring &sBegin, const std::wstring &sEnd)
{
	// Call the mAddLesson function with the provided parameters and the default mLessonInfoPath.
	return AddLesson(week, Lesson, sBegin, sEnd, mLessonInfoPath);
}

// 添加课程5参数函数
int TimeTable::AddLesson(const std::wstring &week, const std::wstring &Lesson, const std::wstring &sBegin, const std::wstring &sEnd, const std::wstring &TargetFilePath)
{
	if (!((bool)week.size() && (bool)Lesson.size() && (bool)sBegin.size() && (bool)sEnd.size() && (bool)TargetFilePath.size()))
	{
		return 0;
	}
	Json::Value root = Json::GetRootJsonValue(TargetFilePath);
	Json::Value Current;
	Current.append(wtu8(Lesson));
	Current.append(wtu8(sBegin));
	Current.append(wtu8(sEnd));
	root[wtu8(week)]["Lessons"].append(Current);
	return SaveJson(TargetFilePath, root);
}
int TimeTable::AddLesson(const Lesson &lesson)
{
	return AddLesson(lesson, mLessonInfoPath);
}
int TimeTable::AddLesson(const Lesson &lesson, const std::wstring &TargetFilePath)
{
	return AddLesson(lesson.mGetDay(), lesson.mGetName(), lesson.GetBeginTimeAsString(), lesson.GetEndTimeAsString(), TargetFilePath);
}
// 添加更多信息，有空也加个重载
int TimeTable::AddMoreInfo(const std::wstring &Days, const std::wstring &Info)
{
	std::fstream os;
	os.open(mLessonInfoPath, std::ios::out | std::ios::in);
	if (!os.is_open())
	{
		return 0;
	}
	Json::Reader reader;
	Json::Value root;
	Json::StyledWriter sw;
	reader.parse(os, root);
	root[wtu8(Days)]["Infos"].append(wtu8(Info));
	os.seekp(std::ios::beg);
	os << sw.write(root);
	os.close();
	os.clear();
	return 0;
}
int TimeTable::ReloadLesson()
{
	CurrentLesson = Lesson();
	return 0;
}
// 转换时间为分钟数便于比较大小
int TimeTable::mHHMMToMin(int input)
{
	return (input - input % 100) / 100 * 60 + input % 100;
}
// 暂时替换文件路径，同时返回旧的文件路径
std::wstring TimeTable::ReplacePath(const std::wstring &Path)
{
	std::wstring old = mLessonInfoPath;
	Json::ChangeValue(L"LessonInfoFile", wtu8(Path), DEFAULT_CONFIG_PATH);
	mLessonInfoPath = Path;
	return old;
}
std::vector<Lesson> TimeTable::GetLessons()
{
	auto root = Json::GetRootJsonValue(mLessonInfoPath);
	std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	std::vector<Lesson> lessons;
	for (const auto& a : Days)
	{
		for (auto& i : root[a]["Lessons"]) {
			lessons.push_back(Lesson(u8tw(a), i));
		}
	}
	return lessons;
}
std::vector<Lesson> TimeTable::GetTodayLessons(const std::wstring& day)
{
	std::vector<Lesson> todayLesson;
	auto lessons = GetLessons();
	auto isNeededDayLesson = [&](Lesson& lesson) {
		if (day == L"Null") {
			return lesson.mGetDay() == GetCurrentTime(L"%a");
		}
		else
		{
			return lesson.mGetDay() == day;
		}
		};
	for (auto& i : lessons | std::views::filter(isNeededDayLesson)) {
		todayLesson.push_back(std::move(i));
	}
	return todayLesson;
}
Lesson TimeTable::GetNextLesson()
{
	using namespace std::ranges::views;
	auto lessons = GetLessons();
	std::sort(lessons.begin(), lessons.end());
	bool lesson_future{ false };
	for (auto& i : lessons | std::views::filter([](Lesson& lesson) {return lesson.mGetDay() == GetCurrentTime(L"%a"); })) {
		if (i.GetEndMin() > GetCurrentTimeMin()) {
			lesson_future = true;
		}
		if (i.GetBeginMin() > GetCurrentTimeMin() && lesson_future == true) {
			return i;
		}
	}
	return Lesson(L"Null", L"", 2359, 2400);
}
Lesson TimeTable::GetCurrentLesson()
{
	auto lessons = GetLessons();
	using namespace std::ranges::views;
	for (auto& i : lessons | std::views::filter([](Lesson& lesson) {return lesson.mGetDay() == GetCurrentTime(L"%a"); })) {
		if (i.IsOnLesson(GetCurrentTimeMin())) {
			return i;
		}
	}
	return Lesson(L"Null", L"Null", 0, 2400);
}
Lesson TimeTable::GetLesson(const std::wstring &week, int index)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if ((!in.is_open()) || (!reader.parse(in, root)))
	{
		return Lesson();
	};
	in.close();
	const Json::Value Lessons = root[wtu8(week)]["Lessons"][index];
	return Lesson(week, u8tw(Lessons[0].asString()), atoi(Lessons[1].asString().c_str()), atoi(Lessons[2].asString().c_str()));
}

// 获取所有的课程并返回至传入的数组
int TimeTable::GetLesson(std::vector<std::wstring> &input)
{
	input.clear();
	std::wstring Days[]{L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun"};
	for (const auto &a : Days)
	{
		GetLesson(input, a);
	}
	return 1;
}
// 获取今日的信息并存入传入的数组
int TimeTable::GetTodayMoreInfo(std::vector<std::wstring> &input)
{
	std::wstring week{GetCurrentTime(L"%a")};
	return GetTodayMoreInfo(input, week);
}
int TimeTable::GetTodayMoreInfo(std::vector<std::wstring> &input, const std::wstring week)
{
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(in, root))
	{
		const Json::Value Infos = root[wtu8(week)]["Infos"];
		for (unsigned int i = 0; i < Infos.size(); ++i)
		{
			input.push_back(u8tw(Infos[i].asString()));
		}
	}
	in.close();
	return 1;
}
// 获取当前时间的课程
std::wstring TimeTable::GetCurrentLesson(const std::wstring &LessonNull)
{
	int lessonIndex = GetCurrentLesson(0);
	if (lessonIndex < 0)
	{
		return LessonNull;
	}
	return CurrentLesson.mGetName();
}

std::wstring TimeTable::GetCurrentTime(const std::wstring &TextFormat)
{
    wchar_t result[1024];
	tm structm;

	GetCurrentTime(structm);
	auto location = TextFormat.find(L'%', 0);
	if (location >= TextFormat.size())
	{
		return TextFormat;
	}
    if (TextFormat.size() >= 2 && !std::regex_match(TextFormat.begin() + location, TextFormat.begin() + location + 2, std::regex("%[^aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%]|%[0-9]{1,}[a-zA-Z]")))
	{
        wcsftime(result, sizeof(result)/sizeof(wchar_t), TextFormat.c_str(), &structm);
	}
	else
	{
		return TextFormat;
	}
	return std::wstring(result);
}
// 从csv导入课程至指定文件
int TimeTable::ImportLessonsFromCsv(const std::wstring &path, const std::wstring &TargetFileName)
{
	CSVEditor CsvEditor{path};
	int a{0};
	if (CsvEditor.mGetCsvData())
	{
		for (int i{1}; i < CsvEditor.mGetLineCount(); i++)
		{ // jump line 1
			a = AddLesson(CsvEditor[i][0], CsvEditor[i][1], CsvEditor[i][2], CsvEditor[i][3], TargetFileName);
		}
		return a;
	}
	return 0;
}
int TimeTable::sortLessons(const std::wstring &Day)
{
	return sortLessons(mLessonInfoPath, Day);
}
// 排序
int TimeTable::sortLessons(const std::wstring &lessonPath, const std::wstring &Day)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root))
	{
		in.close();
		return 0;
	}

	std::vector<Lesson> vectorLessons;

	for (const auto &i : DaysCollectionW)
	{
		if ((Day != i) && (Day != L"All"))
		{
			continue;
		}
		if (i == L"Null")
		{
			continue;
		}
		Json::Value &valueLessons = root[wtu8(i)]["Lessons"];
		vectorLessons.clear();
		for (const auto &value : valueLessons)
		{
			vectorLessons.push_back(
				Lesson(i,
					   u8tw(value[0].asString()),
					   atoi(value[1].asString().c_str()),
					   atoi(value[2].asString().c_str())));
		}
		std::sort(vectorLessons.begin(), vectorLessons.end(), [](const Lesson &lesson1, const Lesson &lesson2)
				  { return lesson1.GetBeginTime() < lesson2.GetBeginTime(); });
		valueLessons.clear();
		valueLessons = Json::Value(Json::arrayValue);
		for (const auto &lesson : vectorLessons)
		{
			Json::Value a = lesson.GetJsonValue();
			valueLessons.append(a);
		}
	}

	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		return 0;
	}

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	return 1;
}

// int TimeTable::insert(size_t index, const Lesson& lesson, const std::wstring& targetFile)
//{
//	return 0;
// }
// 获取课程信息地址
const std::wstring &TimeTable::GetLessonInfoPath()
{
	return mLessonInfoPath;
}
int TimeTable::mMinToHHMM(int input)
{
	int Hour = (input - input % 60) / 60;
	return Hour * 100 + input % 60;
}
// 获取当前时间并存入传入的结构中
int TimeTable::GetCurrentTime(tm &tmTime)
{
	time_t timep;
	time(&timep);
	localtime_s(&tmTime, &timep);
	return 0;
}
// 暂未实现
// int TimeTable::insert(size_t index, const Lesson& lesson)
//{
//	return insert(index,lesson,mLessonInfoPath);
// }
// 删除课程
int TimeTable::deleteLesson(size_t index, const std::wstring &day, const std::wstring &lessonPath)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root))
	{
		in.close();
		return 0;
	}

	Json::Value &valueLessons = root[wtu8(day)]["Lessons"];
	std::vector<Lesson> vectorLessons;
	vectorLessons.clear();
	for (const auto &value : valueLessons)
	{
		vectorLessons.push_back(
			Lesson(day,
				   u8tw(value[0].asString()),
				   atoi(value[1].asString().c_str()),
				   atoi(value[2].asString().c_str())));
	}

	vectorLessons.erase(vectorLessons.begin() + index);

	// write back to json value
	valueLessons.clear();
	valueLessons = Json::Value(Json::arrayValue);
	for (const auto &lesson : vectorLessons)
	{
		valueLessons.append(lesson.GetJsonValue());
	}

	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		return 0;
	}

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	ReloadLesson();
	return 1;
}
int TimeTable::deleteInfo(size_t index, const std::wstring &day)
{
	return deleteInfo(index, day, mLessonInfoPath);
}
int TimeTable::deleteInfo(size_t index, const std::wstring &day, const std::wstring &lessonPath)
{
	Json::Value root = Json::GetRootJsonValue(lessonPath);
	std::vector<std::wstring> vectorInfos;
	Json::Value &valueInfos = root[wtu8(day)]["Infos"];
	for (const auto &a : valueInfos)
	{
		vectorInfos.push_back(u8tw(a.asString()));
	}
	vectorInfos.erase(vectorInfos.begin() + index);
	valueInfos.clear();
	for (const auto &a : vectorInfos)
	{
		valueInfos.append(wtu8(a));
	}
	return SaveJson(lessonPath, root);
}
int TimeTable::changeLesson(size_t index, const std::wstring &day, const Lesson &lesson)
{
	return changeLesson(index, day, lesson, mLessonInfoPath);
}
int TimeTable::changeLesson(size_t index, const std::wstring &day, const Lesson &lesson, const std::wstring &lessonPath)
{
	std::ifstream in(lessonPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root))
	{
		in.close();
		return 0;
	}

	Json::Value &valueLessons = root[wtu8(day)]["Lessons"];
	valueLessons[(int)index] = lesson.GetJsonValue();

	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		return 0;
	}

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	ReloadLesson();
	return 1;
}
int TimeTable::changeInfo(size_t index, const std::wstring &day, const std::wstring &info)
{
	return changeInfo(index, day, info, mLessonInfoPath);
}
// change info
/*
using jsoncpp library
1. call Json::GetRootJsonValue to get the root json in the file of Path
3. Change the value at root[day]["Infos"][index] to info
4. Write the modified JSON back by calling Json::SaveJson
5. Return 1 if successful, -1 otherwise
*/
int TimeTable::changeInfo(size_t index, const std::wstring &day, const std::wstring &info, const std::wstring &Path)
{
	// Read the contents of the JSON file
	std::ifstream file(Path);
	Json::Value root;
	file >> root;

	// Change the value at root[day]["Infos"][index] to info
	root[wtu8(day)]["Infos"][(int)index] = wtu8(info);
	// Write the modified JSON back to the file
	Json::SaveJson(Path, root);
	// Return 1 if successful, -1 otherwise
	return 1;
}

int TimeTable::SaveJson(const std::wstring &TargetPath, const Json::Value &root)
{
	int result = Json::SaveJson(TargetPath, root);
	ReloadLesson();
	return result;
}
/**
 * Returns the index of the current lesson based on the current time.
 * @param None
 * @return The index of the current lesson
 */
int TimeTable::GetCurrentLesson(int)
{
	// Initialize currentIndex to 0
	static int currentIndex{0};

	// Get the current time in the format HHMM
	std::wstring timeCurrentTime{GetCurrentTime(L"%H%M")};

	// Convert the current time to minutes
	int iCurrentTime = mHHMMToMin(_wtoi(timeCurrentTime.c_str()));

	// Check if the current time is within the current lesson's time range
	if (!((CurrentLesson.GetBeginTime() <= iCurrentTime) && (iCurrentTime <= CurrentLesson.GetEndTime())))
	{
		// Get the lesson information from the lesson info JSON file
		Json::Value root = Json::GetRootJsonValue(mLessonInfoPath);

		// Get the current day of the week
		std::wstring week{GetCurrentTime(L"%a")};

		// Get the lessons for the current day
		const Json::Value Lessons = root[wtu8(week)]["Lessons"];

		// Iterate through the lessons
		for (unsigned int i = 0; i < Lessons.size(); ++i)
		{
			// Get the begin and end times of the current lesson
			std::wstring sBeginTime = u8tw(Lessons[i][1].asString());
			std::wstring sEndTime = u8tw(Lessons[i][2].asString());
			int iBeginTime = mHHMMToMin(_wtoi(sBeginTime.c_str()));
			int iEndTime = mHHMMToMin(_wtoi(sEndTime.c_str()));

			// Check if the current time is within the current lesson's time range
			if ((iBeginTime <= iCurrentTime) && (iEndTime >= iCurrentTime))
			{
				// Set the current lesson and update the currentIndex
				CurrentLesson.SetValue(u8tw(Lessons[i][0].asString()), iBeginTime, iEndTime);
				currentIndex = i;
				return currentIndex;
			}
		}
		currentIndex = -1;
	}

	// Return the index of the current lesson
	return currentIndex;
}
int TimeTable::GetCurrentTimeMin()
{
	return mHHMMToMin(_wtoi(GetCurrentTime(L"%H%M").c_str()));
}
// This function retrieves weather information from a specified city using an API and returns the weather as a formatted string. It also updates a boolean pointer to indicate whether the operation was successful or not.
std::wstring TimeTable::GetWeather(const std::wstring& code, const std::wstring& APIKey, bool* isSuccess)
{
	static std::wstring weather{L"getting...."};
	static std::future<requests::Response> result;
	static std::future_status status = std::future_status::deferred;
	try
	{
		if ((status == std::future_status::deferred) && !(*isSuccess))
		{
			result = std::async(std::launch::async, GetResponseFromUrlSync, std::format(L"https://restapi.amap.com/v3/weather/weatherInfo?city={}&key={}&extensions=all", code, APIKey));
			status = std::future_status::timeout;
		}
		else if (status == std::future_status::timeout)
		{
			status = result.wait_for(std::chrono::microseconds(1));
		}
		else if (status == std::future_status::ready)
		{
			Json::Value root;
			Json::Reader reader;
			if (reader.parse(result.get().get()->body, root))
			{
				status = std::future_status::deferred;
				weather = std::format(L"今日天气: {}~{}℃,{},明日天气：{}~{}℃,{}",
									  u8tw(root["forecasts"][0]["casts"][0]["nighttemp"].asString()),
									  u8tw(root["forecasts"][0]["casts"][0]["daytemp"].asString()),
									  GetWeather(
									  u8tw(root["forecasts"][0]["casts"][0]["dayweather"].asString()),
									  u8tw(root["forecasts"][0]["casts"][0]["nightweather"].asString())
									  ),
									  u8tw(root["forecasts"][0]["casts"][1]["nighttemp"].asString()),
									  u8tw(root["forecasts"][0]["casts"][1]["daytemp"].asString()),
									  GetWeather(
									  u8tw(root["forecasts"][0]["casts"][1]["dayweather"].asString()),
									  u8tw(root["forecasts"][0]["casts"][1]["nightweather"].asString())
				)
				);
				*isSuccess = true;
			}
		}
	}
	catch (std::exception &e)
	{
		weather = L"error:" + u8tw(e.what());
		status = std::future_status::deferred;
	}
	catch (...)
	{
		weather = L"unknown error";
	}

	return weather;
}
std::wstring TimeTable::GetWeather(const std::wstring& first, const std::wstring& second)
{
	if (first == second) {
		return first;
	}
	else
	{
		return std::format(L"{}转{}", first, second);
	}
}
std::wstring TimeTable::GetAdcodeSync(const std::wstring& address, const std::wstring& city, const std::wstring& APIKey)
{
	//using namespace Json;
	auto temp = GetResponseFromUrlSync(std::format(L"https://restapi.amap.com/v3/geocode/geo?address={}&city={}&key={}", address, city, APIKey));
	Json::Value value = Json::GetJsonValueFromStr(temp.get()->Body());
	if (u8tw(value["count"].asString()) != L"1") {
		MessageBoxW(0, L"不能准确确定地理编码，请修改地址", L"warning", MB_OK);
	}
	return u8tw(value["geocodes"][0]["adcode"].asString());
}
int TimeTable::deleteLesson(size_t index, const std::wstring &day)
{
	return deleteLesson(index, day, mLessonInfoPath);
}
requests::Response TimeTable::GetResponseFromUrlSync(const std::wstring& url)
{
	requests::Response response;
	int count{ 0 };
	do
	{
		response = requests::get(wtu8(url).c_str());
		count++;
		if (count > 5) {
			throw std::exception("Time out!");
		}
	} while (response.get() == nullptr);
	return response;
}
// 获取当前时间至指定时间的倒计时
std::wstring TimeTable::GetCountDown(tm tmIn, const std::wstring &TimeFormat, int begin)
{
	time_t timeIn{mktime(&tmIn)};
	time_t timeCurrent;
	time(&timeCurrent);
	timeIn -= timeCurrent;
	if (timeIn < 0)
	{
		timeIn = 0;
	}
	gmtime_s(&tmIn, &timeIn);
	wchar_t tmp[1024];
	int times[10]{0};
	times[0] = tmIn.tm_year - 70;
	times[1] = tmIn.tm_yday;
	times[2] = tmIn.tm_hour;
	times[3] = tmIn.tm_min;
	times[4] = tmIn.tm_sec;
	if (begin > 0) {
		times[1] += times[0] * 265;
	}
	if (begin > 1) {
		times[2] += times[1] * 24;
	}
	if (begin > 2) {
		times[3] += times[2] * 60;
	}
	if (begin > 3) {
		times[4] += times[3] * 60;
	}
	_snwprintf_s(tmp, sizeof(tmp) / sizeof(wchar_t), TimeFormat.c_str(), times[begin], times[begin + 1], times[begin + 2], times[begin + 3], times[begin + 4]);
	return tmp;
}

std::wstring TimeTable::GetInfo(bool next)
{
	return GetInfo(GetCurrentTime(L"%a"), next);
}

std::wstring TimeTable::GetInfo(const std::wstring &week, bool next)
{
	std::vector<std::wstring> Infos;
	Infos.clear();
	GetTodayMoreInfo(Infos, week);
	if (Infos.empty())
	{
		return L"";
	}
	if (next) {
		m_InfoCount++;
		m_InfoCount = (m_InfoCount % Infos.size());
	}
	return Infos[m_InfoCount];
}

int TimeTable::GetLesson(std::vector<std::wstring> &input, const std::wstring &week)
{
	std::vector<Lesson> lessons;
	GetLesson(lessons, week);
	for (const auto &i : lessons)
	{
		input.push_back(i.GetValue(L"\t"));
	}
	return 1;
}

int TimeTable::GetLesson(std::vector<Lesson> &input, const std::wstring &week)
{
	Json::Value root = Json::GetRootJsonValue(mLessonInfoPath);
	if (root.isNull() || root[wtu8(week)].isNull())
	{
		input.push_back(Lesson(L"null", 0, 2400));
		return 0;
	}
	const Json::Value Lessons = root[wtu8(week)]["Lessons"];
	for (unsigned int i = 0; i < Lessons.size(); ++i)
	{
		Lesson lesson{week, u8tw(Lessons[i][0].asString()), atoi(Lessons[i][1].asString().c_str()), atoi(Lessons[i][2].asString().c_str())};
		input.push_back(lesson);
	}
	return 1;
}
