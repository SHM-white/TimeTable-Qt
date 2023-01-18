#include "about.h"
#include "ui_about.h"
#include "TimeTableQt.h"

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    TimeTableQt* p = (TimeTableQt*)parentWidget();
    std::string about;
    p->windowsettings.mGetTextItem("About", about);
    QString qabout{ QString::fromLocal8Bit(about) };
    this->ui->textEdit->setText(qabout);
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    close();
}

