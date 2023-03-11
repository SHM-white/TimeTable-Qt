#ifndef CHANGECONFIGPATH_H
#define CHANGECONFIGPATH_H

#include <QDialog>

namespace Ui {
class ChangeConfigPath;
}

class ChangeConfigPath : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeConfigPath(QWidget *parent = nullptr);
    ~ChangeConfigPath();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ChangeConfigPath *ui;
};

#endif // CHANGECONFIGPATH_H
