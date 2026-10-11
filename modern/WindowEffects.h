#pragma once
#include <QColor>
class QWidget;
namespace tt {
// Optional; unsupported Windows versions keep the ordinary translucent background.
bool applyAcrylic(QWidget* window, bool enabled, const QColor& tint);
}
