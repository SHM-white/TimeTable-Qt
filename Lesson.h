#pragma once

#include<string>
#include<string_view>
#include<format>
#include<vector>
#include <strsafe.h>
#include "resource.h"
#include<Windows.h>
#include <string_view>
#include <compare>

//Lesson类，用来保存课程信息，包括名称，开始时间，结束时间
class Lesson {
public:
	Lesson(const std::string& Day, const std::string& Name, int begin, int end);
	Lesson(int Day, const std::string& Name, int begin, int end);
	Lesson(const std::string& Name, int begin, int end);
	Lesson() {}
	const std::string& mGetName() const { return sName; }
	int mGetBeginTime() const { return iBeginTime; }
	int mGetEndTime() const { return iEndTime; }
	int mSetValue(const std::string& Day, const std::string& s, int b, int e);
	int mSetValue(int Day, const std::string& s, int b, int e);
	int mSetValue(const std::string& s, int b, int e);
	const std::string GetValue(const std::string& seprator="\t") const;
	auto operator<=>(const Lesson& another) const;
	auto operator==(const Lesson& another) const;
	const std::string Days[8]{ "Null","Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
private:
	int iBeginTime{};
	int iEndTime{};
	std::string sName{};
	std::string sDay{};
};