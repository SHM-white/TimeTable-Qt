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
    void paintEvent(QPaintEvent*) override;
    TimeTableQt* pParent;
    QPoint mouseStartPoint;
    QPoint windowTopLeftPoint;
    bool m_bDrag{ false };
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
};

#endif // TODAYALLLESSONS_H
