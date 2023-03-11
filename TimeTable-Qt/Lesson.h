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
#include "include/json/json.h"

//Lesson类，用来保存课程信息，包括名称，开始时间，结束时间
//数字1-7对应星期1-7
class Lesson {
public:
	Lesson(const std::string& Day, const std::string& Name, int begin, int end);
	Lesson(int Day, const std::string& Name, int begin, int end);
	Lesson(const std::string& Name, int begin, int end);
	Lesson(Lesson& another) = default;
	Lesson(Lesson&& another) = default;
	Lesson() {}
	const std::string& mGetDay() const { return sDay; }
	const std::string& mGetName() const { return sName; }
	int mGetBeginTime() const { return iBeginTime; }
	const std::string mGetBeginTimeAsString() const;
	const std::string mGetEndTimeAsString() const;
	int mGetEndTime() const { return iEndTime; }
	int mSetValue(const std::string& Day, const std::string& s, int b, int e);
	int mSetValue(int Day, const std::string& s, int b, int e);
	int mSetValue(const std::string& s, int b, int e);
	const std::string GetValue(const std::string& seprator="\t") const;
	const Json::Value GetJsonValue() const;
	std::strong_ordering operator<=>(const Lesson& another) const;
	bool operator<(const Lesson& another) const;
	bool operator==(const Lesson& another) const = default;
	void operator=(const Lesson& another);
	const std::string Days[8]{ "Null","Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
private:
	int iBeginTime{};
	int iEndTime{};
	std::string sName{};
	std::string sDay{};
};