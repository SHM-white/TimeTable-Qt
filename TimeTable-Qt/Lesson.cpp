#include "Lesson.h"
#include "include.h"
#include "Timetable.h"

Lesson::Lesson(const std::wstring& day, const std::wstring& Name, const std::string& begin, const std::string& end)
{
	sDay = day;
	sName = Name;
	iBeginTime = atoi(begin.c_str());
	iEndTime = atoi(end.c_str());
}

Lesson::Lesson(const std::wstring& day, const Json::Value& value)
{
	sDay = day;
	sName = u8tw(value[0].asString());
	iBeginTime = atoi(value[1].asString().c_str());
	iEndTime = atoi(value[2].asString().c_str());
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

std::strong_ordering Lesson::operator<=>(const Lesson& another) const
{
	auto result{ this->mGetName() <=> another.mGetName() };
	if (result == std::strong_ordering::equivalent) {
		result = this->GetBeginTime() <=> another.GetBeginTime();
	}
	return result;
}

Lesson& Lesson::operator=(const Lesson& another)
{
	SetValue(another.sDay, another.mGetName(), another.GetBeginTime(), another.GetEndTime());
	return *this;
}

int Lesson::GetBeginMin() const
{
	return mHHMMToMin(iBeginTime);
}

int Lesson::GetEndMin() const
{
	return mHHMMToMin(iEndTime);
}

bool Lesson::IsOnLesson(int timeMin) const
{
	return (timeMin > GetBeginMin()) && (timeMin < GetEndMin());
}

std::wstring Lesson::GetCountDown() const
{
	tm time;
	TimeTable::GetCurrentTime(time);
	tm lessonTime = time;
	lessonTime.tm_hour = getHourFromHHmm(GetBeginTime());
	lessonTime.tm_min = getMinFromHHmm(GetBeginTime());
	lessonTime.tm_sec = 0;
	return TimeTable::GetCountDown(lessonTime, std::format(L"距离下一节课{}还有%d分钟%d秒", sName), 2);
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

