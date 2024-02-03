#include "SingleItemUIElementBase.h"

SingleItemUIElementBase::SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
	using namespace std::literals::chrono_literals;
	SetChanged();
	m_type = UIElementType::SingleItem;
	m_CanChangeAfterTime = 5s;
}

bool SingleItemUIElementBase::SetChanged()
{
	m_lastChangedTime = std::chrono::steady_clock::now();
	m_CanChange = false;
	return false;
}
