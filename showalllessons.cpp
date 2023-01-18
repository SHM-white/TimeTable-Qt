#include "showalllessons.h"
#include "ui_showalllessons.h"

ShowAllLessons::ShowAllLessons(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowAllLessons)
{
    ui->setupUi(this);
}

ShowAllLessons::~ShowAllLessons()
{
    delete ui;
}

void ShowAllLessons::on_checkBox_stateChanged(int arg1)
{

}

