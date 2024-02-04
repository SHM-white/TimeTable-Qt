#include "Settings_New.h"
#include "EditUIElements.h"

Settings_New::Settings_New(std::vector<std::shared_ptr<BasicWindow>>* Windows, Json::Value WindowSettings, QWidget* parent)
	: BasicWindow(WindowSettings, parent), m_Windows{ Windows }, m_pParent{ parent }
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui.setupUi(this);
	m_hide = false;
	m_moveable = true;
	ui.label_version->setText(QString::fromStdWString(GetCurrentVersion()));
	ui.label_complieTime->setText(QString::fromStdWString(GetCompileTime()));
	on_lineEdit_BackGroundColor_editingFinished();
	on_pushButton_FreshInfo_clicked();
	on_pushButton_FreshLessonList_clicked();
	
}

Settings_New::~Settings_New()
{}

Json::Value Settings_New::SaveAsJson(Json::Value value) const
{
	return value;
}

void Settings_New::on_pushButton_close_clicked()
{
    this->close();
}


void Settings_New::on_spinBox_BackGroundAlpha_valueChanged(int arg1)
{
    this->ui.horizontalSlider_BackGroundAlpha->setValue(arg1);
}


void Settings_New::on_horizontalSlider_BackGroundAlpha_valueChanged(int value)
{
    this->ui.spinBox_BackGroundAlpha->setValue(value);
}


void Settings_New::on_pushButton_FreshLessonList_clicked()
{

}


void Settings_New::on_pushButton_SortLessons_clicked()
{

}


void Settings_New::on_pushButton_DeleteLesson_clicked()
{

}


void Settings_New::on_pushButton_AddLesson_clicked()
{

}


void Settings_New::on_pushButton_AddInfo_clicked()
{

}


void Settings_New::on_pushButton_DeleteInfo_clicked()
{

}


void Settings_New::on_pushButton_FreshInfo_clicked()
{

}


void Settings_New::on_pushButton_Cancel_clicked()
{

}


void Settings_New::on_pushButton_Save_clicked()
{

}


void Settings_New::on_pushButton_EditElements_clicked()
{
	if (m_Windows->empty()) {
		return;
	}
	Json::Value value;
	value["Moveable"] = true;
	int index = ui.columnView_Windows->currentIndex().row();
#ifdef DEBUG
	EditUIElements* window = new EditUIElements((*m_Windows)[0], value, this);
#else
	if (index < 0) {
		return;
	}
	EditUIElements* window = new EditUIElements((*m_Windows)[index], value, this);
#endif // DEBUG
	window->show();
	
}


void Settings_New::on_pushButton_ChooseColor_clicked()
{
	QColor color = QColorDialog::getColor(ui.lineEdit_BackGroundColor->text(), this);
	if (color.isValid()) {
		ui.lineEdit_BackGroundColor->setText(QString::fromStdWString(QColorToHexString(color)));
		on_lineEdit_BackGroundColor_editingFinished();
	}
}


void Settings_New::on_lineEdit_BackGroundColor_editingFinished()
{
	QColor color = HexStringToQColor(ui.lineEdit_BackGroundColor->text().toStdWString());
	if (color.isValid())
	{
		auto pixmap = QPixmap(ui.label_ColorSample->size());
		pixmap.fill(color);
		ui.label_ColorSample->setAutoFillBackground(true);
		ui.label_ColorSample->setPixmap(pixmap);
	}
}


void Settings_New::on_checkBox_ColorBackGround_stateChanged(int arg1)
{

}


void Settings_New::on_checkBox_AutoResize_stateChanged(int arg1)
{

}


void Settings_New::on_checkBox_Moveable_stateChanged(int arg1)
{

}


void Settings_New::on_checkBox_TopMost_stateChanged(int arg1)
{

}


void Settings_New::on_spinBox_W_valueChanged(int arg1)
{

}


void Settings_New::on_spinBox_H_valueChanged(int arg1)
{

}


void Settings_New::on_spinBox_X_valueChanged(int arg1)
{

}


void Settings_New::on_spinBox_Y_valueChanged(int arg1)
{

}


void Settings_New::on_spinBox_FPS_valueChanged(int arg1)
{

}


void Settings_New::on_checkBox_AutoOpen_stateChanged(int arg1)
{

}

void Settings_New::on_commandLinkButton_clicked()
{

}


void Settings_New::on_commandLinkButton_2_clicked()
{

}

