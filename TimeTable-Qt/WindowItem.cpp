#include "WindowItem.h"


bool WindowItem::paint(QPainter& painter)
{
    QRectF rect(position, size); // Create a rectangle with the specified position and size
    painter.setClipRect(rect); // Set the painter's clip region to the rectangle

#if DEBUG||Debug
    painter.drawRect(rect);
#endif

    // Set the painter's font and color
    painter.setFont(font);
    painter.setPen(color);
    // Calculate the text size
    QFontMetrics fm(font);
    QSizeF textSize = fm.size(Qt::TextSingleLine, text);
    //TODO:添加滚动开始和结束短暂的ting'liu
    // If the text is larger than the item's dimensions, enable scrolling
    if (textSize.width() > size.width()) {
        int scrollPosition = (int)(QDateTime::currentMSecsSinceEpoch() / 60) % (int)(textSize.width() - size.width() + 20);
        painter.translate(-scrollPosition-(int)(textSize.width() - size.width())-10, 0);
    }
    // Draw the text within the rectangle
    rect = QRectF(position, textSize);
    painter.drawText(rect, Qt::AlignCenter, text);

    return true;
}
