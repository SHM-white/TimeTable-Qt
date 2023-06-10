#ifndef IMPORTFROMFILE_H
#define IMPORTFROMFILE_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class ImportFromFile;
}

class ImportFromFile : public QDialog
{
    Q_OBJECT

public:
    explicit ImportFromFile(QWidget *parent = nullptr);
    ~ImportFromFile();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

    void on_buttonBox_accepted();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_OK_clicked();

    void on_pushButton_CANCEL_clicked();

private:
    Ui::ImportFromFile *ui;
};

#endif // IMPORTFROMFILE_H
