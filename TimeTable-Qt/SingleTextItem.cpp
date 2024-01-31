#include "SingleTextItem.h"

SingleTextItem::SingleTextItem(Json::Value& value, std::shared_ptr<TimeTable> timetable)
    :SingleItemUIElementBase(value, timetable)
{
    Json::Value& temp = value["Data"][0];
    m_textType = ElementType(temp["Type"].asInt());
    m_textFormat = u8tw(temp["TextFormat"].asString());
    m_color = QColor(
        temp["Color"][0].asInt(),
        temp["Color"][1].asInt(),
        temp["Color"][2].asInt(),
        temp["Color"][3].asInt()
    );
    m_font = QFont(QString::fromStdWString(u8tw(temp["Font"].asString())));
}

QSize SingleTextItem::getNeededSize() const
{
    return m_rect.size();
}

Json::Value SingleTextItem::SaveAsJson(Json::Value& value) const
{
    Json::Value temp;
    temp["TextFormat"] = wtu8(m_textFormat);
    temp["Type"] = (int)m_textType;
    temp["Font"] = wtu8(m_font.toString().toStdWString());
    temp["Color"][0] = m_color.red();
    temp["Color"][1] = m_color.green();
    temp["Color"][2] = m_color.blue();
    temp["Color"][3] = m_color.alpha();
    value["Data"].append(temp);
    return value;
}
