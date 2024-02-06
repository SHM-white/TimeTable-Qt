#include "UIElementBase.h"
#include "EmptyUIElement.h"
#include "SingleItemUIElementBase.h"
#include "MultiItemInOrderUIElementBase.h"
#include "MultiItemAllDisplayUIElementBase.h"
#include "MultiTextItem.h"
#include "SingleTextItem.h"

UIElementBase::UIElementBase(Json::Value& setting, std::shared_ptr<TimeTable> timetable)
	:m_timetable{ timetable }, m_selfJson{ setting }
{
	m_rect = QRect(setting["Location"][0].asInt(), setting["Location"][1].asInt(), setting["Size"][0].asInt(), setting["Size"][1].asInt());
	this->update();
}

bool UIElementBase::paintRect(QPainter& painter) const
{
	painter.save();
	painter.drawRect(m_rect);
	painter.restore();
	return false;
}

bool UIElementBase::update() const
{
	m_lastUpdateTime = std::chrono::steady_clock::now();
	return false;
}

QRect UIElementBase::getNeededRect() const
{
	return m_rect;
}

Json::Value UIElementBase::save() const
{
	Json::Value value = m_selfJson;
	value["Location"].clear();
	value["Size"].clear();
	value["Location"].append(m_rect.left());
	value["Location"].append(m_rect.top());
	value["Size"].append(m_rect.width());
	value["Size"].append(m_rect.height());
	return SaveAsJson(value);
}

std::shared_ptr<SingleItemUIElementBase> CreateSingleItemUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
{
	bool textOnly{ true };
#ifdef DEBUG
	OutputDebugStringW(L"CreateSingleItemUIElement() value:");
	OutputDebugStringW(u8tw(value.toStyledString()).c_str());
#endif // DEBUG
	for (auto& i : value["Data"]) {
		textOnly &= (i["Type"].asInt() <= (int)ElementType::Weather);
	}
	if (textOnly) {
		return std::dynamic_pointer_cast<SingleItemUIElementBase, SingleTextItem>(std::make_shared<SingleTextItem>(value, timetable));
	}
	else
	{
		throw std::exception("Hasn't support!");
	}
}

std::shared_ptr<MultiItemInOrderUIElementBase> CreateMultiItemInOrderUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
{
	bool textOnly{ true };
	for (auto& i : value["Data"]) {
		textOnly &= (i["Type"].asInt() <= (int)ElementType::Weather);
	}
	if (textOnly) {
		return std::dynamic_pointer_cast<MultiItemInOrderUIElementBase, MultiTextItem>(std::make_shared<MultiTextItem>(value, timetable));
	}
	else
	{
		throw std::exception("Hasn't support!");
	}
}

std::shared_ptr<MultiItemAllDisplayUIElementBase> CreateMultiItemAllDisplayUIElement(Json::Value& value, std::shared_ptr<TimeTable> timetable)
{
	return std::shared_ptr<MultiItemAllDisplayUIElementBase>();
}
