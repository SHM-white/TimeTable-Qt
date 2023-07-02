#include "importfromfile.h"
#include "ui_importfromfile.h"
#include "TimeTableQt.h"
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
    QString openfilename = QFileDialog::getOpenFileName(this, QString("请选择需要打开的csv文件"), QString(".\\"), QString("CSV UTF-8(逗号分隔)(*.csv);;All(*.*)"));
    if (openfilename.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    else {
        this->ui->lineEdit->setText(openfilename);
    }
    
}


void ImportFromFile::on_pushButton_2_clicked()
{
    QString openfilename = QFileDialog::getSaveFileName(this, QString("请选择需要导入到的json文件"), QString(".\\"), QString("JSON文件(*.json);;All(*.*)"));
    if (openfilename.isEmpty()) {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    else {
        this->ui->lineEdit_2->setText(openfilename);
    }
    
}


void ImportFromFile::on_pushButton_OK_clicked()
{
    QString csvpath = this->ui->lineEdit->text();
    QString jsonpath = this->ui->lineEdit_2->text();
    TimeTableQt* pParent = (TimeTableQt*)parentWidget();
    if ((!csvpath.isEmpty()) && (!jsonpath.isEmpty())) {
        if (pParent->timetable->mImportLessonsFromCsv((std::string)csvpath.toLocal8Bit(), (std::string)jsonpath.toLocal8Bit())) {
            QMessageBox::information(this, QString("成功"), QString("添加成功"));
        }
    }
    else {
        QMessageBox::warning(this, QString("提示"), QString("未选择文件"));
    }
    //close();
}


void ImportFromFile::on_pushButton_CANCEL_clicked()
{
    close();
}

