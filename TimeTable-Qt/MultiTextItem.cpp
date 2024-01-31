#include "MultiTextItem.h"
#include "SingleTextItem.h"

MultiTextItem::MultiTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable)
	:MultiItemInOrderUIElementBase(value, timetable)
{
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
	return value;
}
