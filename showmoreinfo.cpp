#include "showmoreinfo.h"
#include "ui_showmoreinfo.h"

ShowMoreInfo::ShowMoreInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowMoreInfo)
{
    ui->setupUi(this);
}

ShowMoreInfo::~ShowMoreInfo()
{
    delete ui;
}

void ShowMoreInfo::on_comboBox_currentTextChanged(const QString &arg1)
{

}

