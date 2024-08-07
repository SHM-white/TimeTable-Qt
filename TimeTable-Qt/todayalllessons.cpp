﻿#include "TodayAllLessons.h"

TodayAllLessons::TodayAllLessons(Json::Value& value, std::shared_ptr<TimeTable> timetable)
    :SingleItemUIElementBase(value, timetable)
{
    Json::Value& temp = value["Data"][0];
    m_textType = ContentType(temp["Type"].asInt());
    //m_direction_Vertical = temp["Data"]["Direction"].asInt();
    m_color = QColor(
        temp["TextColor"][0].asInt(),
        temp["TextColor"][1].asInt(),
        temp["TextColor"][2].asInt(),
        temp["TextColor"][3].asInt()
    );
    m_font = QFont(QString::fromStdWString(u8tw(temp["Font"].asString())), temp["TextSize"].asInt());
    m_Data = temp["Data"];
    //m_fullName = temp["Data"]["FullName"].asBool();
    update();
}

bool TodayAllLessons::update() const
{
    m_CurrentLessonIndex = m_timetable->GetCurrentLesson(0);
    m_lessons.clear();
    lessons.clear();
    m_lessons = m_timetable->GetTodayLessons(TimeTable::GetCurrentTime(L"%a"));
    if (m_Data["FullName"].asBool()) {
        for (const auto& i : m_lessons) {
            lessons.push_back(QString::fromStdWString(i.mGetName()));
        }
    }
    else
    {
        for (const auto& i : m_lessons) {
            lessons.push_back(QString::fromStdWString(std::wstring(i.mGetName().begin(), i.mGetName().begin() + 1)));
        }
    }
    return false;
}

bool TodayAllLessons::paint(QPainter& painter) const
{
    using namespace std::chrono;
    if (std::chrono::system_clock::now() - m_lastUpdateTime > 10s) {
        update();
    }
    painter.save();
    painter.setClipRect(m_rect);
    painter.setFont(m_font);
    QFontMetrics fontMetrics = painter.fontMetrics();
    if (m_Data["Direction"].asInt()) {
        int maxLength{ 0 };
        for (const auto& i : lessons) {
            maxLength = std::max(maxLength, (int)i.size());
        }
        auto size = QSize(lessons.size() * fontMetrics.maxWidth(), (int)(maxLength * fontMetrics.height()) + 5);
        m_rect = QRect(m_rect.topLeft(), size);
        for (int i = 0; i < lessons.size(); i++) {
            QRect rect(m_rect.left() + i * fontMetrics.maxWidth(), m_rect.top(), fontMetrics.maxWidth(), maxLength * fontMetrics.height());
            painter.drawText(rect, Qt::TextWordWrap | Qt::AlignVCenter, lessons[i]);
        }
        painter.setPen(QPen(QBrush(QColor(67, 255, 9, 255)), 2));
        painter.drawRect(m_rect.left() + m_CurrentLessonIndex * fontMetrics.maxWidth() - 3, m_rect.top() + 2, fontMetrics.maxWidth(), maxLength * fontMetrics.height());
    }
    else
    {
        int maxLength{ 0 };
        for (const auto& i : lessons) {
            maxLength = std::max(maxLength, (int)i.size());
        }
        auto size = QSize(fontMetrics.maxWidth() * maxLength, fontMetrics.height() * lessons.size());
        m_rect = QRect(m_rect.topLeft(), size);
        for (int i = 0; i < lessons.size(); i++) {
            QRect rect(m_rect.left(), m_rect.top() + fontMetrics.height() * i, maxLength * fontMetrics.maxWidth(), fontMetrics.height());
            painter.drawText(rect, Qt::TextWordWrap | Qt::AlignCenter, lessons[i]);
        }
        painter.setPen(QPen(QBrush(QColor(67, 255, 9, 255)), 2));
        painter.drawRect(m_rect.left(), m_rect.top() + m_CurrentLessonIndex * fontMetrics.height(), fontMetrics.maxWidth() * maxLength, fontMetrics.height());

    }
    painter.restore();
    return false;
}

QSize TodayAllLessons::getNeededSize() const
{ 
    return m_rect.size();
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
    //temp["Data"]["Direction"] = (int)m_direction_Vertical;
    //temp["Data"]["FullName"] = m_fullName;
    value["Data"].append(temp);
//#ifdef DEBUG
//    OutputDebugStringW(L"Saved value:");
//    OutputDebugStringW(u8tw(value.toStyledString()).c_str());
//#endif // DEBUG
    return value;

}
