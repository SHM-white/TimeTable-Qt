﻿#pragma once

#include "include.h"
#include "Lesson.h"


//TimeTable类，包含与获取课程有关的函数，需初始化配置文件路径
class TimeTable {
public:
	TimeTable() = default;
	TimeTable(const TimeTable& another) = default;
	TimeTable(TimeTable& another) = default;
	TimeTable(std::wstring path) : mLessonInfoPath{ path } {};
	TimeTable& operator=(const TimeTable& another) = default;
	int AddLesson(const std::wstring& week, const std::wstring& Lesson, const std::wstring& sBegin, const std::wstring& sEnd, const std::wstring& TargetFilePath);
	int AddLesson(const Lesson& lesson);
	int AddLesson(const Lesson& lesson, const std::wstring& TargetFilePath);
	int AddLesson(const std::wstring& week, const std::wstring& Lesson, const std::wstring& sBegin, const std::wstring& sEnd);
	int AddMoreInfo(const std::wstring& week, const std::wstring& Info);
	int ReloadLesson();
	int GetLesson(std::vector<std::wstring>& input);
	int GetLesson(std::vector<std::wstring>& input, const std::wstring& week);
	int GetLesson(std::vector<Lesson>& input, const std::wstring& week);
	int GetTodayMoreInfo(std::vector<std::wstring>& input);
	int GetTodayMoreInfo(std::vector<std::wstring>& input, const std::wstring week);
	int ImportLessonsFromCsv(const std::wstring& path, const std::wstring& TargetFileName);
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
	int GetCurrentLesson(int);
	static int GetCurrentTimeMin();
	static std::wstring GetWeather(const std::wstring& code,const std::wstring& APIKey,bool* isSuccess);
	static std::wstring GetWeather(const std::wstring& first, const std::wstring& second);
	static std::wstring GetAdcodeSync(const std::wstring& address,const std::wstring& city, const std::wstring& APIKey);
	static requests::Response GetResponseFromUrlSync(const std::wstring& url);
	static std::wstring GetCountDown(tm tmIn, const std::wstring& TimeFormat, int begin = 1);
	std::wstring GetInfo(bool next = false);
	std::wstring GetInfo(const std::wstring& week, bool next = false);
	std::wstring GetCurrentLesson(const std::wstring& LessonNull);
	std::wstring ReplacePath(const std::wstring& Path);
	std::vector<Lesson> GetLessons();
	std::vector<Lesson> GetTodayLessons(const std::wstring& day = L"Null");
	Lesson GetNextLesson();
	Lesson GetCurrentLesson();
	Lesson GetLesson(const std::wstring& week, int index);
	const std::wstring& GetLessonInfoPath();
	static std::wstring GetCurrentTime(const std::wstring& TextFormat);
	static int GetCurrentTime(tm& tmTime);
	static int mHHMMToMin(int input);
	static int mMinToHHMM(int input);
private:
	Lesson CurrentLesson;
	//std::vector<Lesson> Lessons;
	std::wstring mLessonInfoPath{ L"Lessons.json" };
	bool m_needUpdateWeather{true};
	int m_InfoCount{ 0 };
};





