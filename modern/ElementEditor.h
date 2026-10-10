#pragma once
#include "ElementLayout.h"
#include <QWidget>
namespace tt {
class ElementEditor {
public:
    // Transactional editor: caller's layout stays unchanged unless Save is pressed.
    static bool edit(QWidget* parent,QList<ElementDefinition>& layout,
                     const ScheduleStore& schedule,QSize canvasSize,bool dark);
};
}
