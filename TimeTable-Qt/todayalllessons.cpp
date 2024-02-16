#include "TodayAllLessons.h"

TodayAllLessons::TodayAllLessons(Json::Value& value, std::shared_ptr<TimeTable> timetable)
    :SingleItemUIElementBase(value, timetable)
{
    Json::Value& temp = value["Data"][0];
    m_textType = ElementType(temp["Type"].asInt());
    m_color = QColor(
        temp["TextColor"][0].asInt(),
        temp["TextColor"][1].asInt(),
        temp["TextColor"][2].asInt(),
        temp["TextColor"][3].asInt()
    );
    m_font = QFont(QString::fromStdWString(u8tw(temp["Font"].asString())), temp["TextSize"].asInt());
    m_Data = temp["Data"];
    update();
}

bool TodayAllLessons::paint(QPainter& painter) const
{
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_lastUpdateTime).count() > 10) {
        update();
    }
    return false;
}

QSize TodayAllLessons::getNeededSize() const
{
    return QSize();
}

Json::Value TodayAllLessons::SaveAsJson(Json::Value& value) const
{
    value["Data"].clear();
    Json::Value temp;
    temp["Type"] = (int)m_textType;
    temp["Font"] = wtu8(m_font.family().toStdWString());
    temp["TextSize"] = m_font.pointSize();
    temp["TextColor"][0] = m_color.red();
    temp["TextColor"][1] = m_color.green();
    temp["TextColor"][2] = m_color.blue();
    temp["TextColor"][3] = m_color.alpha();
    temp["Data"] = m_Data;
    value["Data"].append(temp);
#ifdef DEBUG
    OutputDebugStringW(L"Saved value:");
    OutputDebugStringW(u8tw(value.toStyledString()).c_str());
#endif // DEBUG
    return value;

}
