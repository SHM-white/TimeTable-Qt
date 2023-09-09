#include "changeconfigpath.h"
#include "ui_changeconfigpath.h"
#include "TimeTableQt.h"
#include <qfiledialog.h>
#include <qmessagebox.h>

ChangeConfigPath::ChangeConfigPath(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeConfigPath)
{
    ui->setupUi(this);
    this->ui->lineEdit->setText(QString::fromStdWString(L".\\Config.json"));
}

ChangeConfigPath::~ChangeConfigPath()
{
    delete ui;
}

void ChangeConfigPath::on_pushButton_clicked()
{
    QString openfilename = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择需要打开json"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"JSON文件(*.json);;All(*.*)"));
    if (openfilename.isEmpty()) {
        QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
    }
    else {
        this->ui->lineEdit->setText(openfilename);
    }
    //choose
}


void ChangeConfigPath::on_pushButton_2_clicked()
{
    TimeTableQt* pParent = (TimeTableQt*)parentWidget();
    if (pParent->windowsettings->mChangeConfigPath(this->ui->lineEdit->text().toStdWString()).length()!=0) {
        pParent->windowsettings->mGetWindowSettings();
        pParent->mInitializeWindow();
        QMessageBox::information(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"成功"));
        this->close();
    }
    else {
        QMessageBox::information(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"失败"));
    }
}

