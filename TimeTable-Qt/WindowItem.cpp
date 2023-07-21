#include "WindowItem.h"


bool WindowItem::paint(QPainter& painter)
{
    QRectF rect(position, size); // Create a rectangle with the specified position and size
    painter.setClipRect(rect); // Set the painter's clip region to the rectangle

    // Calculate the text size
    QFontMetrics fm(font);
    QSizeF textSize = fm.size(Qt::TextSingleLine, text);

    // If the text is larger than the item's dimensions, enable scrolling
    if (textSize.width() > size.width() || textSize.height() > size.height()) {
        int scrollPosition = (int)(QDateTime::currentMSecsSinceEpoch() / 10) % (int)(textSize.width() + size.width());
        painter.translate(-scrollPosition, 0);
    }

    // Set the painter's font and color
    painter.setFont(font);
    painter.setPen(color);

    // Draw the text within the rectangle
    painter.drawText(rect, Qt::AlignCenter, text);

    return true;
}
