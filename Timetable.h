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
	int mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd,const std::string& TargetFilePath);
	int mAddLesson(const std::string& week, const std::string& Lesson, const std::string& sBegin, const std::string& sEnd);
	int mAddMoreInfo(const std::string& week, const std::string& Info);
	int mGetLesson(std::vector<std::string>& input);
	int mGetLesson(std::vector<std::string>& input, const std::string& week);
	int mGetTodayMoreInfo(std::vector<std::string>& input);
	int mGetTodayMoreInfo(std::vector<std::string>& input,const std::string week);
	int mImportLessonsFromCsv(const std::string& path,const std::string& TargetFileName);
    std::string mGetCountDown(tm tmIn, const std::string& TimeFormat);
	std::string mGetCurrentLesson(std::string& LessonNull);
	std::string mGetCurrentTime(const std::string& TextFormat);
	int mGetCurrentTime(tm& tmTime);
	const std::string& mGetLessonInfoPath();
private:
	Lesson CurrentLesson;
	std::string mLessonInfoPath{ "Lessons.json" };
	int mTimeToMin(int input);
	std::string mReplacePath(const std::string& Path);
};





