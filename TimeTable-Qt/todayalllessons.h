#ifndef TODAYALLLESSONS_H
#define TODAYALLLESSONS_H

#include <QWidget>
#include "TimeTableQt.h"

namespace Ui {
class TodayAllLessons;
}

class TodayAllLessons : public QWidget
{
    Q_OBJECT

public:
    explicit TodayAllLessons(QWidget *parent = nullptr);
    ~TodayAllLessons();

private:
    Ui::TodayAllLessons *ui;
    void paintEvent(QPaintEvent* event) override;
    TimeTableQt* pParent;
};

#endif // TODAYALLLESSONS_H
