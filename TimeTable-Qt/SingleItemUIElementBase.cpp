#include "SingleItemUIElementBase.h"

SingleItemUIElementBase::SingleItemUIElementBase(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:UIElementBase(value, timetable)
{
	using namespace std::literals::chrono_literals;
	SetChanged();
	m_type = UIElementType::SingleItem;
	m_CanChangeAfterTime = 5000ms;
}

bool SingleItemUIElementBase::SetChanged() const
{
	m_lastChangedTime = std::chrono::system_clock::now();
	m_CanChange = false;
	return false;
}
