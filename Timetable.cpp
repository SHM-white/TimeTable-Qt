
#include "Timetable.h"
#include <Windows.h>
#include <fstream>
#include <vector>
#include <strsafe.h>
#include "include\json\json.h"
#include<format>
#include <algorithm>
#include <ranges>
#include "CSVEditor.h"

//TimeTable类的实现函数
//添加课程的4参数重载函数，兼容旧代码（其实就是懒），默认写入成员变量保存的路径
int TimeTable::mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd)
{
	return mAddLesson(week, Lesson, sBegin, sEnd, mLessonInfoPath);
}
//添加课程5参数函数
int TimeTable::mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd, const std::string& TargetFilePath)
{
	if (!((bool)week.size() && (bool)Lesson.size() && (bool)sBegin.size() && (bool)sEnd.size() && (bool)TargetFilePath.size())) {
		return 0;
	}
	Json::Reader reader;
	Json::Value root;
	Json::StyledWriter sw;
	std::fstream os;
	Json::Value Current;
	os.open(TargetFilePath, std::ios::in);
	if (!os.is_open()) {
		os.close();
		os.clear();
		return 0;
	}
	if (!reader.parse(os, root)) {
		os.close();
		os.clear();
		return 0;
	}
	os.close();
	os.clear();
	os.open(TargetFilePath, std::ios::out | std::ios::trunc);
	Current.append(Lesson);
	Current.append(sBegin);
	Current.append(sEnd);
	root[week]["Lessons"].append(Current);
	os.seekp(std::ios::beg);
	os << sw.write(root);
	os.close();
	os.clear();
	return 1;
}
//添加更多信息，有空也加个重载
int TimeTable::mAddMoreInfo(const std::string& Days, const std::string& Info)
{
	std::fstream os;
	os.open(mLessonInfoPath, std::ios::out | std::ios::in);
	if (!os.is_open()) {
		return 0;
	}
	Json::Reader reader;
	Json::Value root;
	Json::StyledWriter sw;
	reader.parse(os, root);
	root[Days]["Infos"].append(Info);
	os.seekp(std::ios::beg);
	os << sw.write(root);
	os.close();
	os.clear();
	return 0;
}
//转换时间为分钟数便于比较大小
int TimeTable::mTimeToMin(int input)
{
	return (input - input % 100) / 100 * 60 + input % 100;
}
//暂时替换文件路径，同时返回旧的文件路径，只有内部函数能调用
std::string TimeTable::mReplacePath(const std::string& Path)
{
	std::string old = mLessonInfoPath;
	mLessonInfoPath = Path;
	return old;
}
//获取所有的课程并返回至传入的数组
int TimeTable::mGetLesson(std::vector<std::string>& input)
{
	input.clear();
	std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	for (const auto& a : Days) {
		mGetLesson(input, a);
	}
	return 1;
}
//获取今日的信息并存入传入的数组
int TimeTable::mGetTodayMoreInfo(std::vector<std::string>& input)
{
	std::string week{ mGetCurrentTime("%a") };
	return mGetTodayMoreInfo(input, week);
}
int TimeTable::mGetTodayMoreInfo(std::vector<std::string>& input, const std::string week)
{
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (reader.parse(in, root)) {
		const Json::Value Infos = root[week]["Infos"];
		for (unsigned int i = 0; i < Infos.size(); ++i) {
			input.push_back(Infos[i].asString());
		}
	}
	in.close();
	return 1;
}
//获取当前时间的课程
std::string TimeTable::mGetCurrentLesson(const std::string& LessonNull)
{
	std::string timeCurrentTime{ mGetCurrentTime("%H%M") };
	int iCurrentTime = mTimeToMin(atoi(timeCurrentTime.c_str()));
	if (!((CurrentLesson.mGetBeginTime() <= iCurrentTime) && (iCurrentTime <= CurrentLesson.mGetEndTime()))) {
		Json::Reader reader;
		Json::Value root;
		std::ifstream in(mLessonInfoPath, std::ios::in);
		if (!in.is_open())
		{
			return std::string("Can't find ") + mLessonInfoPath;
		};
		if (reader.parse(in, root)) {
			std::string week{ mGetCurrentTime("%a") };
			const Json::Value Lessons = root[week]["Lessons"];
			for (unsigned int i = 0; i < Lessons.size(); ++i) {
				std::string sBeginTime = Lessons[i][1].asString();
				std::string sEndTime = Lessons[i][2].asString();
				int iBeginTime = mTimeToMin(atoi(sBeginTime.c_str()));
				int iEndTime = mTimeToMin(atoi(sEndTime.c_str()));
				if ((iBeginTime <= iCurrentTime) && (iEndTime >= iCurrentTime)) {
					in.close();
					CurrentLesson.mSetValue(Lessons[i][0].asString(), iBeginTime, iEndTime);
					return CurrentLesson.mGetName();
				}
			}
			CurrentLesson.mSetValue(LessonNull, 0, 0);
		}
		in.close();
		in.clear();
	}
	return CurrentLesson.mGetName();
}

