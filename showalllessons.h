#ifndef SHOWALLLESSONS_H
#define SHOWALLLESSONS_H

#include <QDialog>

namespace Ui {
class ShowAllLessons;
}

class ShowAllLessons : public QDialog
{
    Q_OBJECT

public:
    explicit ShowAllLessons(QWidget *parent = nullptr);
    ~ShowAllLessons();

private slots:
    void on_checkBox_stateChanged(int arg1);

private:
    Ui::ShowAllLessons *ui;
};

#endif // SHOWALLLESSONS_H
