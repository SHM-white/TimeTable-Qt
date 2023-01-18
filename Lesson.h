#pragma once

#include<string>
#include<string_view>
#include<format>
#include<vector>
#include "resource.h"
#include<Windows.h>

//Lesson类，用来保存课程信息，包括名称，开始时间，结束时间
class Lesson {
public:
	Lesson(std::string s, int b, int e) :sName{ s }, iBeginTime{ b }, iEndTime{ e } {}
	Lesson(Lesson& OtherLesson)
	{
		iBeginTime = OtherLesson.iBeginTime;
		iEndTime = OtherLesson.iEndTime;
		sName = OtherLesson.sName;
	}
	Lesson() {}
	std::string mGetName() { return sName; }
	int mGetBeginTime() { return iBeginTime; }
	int mGetEndTime() { return iEndTime; }
	int mSetValue(const std::string& s, int b, int e) {
		iBeginTime = b;
		iEndTime = e;
		sName = s;
		return 0;
	}
private:
	int iBeginTime{};
	int iEndTime{};
	std::string sName{};
};