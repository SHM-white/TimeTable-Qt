#pragma once

#include "include.h"
#include<string>
#include<string_view>
#include<format>
#include<vector>
#include "resource.h"
#include<Windows.h>
#include"Lesson.h"


//TimeTable类，包含与获取课程有关的函数，需初始化配置文件路径
class TimeTable {
public:
	TimeTable() = default;
	TimeTable(const TimeTable& another) = default;
	TimeTable(TimeTable& another) = default;
	TimeTable(std::wstring path) : mLessonInfoPath{ path } {};
	TimeTable& operator=(const TimeTable& another) = default;
	int mAddLesson(const std::wstring& week, const std::wstring& Lesson, const std::wstring& sBegin, const std::wstring& sEnd, const std::wstring& TargetFilePath);
	int mAddLesson(const Lesson& lesson);
	int mAddLesson(const Lesson& lesson, const std::wstring& TargetFilePath);
	int mAddLesson(const std::wstring& week, const std::wstring& Lesson, const std::wstring& sBegin, const std::wstring& sEnd);
	int mAddMoreInfo(const std::wstring& week, const std::wstring& Info);
	int mReloadLesson();
	int mGetLesson(std::vector<std::wstring>& input);
	int mGetLesson(std::vector<std::wstring>& input, const std::wstring& week);
	int mGetLesson(std::vector<Lesson>& input, const std::wstring& week);
	int mGetTodayMoreInfo(std::vector<std::wstring>& input);
	int mGetTodayMoreInfo(std::vector<std::wstring>& input, const std::wstring week);
	int mImportLessonsFromCsv(const std::wstring& path, const std::wstring& TargetFileName);
	int sortLessons(const std::wstring& Day = L"All");
	int sortLessons(const std::wstring& lessonpath, const std::wstring& Day);
	int deleteLesson(size_t index, const std::wstring& day);
	int deleteLesson(size_t index, const std::wstring& day,const std::wstring& lessonPath);
	int deleteInfo(size_t index, const std::wstring& day);
	int deleteInfo(size_t index, const std::wstring& day, const std::wstring& lessonPath);
	int changeLesson(size_t index, const std::wstring& day,const Lesson& lesson);
	int changeLesson(size_t index, const std::wstring& day, const Lesson& lesson, const std::wstring& lessonPath);
	int changeInfo(size_t index, const std::wstring& day, const std::wstring& info);
	int changeInfo(size_t index, const std::wstring& day, const std::wstring& info, const std::wstring& Path);
	int SaveJson(const std::wstring& TargetPath, const Json::Value& root);
	int mGetCurrentLesson(int);
	std::wstring mGetWeather(int code);
	std::wstring mGetCountDown(tm tmIn, const std::wstring& TimeFormat);
	std::wstring mGetInfo();
	std::wstring mGetInfo(const std::wstring& week);
	std::wstring mGetCurrentLesson(const std::wstring& LessonNull);
	std::wstring mReplacePath(const std::wstring& Path);
	Lesson mGetLesson(const std::wstring& week, int index);
	static std::wstring mGetCurrentTime(const std::wstring& TextFormat);
	static int mGetCurrentTime(tm& tmTime);
	const std::wstring& mGetLessonInfoPath();
	static int mHHMMToMin(int input);
	static int mMinToHHMM(int input);
	const std::wstring Days[8]{ L"Null",L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
private:
	Lesson CurrentLesson;
	std::vector<Lesson> Lessons;
	std::wstring mLessonInfoPath{ L"Lessons.json" };
};





