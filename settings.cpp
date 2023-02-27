#include "settings.h"
#include "ui_settings.h"
#include "TimeTableQt.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    pParent = parent;
    ui->setupUi(this);
}

Settings::~Settings()
{
    delete ui;
}

void Settings::FlashList(int index)
{
    if (index == 0) {//Lessons

    }
    else if(index==1){//Infos
    
    }
    else if (index == 2) {//Windows

    }
    else if(index==3){//path
    
    }
    
}

void Settings::FlashList()
{
    FlashList(ui->tabWidget->currentIndex());
}

void Settings::on_tabWidget_currentChanged(int index)
{
    FlashList(index);
}


void Settings::on_pushButton_delete_clicked()
{

}


void Settings::on_pushButton_order_clicked()
{

}