std::string TimeTable::mGetCurrentTime(const std::string& TextFormat)
{
	char tmp[256];
	tm structm;
	mGetCurrentTime(structm);
	strftime(tmp, sizeof(tmp), TextFormat.c_str(), &structm);
	return std::string(tmp);
}
//从csv导入课程至指定文件
int TimeTable::mImportLessonsFromCsv(const std::string& path, const std::string& TargetFileName)
{
	CSVEditor CsvEditor{ path };
	int a{ 0 };
	if (CsvEditor.mGetCsvData()) {
		for (int i{ 0 }; i < CsvEditor.mGetLineCount(); i++) {
			a = mAddLesson(CsvEditor[i][0], CsvEditor[i][1], CsvEditor[i][2], CsvEditor[i][3], TargetFileName);
		}
		return a;
	}
	return 0;
}
//排序
int TimeTable::sort()
{
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	if (!reader.parse(in, root)) { 
		in.close();
		return 0; 
	}
	std::fstream os(mLessonInfoPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }
	for (const auto& i : Days) {
		if (i == "Null") { continue; }
		Json::Value& valueLessons = root[i]["Lessons"];
		std::vector<Lesson> vectorLessons;
		for (const auto& value : valueLessons) {
			vectorLessons.push_back(Lesson(i, value[0].asString(), value[1].asInt(), value[2].asInt()));
		}
		//auto ILessons = Lessons | std::views::transform([i](const Json::Value& value) {return Lesson(i, value[0].asString(), value[1].asInt(), value[2].asInt()); });
		std::sort(vectorLessons.begin(),vectorLessons.end());
		valueLessons.clear();
		for (const auto& lesson : vectorLessons) {
			valueLessons.append(Json::Value().append(lesson.mGetName()).append(lesson.mGetBeginTime()).append(lesson.mGetEndTime()));
		}
	}
	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	return 1;
}
//获取课程信息地址
const std::string& TimeTable::mGetLessonInfoPath()
{
	return mLessonInfoPath;
}
//获取当前时间并存入传入的结构中
int TimeTable::mGetCurrentTime(tm& tmTime)
{
	time_t timep;
	time(&timep);
	localtime_s(&tmTime, &timep);
	return 0;
}
//获取当前时间至指定时间的倒计时
std::string TimeTable::mGetCountDown(tm tmIn, const std::string& TimeFormat)
{
	time_t timeIn{ mktime(&tmIn) };
	time_t timeCurrent;
	time(&timeCurrent);
	timeIn -= timeCurrent;
	if (timeIn < 0) {
		timeIn = 0;
	}
	gmtime_s(&tmIn, &timeIn);
	char tmp[256];
	snprintf(tmp, sizeof(tmp) - 1, TimeFormat.c_str(),  tmIn.tm_yday, tmIn.tm_hour, tmIn.tm_min, tmIn.tm_sec);
	//vsprintf_s(tmp, sizeof(tmp), TimeFormat.c_str(), tmIn.tm_mon, tmIn.tm_mday-1, tmIn.tm_hour, tmIn.tm_min, tmIn.tm_sec);
	return tmp;
}

int TimeTable::mGetLesson(std::vector<std::string>& input, const std::string& week)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(mLessonInfoPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	//std::string Days[]{ "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	if (reader.parse(in, root)) {
		const Json::Value Lessons = root[week]["Lessons"];
		for (unsigned int i = 0; i < Lessons.size(); ++i) {
			std::string result{ week + "\t" + Lessons[i][0].asString() + "\t" + Lessons[i][1].asString() + "\t" + Lessons[i][2].asString() };
			input.push_back(result);
		}
	}
	in.close();
	in.clear();
	return 1;
}