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
    m_font = QFont(QString::fromStdWString(u8tw(temp["Font"].asString())),temp["TextSize"].asInt());
    m_Data = temp["Data"];
}

QSize SingleTextItem::getNeededSize() const
{
    QFontMetrics fm(m_font);
    QSize textSize = fm.size(Qt::TextSingleLine, QString::fromStdWString(m_formatedText));
    return QSize(std::max(textSize.width(), m_rect.size().width()), std::max(textSize.height(), m_rect.size().height()));
}

Json::Value SingleTextItem::SaveAsJson(Json::Value& value) const
{
    value["Data"].clear();
    Json::Value temp;
    temp["TextFormat"] = wtu8(m_textFormat);
    temp["Type"] = (int)m_textType;
    temp["Font"] = wtu8(m_font.toString().toStdWString());
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
        m_formatedText = m_timetable->GetInfo(true);
        m_updateAfterTime = 5s;
        break;
    case Weather:
    {
        static bool success{ false };
        if (m_Data["Adcode"].isNull()) {
            m_Data["Adcode"] = wtu8(m_timetable->GetAdcodeSync(u8tw(m_Data["Address"].asString()), u8tw(m_Data["City"].asString()), u8tw(m_Data["APIKey"].asString()))).c_str();
        }
        if (!success) {
            m_formatedText = m_timetable->GetWeather(u8tw(m_Data["Adcode"].asString()), u8tw(m_Data["APIKey"].asString()), &success);
            m_updateAfterTime = 1s;
        }
        else
        {
            m_updateAfterTime = 1h;
            success = false;
        }
    }
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
    default:
        m_formatedText = m_textFormat;
        m_updateAfterTime = 1h;
        break;
    }
    return false;
}

bool SingleTextItem::paint(QPainter& painter) const
{
    painter.save();
    painter.setClipRect(m_rect);
    painter.setFont(m_font);
    painter.setPen(m_color);
    auto newRect = m_rect;
    if (getNeededSize().width() > m_rect.size().width()) {
        auto timeAfterChange = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_lastChangedTime).count();
        newRect.setTopLeft(m_rect.topLeft() + QPoint(getNeededRect().width() - timeAfterChange / ITEM_SCROLL_SPEED, 0));
        newRect.setSize(getNeededSize());
        if (newRect.right() < m_rect.left()) {
            m_CanChange = true;
        }
    }
    else {
        if ((std::chrono::system_clock::now() - m_lastChangedTime) > m_CanChangeAfterTime) {
            m_CanChange = true;
        }
    }
    painter.drawText(newRect, Qt::AlignCenter, QString::fromStdWString(m_formatedText));
    painter.restore();
    
    if (std::chrono::system_clock::now() - m_lastUpdateTime > m_updateAfterTime) {
        update();
    }
    return false;
}
