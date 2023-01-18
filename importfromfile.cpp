#include "importfromfile.h"
#include "ui_importfromfile.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

ImportFromFile::ImportFromFile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportFromFile)
{
    ui->setupUi(this);
}

ImportFromFile::~ImportFromFile()
{
    delete ui;
}

void ImportFromFile::on_buttonBox_clicked(QAbstractButton *button)
{
    //close();
}


void ImportFromFile::on_buttonBox_accepted()
{
    //close();
}


void ImportFromFile::on_pushButton_clicked()
{
    QString openfilename = QFileDialog::getOpenFileName(this, QString("请选择需要打开的csv文件"), QString("C:\\"), QString("ANSI编码的csv文件(*.csv);;All(*.*)"));
    if (openfilename.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    QMessageBox::warning(this, QString(""), QString(""));
}


void ImportFromFile::on_pushButton_2_clicked()
{
    QString openfilename = QFileDialog::getSaveFileName(this, QString("请选择需要导入到的json文件"), QString("C:\\"), QString("JSON文件(*.json);;All(*.*)"));
}

