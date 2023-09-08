#include "Lesson.h"
#include "include.h"
#include <compare>
#include <format>

std::strong_ordering Lesson::operator<=>(const Lesson& another) const
{
	auto result{ this->mGetName() <=> another.mGetName() };
	if (result == std::strong_ordering::equivalent) {
		result = this->mGetBeginTime() <=> another.mGetBeginTime();
	}
	return result;
}

Lesson::Lesson(const std::wstring& Day, const std::wstring& Name, int begin, int end)
{
	mSetValue(Day, Name, begin, end);
}

Lesson::Lesson(int Day, const std::wstring& Name, int begin, int end)
{
	mSetValue(Days[Day], Name, begin, end);
}

Lesson::Lesson(const std::wstring& Name, int begin, int end)
{
	mSetValue(Name, begin, end);
}

const std::wstring Lesson::mGetBeginTimeAsString() const
{
	return std::to_wstring(iBeginTime);
}

const std::wstring Lesson::mGetEndTimeAsString() const
{
	return std::to_wstring(iEndTime);
}

int Lesson::mSetValue(const std::wstring& Day, const std::wstring& s, int b, int e)
{
	iBeginTime = b;
	iEndTime = e;
	sName = s;
	sDay = Day;
	return 0;
}

int Lesson::mSetValue(int Day, const std::wstring& s, int b, int e)
{
	return mSetValue(Days[Day], s, b, e);
}

int Lesson::mSetValue(const std::wstring& s, int b, int e)
{
	return mSetValue(Days[0], s, b, e);
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
	mSetValue(another.sDay, another.mGetName(), another.mGetBeginTime(), another.mGetEndTime());
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
	return this->mGetBeginTime()<another.mGetBeginTime();
}

