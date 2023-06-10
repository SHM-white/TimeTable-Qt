#pragma once

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
	TimeTable() = delete;
	TimeTable(std::string path) : mLessonInfoPath{ path } {};
	int mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd, const std::string& TargetFilePath);
	int mAddLesson(const Lesson& lesson);
	int mAddLesson(const Lesson& lesson, const std::string& TargetFilePath);
	int mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd);
	int mAddMoreInfo(const std::string& week, const std::string& Info);
	int mReloadLesson();
	int mGetLesson(std::vector<std::string>& input);
	int mGetLesson(std::vector<std::string>& input, const std::string& week);
	int mGetLesson(std::vector<Lesson>& input, const std::string& week);
	int mGetTodayMoreInfo(std::vector<std::string>& input);
	int mGetTodayMoreInfo(std::vector<std::string>& input, const std::string week);
	int mImportLessonsFromCsv(const std::string& path, const std::string& TargetFileName);
	int sortLessons(const std::string& Day = "All");
	int sortLessons(const std::string& lessonpath, const std::string& Day);
	int deleteLesson(size_t index, const std::string& day);
	int deleteLesson(size_t index, const std::string& day,const std::string& lessonPath);
	int deleteInfo(size_t index, const std::string& day);
	int deleteInfo(size_t index, const std::string& day, const std::string& lessonPath);
	int changeLesson(size_t index, const std::string& day,const Lesson& lesson);
	int changeLesson(size_t index, const std::string& day, const Lesson& lesson, const std::string& lessonPath);
	int SaveJson(const std::string& TargetPath, const Json::Value& root);
	std::string mGetCountDown(tm tmIn, const std::string& TimeFormat);
	std::string mGetInfo();
	std::string mGetInfo(const std::string& week);
	std::string mGetCurrentLesson(const std::string& LessonNull);
	std::string mReplacePath(const std::string& Path);
	Lesson mGetLesson(const std::string& week, int index);
	Json::Value GetRootJsonValue(const std::string& TargetPath);
	static std::string mGetCurrentTime(const std::string& TextFormat);
	static int mGetCurrentTime(tm& tmTime);
	const std::string& mGetLessonInfoPath();
	static int mHHMMToMin(int input);
	static int mMinToHHMM(int input);
	const std::string Days[8]{ "Null","Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
private:
	Lesson CurrentLesson;
	std::vector<Lesson> Lessons;
	std::string mLessonInfoPath{ "Lessons.json" };
};





