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
	return nullptr;
}

void EditUIElements::CurrentElementChanged()
{
	int row1 = ui.listWidget->currentRow();
	if (row1 >= 0 && row1 < m_window->m_UIElements.size()) {
		auto p1 = m_window->m_UIElements[row1];
		if (p1->m_type == UIElementType::MultiItemInOrder) {
			ui.listWidget_2->setEnabled(true);
			ui.comboBox_ElementType->setEnabled(true);
			
		}
		else if (p1->m_type == UIElementType::SingleItem)
		{
			ui.listWidget_2->setEnabled(false);
			ui.comboBox_ElementType->setEnabled(false);
		}
	}

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
	QColor color = HexStringToQColor(ui.lineEdit_BackGroundColor->text().toStdWString());
	if (color.isValid())
	{
		auto pixmap = QPixmap(ui.label_ColorSample->size());
		pixmap.fill(color);
		ui.label_ColorSample->setAutoFillBackground(true);
		ui.label_ColorSample->setPixmap(pixmap);
		int icolor[4]{ color.red(),color.green(),color.blue(),color.alpha() };
	}
}

void EditUIElements::on_pushButton_chooseColor_clicked()
{
	QColor color = QColorDialog::getColor(ui.lineEdit_BackGroundColor->text(), this);
	if (color.isValid()) {
		ui.lineEdit_BackGroundColor->setText(QString::fromStdWString(QColorToHexString(color)));
		on_lineEdit_BackGroundColor_editingFinished();
	}
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
