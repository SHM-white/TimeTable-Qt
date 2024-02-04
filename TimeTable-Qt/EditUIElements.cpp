#include "EditUIElements.h"

EditUIElements::EditUIElements(std::shared_ptr<BasicWindow> window, Json::Value WindowSettings, QWidget* parent)
	: BasicWindow(WindowSettings,parent),m_window{window}
{
	ui.setupUi(this);
	m_hide = false;
	m_moveable = true;

	this->setAttribute(Qt::WA_DeleteOnClose);
	connect(this, SIGNAL(QWidget::destroyed), this, SLOT(QDialog::deleteLater));
	this->setWindowModality(Qt::ApplicationModal);
}

EditUIElements::~EditUIElements()
{}

Json::Value EditUIElements::SaveAsJson(Json::Value value) const
{
	return value;
}

void EditUIElements::on_pushButton_close_clicked()
{
	this->close();
}


void EditUIElements::on_lineEdit_BackGroundColor_editingFinished()
{

}


void EditUIElements::on_pushButton_chooseColor_clicked()
{

}


void EditUIElements::on_fontComboBox_currentFontChanged(const QFont &f)
{

}


void EditUIElements::on_lineEdit_editingFinished()
{

}


void EditUIElements::on_comboBox_currentIndexChanged(int index)
{

}


void EditUIElements::on_pushButton_checkAndSaveData_clicked()
{

}


void EditUIElements::on_comboBox_2_currentIndexChanged(int index)
{

}


void EditUIElements::on_pushButton_addUIElement_clicked()
{

}


void EditUIElements::on_pushButton_deleteUIElemrnt_clicked()
{

}


void EditUIElements::on_pushButton_AddItem_clicked()
{

}


void EditUIElements::on_pushButton_deleteItem_clicked()
{

}

