#include "include.h"
#include "Timetable.h"
#include "include\json\json.h"
#include "CSVEditor.h"
#include "Lesson.h"

// This function is an overloaded version of the mAddLesson function in the TimeTable class.
// It adds a lesson with four parameters: week, Lesson, sBegin, sEnd.
// If the fifth parameter, mLessonInfoPath, is not provided, it defaults to the value of the member variable mLessonInfoPath.
// Returns the result of calling the mAddLesson function with the provided parameters.
int TimeTable::mAddLesson(const std::wstring &week, const std::wstring &Lesson, const std::wstring &sBegin, const std::wstring &sEnd)
{
	// Call the mAddLesson function with the provided parameters and the default mLessonInfoPath.
	return mAddLesson(week, Lesson, sBegin, sEnd, mLessonInfoPath);
}

// 添加课程5参数函数
int TimeTable::mAddLesson(const std::wstring &week, const std::wstring &Lesson, const std::wstring &sBegin, const std::wstring &sEnd, const std::wstring &TargetFilePath)
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
int TimeTable::mAddLesson(const Lesson &lesson)
{
	return mAddLesson(lesson, mLessonInfoPath);
}
int TimeTable::mAddLesson(const Lesson &lesson, const std::wstring &TargetFilePath)
{
	return mAddLesson(lesson.mGetDay(), lesson.mGetName(), lesson.mGetBeginTimeAsString(), lesson.mGetEndTimeAsString(), TargetFilePath);
}
// 添加更多信息，有空也加个重载
int TimeTable::mAddMoreInfo(const std::wstring &Days, const std::wstring &Info)
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
int TimeTable::mReloadLesson()
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
std::wstring TimeTable::mReplacePath(const std::wstring &Path)
{
	std::wstring old = mLessonInfoPath;
	Json::ChangeValue(L"LessonInfoFile", wtu8(Path), DEFAULT_CONFIG_PATH);
	mLessonInfoPath = Path;
	return old;
}
Lesson TimeTable::mGetLesson(const std::wstring &week, int index)
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
int TimeTable::mGetLesson(std::vector<std::wstring> &input)
{
	input.clear();
	std::wstring Days[]{L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun"};
	for (const auto &a : Days)
	{
		mGetLesson(input, a);
	}
	return 1;
}
// 获取今日的信息并存入传入的数组
int TimeTable::mGetTodayMoreInfo(std::vector<std::wstring> &input)
{
	std::wstring week{mGetCurrentTime(L"%a")};
	return mGetTodayMoreInfo(input, week);
}
int TimeTable::mGetTodayMoreInfo(std::vector<std::wstring> &input, const std::wstring week)
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
std::wstring TimeTable::mGetCurrentLesson(const std::wstring &LessonNull)
{
	int lessonIndex = mGetCurrentLesson(0);
	if (lessonIndex < 0)
	{
		return LessonNull;
	}
	return CurrentLesson.mGetName();
}

std::wstring TimeTable::mGetCurrentTime(const std::wstring &TextFormat)
{
    wchar_t result[1024];
	tm structm;

	mGetCurrentTime(structm);
	auto location = TextFormat.find(L'%', 0);
	if (location >= TextFormat.size())
	{
		location = 0;
	}
    if (TextFormat.size() >= 2 && !std::regex_match(TextFormat.begin() + location, TextFormat.begin() + location + 2, std::regex("%[^aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%]|%[0-9]{1,}[a-zA-Z]")))
	{
        wcsftime(result, sizeof(result)/2, TextFormat.c_str(), &structm);
	}
	else
	{
		return TextFormat;
	}
	return std::wstring(result);
}
// 从csv导入课程至指定文件
int TimeTable::mImportLessonsFromCsv(const std::wstring &path, const std::wstring &TargetFileName)
{
	CSVEditor CsvEditor{path};
	int a{0};
	if (CsvEditor.mGetCsvData())
	{
		for (int i{1}; i < CsvEditor.mGetLineCount(); i++)
		{ // jump line 1
			a = mAddLesson(CsvEditor[i][0], CsvEditor[i][1], CsvEditor[i][2], CsvEditor[i][3], TargetFileName);
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
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		return 0;
	}

	std::vector<Lesson> vectorLessons;

	for (const auto &i : Days)
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
				  { return lesson1.mGetBeginTime() < lesson2.mGetBeginTime(); });
		valueLessons.clear();
		valueLessons = Json::Value(Json::arrayValue);
		for (const auto &lesson : vectorLessons)
		{
			Json::Value a = lesson.GetJsonValue();
			valueLessons.append(a);
		}
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
const std::wstring &TimeTable::mGetLessonInfoPath()
{
	return mLessonInfoPath;
}
int TimeTable::mMinToHHMM(int input)
{
	int Hour = (input - input % 60) / 60;
	return Hour * 100 + input % 60;
}
// 获取当前时间并存入传入的结构中
int TimeTable::mGetCurrentTime(tm &tmTime)
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
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
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

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	mReloadLesson();
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
	std::fstream os(lessonPath, std::ios::out | std::ios::trunc);
	if (!os.is_open())
	{
		return 0;
	}

	Json::Value &valueLessons = root[wtu8(day)]["Lessons"];
	valueLessons[(int)index] = lesson.GetJsonValue();

	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	mReloadLesson();
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
	mReloadLesson();
	return result;
}
/**
 * Returns the index of the current lesson based on the current time.
 * @param None
 * @return The index of the current lesson
 */
int TimeTable::mGetCurrentLesson(int)
{
	// Initialize currentIndex to 0
	static int currentIndex{0};

	// Get the current time in the format HHMM
	std::wstring timeCurrentTime{mGetCurrentTime(L"%H%M")};

	// Convert the current time to minutes
	int iCurrentTime = mHHMMToMin(_wtoi(timeCurrentTime.c_str()));

	// Check if the current time is within the current lesson's time range
	if (!((CurrentLesson.mGetBeginTime() <= iCurrentTime) && (iCurrentTime <= CurrentLesson.mGetEndTime())))
	{
		// Get the lesson information from the lesson info JSON file
		Json::Value root = Json::GetRootJsonValue(mLessonInfoPath);

		// Get the current day of the week
		std::wstring week{mGetCurrentTime(L"%a")};

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
				CurrentLesson.mSetValue(u8tw(Lessons[i][0].asString()), iBeginTime, iEndTime);
				currentIndex = i;
				return currentIndex;
			}
		}
		currentIndex = -1;
	}

	// Return the index of the current lesson
	return currentIndex;
}
std::wstring TimeTable::mGetWeather(int code)
{
	static std::wstring weather{L"getting...."};
	static std::future<requests::Response> result;
	static std::future_status status = std::future_status::deferred;
	try
	{
		if ((status == std::future_status::deferred) && m_needUpdateWeather)
		{
			result = std::async(
				std::launch::async,
				[code]()
				{
					requests::Response response;
					do
					{
						response = requests::get(std::format("https://restapi.amap.com/v3/weather/weatherInfo?city={}&key=7654cff2801031c93fba40fe770e7016&extensions=all", code).c_str());
					} while (response.get() == nullptr);
					return response;
				});
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
				weather = std::format(L"今日天气: {}-{}℃,{},明日天气：{}-{}℃,{}",
									  u8tw(root["forecasts"][0]["casts"][0]["nighttemp"].asString()),
									  u8tw(root["forecasts"][0]["casts"][0]["daytemp"].asString()),
									  mGetWeather(
									  u8tw(root["forecasts"][0]["casts"][0]["dayweather"].asString()),
									  u8tw(root["forecasts"][0]["casts"][0]["nightweather"].asString())
									  ),
									  u8tw(root["forecasts"][0]["casts"][1]["nighttemp"].asString()),
									  u8tw(root["forecasts"][0]["casts"][1]["daytemp"].asString()),
									  mGetWeather(
									  u8tw(root["forecasts"][0]["casts"][1]["dayweather"].asString()),
									  u8tw(root["forecasts"][0]["casts"][1]["nightweather"].asString())
				)
				);
			}
		}
	}
	catch (std::exception &e)
	{
		weather = L"error:" + u8tw(e.what());
	}
	catch (...)
	{
		weather = L"unknown error";
	}

	return weather;
}
std::wstring TimeTable::mGetWeather(const std::wstring& first, const std::wstring& second)
{
	if (first == second) {
		return first;
	}
	else
	{
		return std::format(L"{}转{}", first, second);
	}
}
int TimeTable::deleteLesson(size_t index, const std::wstring &day)
{
	return deleteLesson(index, day, mLessonInfoPath);
}
// 获取当前时间至指定时间的倒计时
std::wstring TimeTable::mGetCountDown(tm tmIn, const std::wstring &TimeFormat)
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
	wchar_t tmp[256];
	_snwprintf(tmp, sizeof(tmp) - 1, TimeFormat.c_str(), tmIn.tm_yday, tmIn.tm_hour, tmIn.tm_min, tmIn.tm_sec);
	return tmp;
}

std::wstring TimeTable::mGetInfo()
{
	return mGetInfo(mGetCurrentTime(L"%a"));
}

std::wstring TimeTable::mGetInfo(const std::wstring &week)
{
	std::vector<std::wstring> Infos;
	Infos.clear();
	mGetTodayMoreInfo(Infos, week);
	static int count{0};
	static int currentItem{0};
	const int changeAfterTimes{19};
	if (Infos.empty())
	{
		return L"";
	}
	if (currentItem >= Infos.size() - 1)
	{
		currentItem = 0;
	}
	else if (count >= changeAfterTimes)
	{
		++currentItem;
		count = 0;
	}
	else
	{
		++count;
	}
	return Infos[currentItem];
}

int TimeTable::mGetLesson(std::vector<std::wstring> &input, const std::wstring &week)
{
	std::vector<Lesson> lessons;
	mGetLesson(lessons, week);
	for (const auto &i : lessons)
	{
		input.push_back(i.GetValue(L"\t"));
	}
	return 1;
}

int TimeTable::mGetLesson(std::vector<Lesson> &input, const std::wstring &week)
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
