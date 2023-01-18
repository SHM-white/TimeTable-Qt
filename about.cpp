#include "about.h"
#include "ui_about.h"
#include "TimeTableQt.h"
#include <QDesktopServices>

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


void About::on_commandLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl(QString("https://github.com/SHM-white/TimeTable")));
}


void About::on_commandLinkButton_2_clicked()
{
    QDesktopServices::openUrl(QUrl(QString("https://github.com/SHM-white/TimeTable-Qt")));
}

