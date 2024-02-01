#include "SingleTextItem.h"

SingleTextItem::SingleTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable)
    :SingleItemUIElementBase(value, timetable)
{
#ifdef DEBUG
    OutputDebugStringW(L"SingleTextItem() value:");
    OutputDebugStringW(u8tw(value.toStyledString()).c_str());
#endif // DEBUG
    Json::Value& temp = value["Data"][0];
    m_textType = ElementType(temp["Type"].asInt());
    m_textFormat = u8tw(temp["TextFormat"].asString());
    m_color = QColor(
        temp["TextColor"][0].asInt(),
        temp["TextColor"][1].asInt(),
        temp["TextColor"][2].asInt(),
        temp["TextColor"][3].asInt()
    );
    m_font = QFont(QString::fromStdWString(u8tw(temp["Font"].asString())));
}

QSize SingleTextItem::getNeededSize() const
{
    return m_rect.size();
}

Json::Value SingleTextItem::SaveAsJson(Json::Value& value) const
{
    value["Data"].clear();
    Json::Value temp;
    temp["TextFormat"] = wtu8(m_textFormat);
    temp["Type"] = (int)m_textType;
    temp["Font"] = wtu8(m_font.toString().toStdWString());
    temp["TextColor"][0] = m_color.red();
    temp["TextColor"][1] = m_color.green();
    temp["TextColor"][2] = m_color.blue();
    temp["TextColor"][3] = m_color.alpha();
    value["Data"].append(temp);
    return value;
}
