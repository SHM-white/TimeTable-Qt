#include "Lesson.h"
#include <compare>

auto Lesson::operator<=>(const Lesson& another) const
{
	auto result{ this->mGetName() <=> another.mGetName() };
	if (result == std::strong_ordering::equivalent) {
		result = this->mGetBeginTime() <=> another.mGetBeginTime();
	}
	return result;
}

Lesson::Lesson(const std::string& Day, const std::string& Name, int begin, int end)
{
	mSetValue(Day, Name, begin, end);
}

Lesson::Lesson(int Day, const std::string& Name, int begin, int end)
{
	mSetValue(Days[Day], Name, begin, end);
}

Lesson::Lesson(const std::string& Name, int begin, int end)
{
	mSetValue(Name, begin, end);
}

int Lesson::mSetValue(const std::string& Day, const std::string& s, int b, int e)
{
	iBeginTime = b;
	iEndTime = e;
	sName = s;
	sDay = Day;
	return 0;
}

int Lesson::mSetValue(int Day, const std::string& s, int b, int e)
{
	return mSetValue(Days[Day], s, b, e);
}

int Lesson::mSetValue(const std::string& s, int b, int e)
{
	return mSetValue(Days[0], s, b, e);
}

const std::string Lesson::GetValue(const std::string& seprator) const
{
	auto result{ std::format("{0}{4}{1}{4}{2}{4}{3}",sDay, sName,iBeginTime,iEndTime,seprator) };
	return result;
}

void Lesson::operator=(const Lesson& another)
{
	mSetValue(another.sDay, another.mGetName(), another.mGetBeginTime(), another.mGetEndTime());
}

bool Lesson::operator<(const Lesson& another) const
{
	return this->mGetBeginTime()<another.mGetBeginTime();
}

