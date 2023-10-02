#pragma once

#include "include.h"
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
	Lesson(const std::wstring& Day, const std::wstring& Name, int begin, int end);
	Lesson(int Day, const std::wstring& Name, int begin, int end);
	Lesson(const std::wstring& Name, int begin, int end);
	Lesson(Lesson& another) = default;
	Lesson(Lesson&& another) = default;
	Lesson(const Lesson& another) = default;
	Lesson() {}
	const std::wstring& mGetDay() const { return sDay; }
	const std::wstring& mGetName() const { return sName; }
	int mGetBeginTime() const { return iBeginTime; }
	int mGetEndTime() const { return iEndTime; }
	const std::wstring mGetBeginTimeAsString() const;
	const std::wstring mGetEndTimeAsString() const;
	int mSetValue(const std::wstring& Day, const std::wstring& s, int b, int e);
	int mSetValue(int Day, const std::wstring& s, int b, int e);
	int mSetValue(const std::wstring& s, int b, int e);
	const std::wstring GetValue(const std::wstring& seprator=L"\t") const;
	const Json::Value GetJsonValue() const;
	std::strong_ordering operator<=>(const Lesson& another) const;
	bool operator<(const Lesson& another) const;
	bool operator==(const Lesson& another) const = default;
	Lesson& operator=(const Lesson& another);
	const std::wstring Days[8]{ L"Null",L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat", L"Sun" };
	static int getHourFromHHmm(int input);
	static int getMinFromHHmm(int input);
private:
	int iBeginTime{};
	int iEndTime{};
	std::wstring sName{};
	std::wstring sDay{};
};