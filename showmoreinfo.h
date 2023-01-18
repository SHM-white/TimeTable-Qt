#ifndef SHOWMOREINFO_H
#define SHOWMOREINFO_H

#include <QDialog>

namespace Ui {
class ShowMoreInfo;
}

class ShowMoreInfo : public QDialog
{
    Q_OBJECT

public:
    explicit ShowMoreInfo(QWidget *parent = nullptr);
    ~ShowMoreInfo();

private slots:
    void on_comboBox_currentTextChanged(const QString &arg1);

private:
    Ui::ShowMoreInfo *ui;
};

#endif // SHOWMOREINFO_H
