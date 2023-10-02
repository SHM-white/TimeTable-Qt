#include "WindowItem.h"


bool WindowItem::paint(QPainter& painter)
{
    QRectF rect(position, size); // Create a rectangle with the specified position and size
    painter.setClipRect(rect); // Set the painter's clip region to the rectangle
    if (SHOW_ITEM_RECT) {
        painter.drawRect(rect);
    }
    // Set the painter's font and color
    painter.setFont(font);
    painter.setPen(color);
    // Calculate the text size
    QFontMetrics fm(font);
    QSizeF textSize = fm.size(Qt::TextSingleLine, text);
    QPoint newPosition;
    // If the text is larger than the item's dimensions, enable scrolling
    if (textSize.width() > size.width()) {
        if(m_updated){
            m_lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
            m_updated = false;
        }
        //todo：从最右侧开始滚动至最左侧后将m_needUpdate设置为true
        int scrollPosition = ((int(QDateTime::currentMSecsSinceEpoch()) - m_lastUpdateTime + int(textSize.width())) / 20) % (int(textSize.width()) + size.width());
        newPosition = QPoint(position.x() + size.width() - scrollPosition, position.y());
        if (scrollPosition >= textSize.width() + size.width() - 1) {
            m_needUpdate = true;
        }
    }
    else {
        newPosition = position;
        ++m_printCount;
        if (m_printCount >= 120) {
            m_needUpdate = true;
            m_printCount = 0;
        }
    }
    // Draw the text within the rectangle
    rect = QRectF(newPosition, textSize);
    if (SHOW_ITEM_RECT) {
        painter.drawRect(rect);
    }
    painter.drawText(rect, Qt::AlignCenter, text);
    
    return true;
}

QString& WindowItem::Text()
{
    return text;
}
