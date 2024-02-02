#include "Lesson.h"
#include "include.h"
#include <compare>
#include <format>

std::strong_ordering Lesson::operator<=>(const Lesson& another) const
{
	auto result{ this->mGetName() <=> another.mGetName() };
	if (result == std::strong_ordering::equivalent) {
		result = this->GetBeginTime() <=> another.GetBeginTime();
	}
	return result;
}

Lesson::Lesson(const std::wstring& Day, const std::wstring& Name, int begin, int end)
{
	SetValue(Day, Name, begin, end);
}

//Lesson::Lesson(int Day, const std::wstring& Name, int begin, int end)
//{
//	mSetValue(Days[Day], Name, begin, end);
//}

Lesson::Lesson(Days day, const std::wstring& Name, int begin, int end)
{
	SetValue(GetWStrDay(day), Name, begin, end);
}

Lesson::Lesson(const std::wstring& Name, int begin, int end)
{
	SetValue(Name, begin, end);
}

const std::wstring Lesson::GetBeginTimeAsString() const
{
	return std::to_wstring(iBeginTime);
}

const std::wstring Lesson::GetEndTimeAsString() const
{
	return std::to_wstring(iEndTime);
}

int Lesson::SetValue(const std::wstring& Day, const std::wstring& s, int b, int e)
{
	iBeginTime = b;
	iEndTime = e;
	sName = s;
	sDay = Day;
	return 0;
}

int Lesson::SetValue(int Day, const std::wstring& s, int b, int e)
{
	return SetValue(GetWStrDay(Days(Day)), s, b, e);
}

int Lesson::SetValue(const std::wstring& s, int b, int e)
{
	return SetValue(GetWStrDay(Days::Null), s, b, e);
}

const std::wstring Lesson::GetValue(const std::wstring& seprator) const
{
	auto result{ std::format(L"{0}{4}{1}{4}{2}{4}{3}",sDay, sName,iBeginTime,iEndTime,seprator) };
	return result;
}

const Json::Value Lesson::GetJsonValue() const
{
	Json::Value result;
	result.append(wtu8(sName));
	result.append(wtu8(std::format(L"{:04}", iBeginTime)));
	result.append(wtu8(std::format(L"{:04}", iEndTime)));
	return result;
}

Lesson& Lesson::operator=(const Lesson& another)
{
	SetValue(another.sDay, another.mGetName(), another.GetBeginTime(), another.GetEndTime());
	return *this;
}

int Lesson::getHourFromHHmm(int input)
{
	return (input-input%100)/100;
}

int Lesson::getMinFromHHmm(int input)
{
	return input%100;
}

bool Lesson::operator<(const Lesson& another) const
{
	return this->GetBeginTime()<another.GetBeginTime();
}
// 转换时间为分钟数便于比较大小
int Lesson::mHHMMToMin(int input)
{
	return (input - input % 100) / 100 * 60 + input % 100;
}
int Lesson::mMinToHHMM(int input)
{
	int Hour = (input - input % 60) / 60;
	return Hour * 100 + input % 60;
}

