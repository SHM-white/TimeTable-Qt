#include "SingleTextItem.h"
#include "include.h"

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
    m_Data = temp["Data"];
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
    temp["Data"] = m_Data;
    value["Data"].append(temp);
    return value;
}

bool SingleTextItem::update() const
{
    using namespace std::literals::chrono_literals;
    UIElementBase::update();
    switch (m_textType)
    {
    case CurrentTime:
        m_formatedText = m_timetable->GetCurrentTime(m_textFormat);
        m_updateAfterTime = 1s;
        break;
    case CurrentLesson:
        m_formatedText = m_textFormat + m_timetable->GetCurrentLesson(L"");
        m_updateAfterTime = 3s;
        break;
    case CountDownDay:
    {
        tm TargetTime;
        auto time = m_Data["TargetTime"];
        TargetTime.tm_year = time[0].asInt() - 1900;
        TargetTime.tm_mon  = time[1].asInt() - 1;
        TargetTime.tm_mday = time[2].asInt();
        TargetTime.tm_hour = time[3].asInt();
        TargetTime.tm_min  = time[4].asInt();
        TargetTime.tm_sec  = time[5].asInt();
        m_formatedText = m_timetable->GetCountDown(TargetTime,m_textFormat);
        m_updateAfterTime = 1s;
    }
        break;
    case TodayInfo:
        m_updateAfterTime = 5s;
        break;
    case Weather:
        m_updateAfterTime = 1h;
        break;
    case News:
        //break;
    case NextLesson:
//        break;
    case DailyWord:
        //break;
    case NextLessonTime:
//        break;
    case Default:
        m_formatedText = m_textFormat;
        m_updateAfterTime = 1h;
        break;
    default:
        break;
    }

    return false;
}

bool SingleTextItem::paint(QPainter& painter) const
{
    painter.save();
    painter.setClipRect(m_rect);
    return false;
}
