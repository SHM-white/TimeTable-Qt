﻿#pragma once

#include "include.h"

//Lesson类，用来保存课程信息，包括名称，开始时间，结束时间
//数字1-7对应星期1-7
//默认存储时间为HHMM格式
class Lesson {
public:
	Lesson(const std::wstring& day, const std::wstring& Name, const std::string& begin, const std::string& end);
	Lesson(const std::wstring& day, const Json::Value& value);
	Lesson(const std::wstring& Day, const std::wstring& Name, int begin, int end);
	//Lesson(int Day, const std::wstring& Name, int begin, int end);
	Lesson(Days day, const std::wstring& Name, int begin, int end);
	Lesson(const std::wstring& Name, int begin, int end);
	Lesson(Lesson& another) = default;
	Lesson(Lesson&& another) = default;
	Lesson(const Lesson& another) = default;
	Lesson() = default;
	const std::wstring& mGetDay() const { return sDay; }
	const std::wstring& mGetName() const { return sName; }
	int GetBeginTime() const { return iBeginTime; }
	int GetEndTime() const { return iEndTime; }
	const std::wstring GetBeginTimeAsString() const;
	const std::wstring GetEndTimeAsString() const;
	int SetValue(const std::wstring& Day, const std::wstring& s, int b, int e);
	int SetValue(int Day, const std::wstring& s, int b, int e);
	int SetValue(const std::wstring& s, int b, int e);
	const std::wstring GetValue(const std::wstring& seprator=L"\t") const;
	const Json::Value GetJsonValue() const;
	std::strong_ordering operator<=>(const Lesson& another) const;
	bool operator<(const Lesson& another) const;
	bool operator==(const Lesson& another) const = default;
	Lesson& operator=(const Lesson& another);
	int GetBeginMin() const;
	int GetEndMin() const;
	bool IsOnLesson(int timeMin) const;
	std::wstring GetCountDown() const;
	static int getHourFromHHmm(int input);
	static int getMinFromHHmm(int input);
	static int mHHMMToMin(int input);
	static int mMinToHHMM(int input);
private:
	int iBeginTime{0};
	int iEndTime{0};
	std::wstring sName{L""};
	std::wstring sDay{L""};
};