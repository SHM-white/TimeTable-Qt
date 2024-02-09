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

UIElementBase* EditUIElements::GetCurrentElement()
{
	int row1 = ui.listWidget->currentRow();
	if (row1 >= 0 && row1 < m_window->m_UIElements.size()) {
		auto p1 = m_window->m_UIElements[row1];
		if (p1->m_type == UIElementType::MultiItemInOrder) {
			ui.listWidget_2->setEnabled(true);
			ui.comboBox_ElementType->setEnabled(true);
			//auto p2 = std::dynamic_pointer_cast<MultiTextItem, UIElementBase>(p1);

		}
		else if(p1->m_type==UIElementType::SingleItem)
		{
			ui.listWidget_2->setEnabled(false);
			ui.comboBox_ElementType->setEnabled(false);
		}
	}
	return nullptr;
}

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

void EditUIElements::on_pushButton_checkAndSaveData_clicked()
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

void EditUIElements::FreshUIElementsList()
{
}

void EditUIElements::FreshElementsList()
{
}
