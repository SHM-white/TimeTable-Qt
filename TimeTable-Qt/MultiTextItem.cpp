#include "MultiTextItem.h"
#include "SingleTextItem.h"

MultiTextItem::MultiTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:MultiItemInOrderUIElementBase(value, timetable)
{
	Json::Value temp = value;
	while (!temp["Data"].empty())
	{
		Json::Value removed;
		m_Texts.push_back(std::make_shared<SingleTextItem>(temp, timetable));
		temp["Data"].removeIndex(0, &removed);
#ifdef DEBUG
		OutputDebugStringW(L"removed:");
		OutputDebugStringW(u8tw(removed.toStyledString()).c_str());
		OutputDebugStringW(L"after remove:");
		OutputDebugStringW(u8tw(value.toStyledString()).c_str());
#endif // DEBUG
	}
}

QSize MultiTextItem::getNeededSize() const
{
	QSize maxSize;
	for (auto& i : m_Texts) {
		maxSize.setWidth(std::max(maxSize.width(), i->getNeededSize().width()));
		maxSize.setHeight(std::max(maxSize.height(), i->getNeededSize().height()));
	};
	return maxSize;
}

Json::Value MultiTextItem::SaveAsJson(Json::Value& value) const
{
	for (auto& i : m_Texts) {
		value["Data"].append(i->save()["Data"][0]);
	}
	return value;
}

bool MultiTextItem::update() const
{
	m_currentIndex = (m_currentIndex + 1) % m_Texts.size();
	return false;
}

bool MultiTextItem::paint(QPainter& painter) const
{
	auto currentText = m_Texts[m_currentIndex];
	currentText->paint(painter);
	if (currentText->m_CanChange) {
		currentText->SetChanged();		
		update();
	}
	return 0;
}
