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
	m_lastChangedTime = std::chrono::system_clock::now();
	m_CanChange = false;
#ifdef DEBUG
	OutputDebugStringW(std::format(L"Item:{} SetChanged() called\n", int(m_ItemType)).c_str());
	OutputDebugStringW(std::format(L"m_lastChangedTime:{}s\n", std::chrono::duration_cast<std::chrono::seconds>(m_lastChangedTime.time_since_epoch()).count()).c_str());
#endif // DEBUG

	return false;
}
