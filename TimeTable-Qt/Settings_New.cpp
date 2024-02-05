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
	ui.spinBox_scrollSpeed->setValue(ITEM_SCROLL_SPEED);
	on_pushButton_FreshInfo_clicked();
	on_pushButton_FreshLessonList_clicked();
	on_pushButton_FreshWindowList_clicked();
	on_lineEdit_BackGroundColor_editingFinished();

}

Settings_New::~Settings_New()
{}

std::shared_ptr<BasicWindow> Settings_New::GetCurrentWindow()
{
	int row = ui.listWidget_Windows->currentIndex().row();
	if ((row < 0) || (row > (*m_Windows).size())) {
		throw std::exception("error");
	}
	else
	{
		return (*m_Windows)[row];
	}
}

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
	on_lineEdit_BackGroundColor_editingFinished();
}


void Settings_New::on_pushButton_FreshLessonList_clicked()
{
	//ui.listWidget_Lessons->clear();
	//std::vector<std::wstring> in;
	//m_TimeTable->GetLesson(in, ui.comboBox_LessonDay->currentText().toStdWString());
	//for (auto& i : in) {
	//	QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(i));
	//	item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
	//	ui.listWidget_Lessons->addItem(item); 
	//}
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
	ui.listWidget_Infos->clear();
	std::vector<std::wstring> in;
	m_TimeTable->GetTodayMoreInfo(in, ui.comboBox_LessonDay->currentText().toStdWString());
	for (auto& i : in) {
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(i));
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.listWidget_Infos->addItem(item);
	}

}


void Settings_New::on_pushButton_Cancel_clicked()
{
	on_pushButton_reloadWindows_clicked();
}


void Settings_New::on_pushButton_Save_clicked()
{
	Json::Value Windows;
	for (auto i : (*m_Windows)) {
		Windows.append(i->save());
	}
	std::wstring path;
	Json::GetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
	Json::Value root = Json::GetRootJsonValue(path);
	root["Windows"] = Windows;
	Json::SaveJson(path, root);
	QMessageBox::information(this, QString::fromStdWString(L"success"), QString::fromStdWString(L"保存成功"));
	on_pushButton_reloadWindows_clicked();
}


void Settings_New::on_pushButton_EditElements_clicked()
{
	if (m_Windows->empty()) {
		return;
	}
	Json::Value value;
	value["Moveable"] = true;
	EditUIElements* window = new EditUIElements(GetCurrentWindow(), value, this);
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
		color.setAlpha(ui.spinBox_BackGroundAlpha->value());
		auto pixmap = QPixmap(ui.label_ColorSample->size());
		pixmap.fill(color);
		ui.label_ColorSample->setAutoFillBackground(true);
		ui.label_ColorSample->setPixmap(pixmap);
		int icolor[4]{ color.red(),color.green(),color.blue(),color.alpha() };
		auto window = GetCurrentWindow();
		for (int i = 0; i < 4; i++) {
			window->miBackGroundColor[i] = icolor[i];
		}
	}
}


void Settings_New::on_checkBox_ColorBackGround_stateChanged(int arg1)
{
	GetCurrentWindow()->mUseImgAsBackGround = !arg1;
}


void Settings_New::on_checkBox_AutoResize_stateChanged(int arg1)
{
	GetCurrentWindow()->m_autoResize = arg1;
}


void Settings_New::on_checkBox_Moveable_stateChanged(int arg1)
{
	GetCurrentWindow()->m_moveable = arg1;
}


void Settings_New::on_checkBox_TopMost_stateChanged(int arg1)
{
	GetCurrentWindow()->m_TopMost = arg1;
}


void Settings_New::on_spinBox_W_valueChanged(int arg1)
{
	GetCurrentWindow()->miWindowWeight = arg1;
}


void Settings_New::on_spinBox_H_valueChanged(int arg1)
{
	GetCurrentWindow()->miWindowHeight = arg1;
}


void Settings_New::on_spinBox_X_valueChanged(int arg1)
{
	GetCurrentWindow()->miWindowX = arg1;
}


void Settings_New::on_spinBox_Y_valueChanged(int arg1)
{
	GetCurrentWindow()->miWindowY = arg1;
}


void Settings_New::on_spinBox_FPS_valueChanged(int arg1)
{
	GetCurrentWindow()->m_maxFPS = arg1;
}


void Settings_New::on_checkBox_AutoOpen_stateChanged(int arg1)
{
	GetCurrentWindow()->m_AutoOpen = arg1;
}

void Settings_New::on_commandLinkButton_clicked()
{
	QDesktopServices::openUrl(QUrl(QString::fromStdWString(L"https://github.com/SHM-white/TimeTable")));
}


void Settings_New::on_commandLinkButton_2_clicked()
{
	QDesktopServices::openUrl(QUrl(QString::fromStdWString(L"https://github.com/SHM-white/TimeTable-Qt")));
}

void Settings_New::on_pushButton_newWindow_clicked()
{
	Json::Value value;
	value["Moveable"] = true;
	m_Windows->push_back(std::make_shared<SubWindow>(value, m_TimeTable));
	on_pushButton_FreshWindowList_clicked();
}

void Settings_New::on_pushButton_deleteWindow_clicked()
{
	int index = ui.listWidget_Windows->currentRow();
	if (index < 0 || index >= m_Windows->size()) {
		QMessageBox::warning(this, QString::fromStdWString(L"error"), QString::fromStdWString(L"所选项目无效"));
		return;
	}
	else
	{
		auto choose = QMessageBox::warning(this, QString::fromStdWString(L"警告"), QString::fromStdWString(std::format(L"正在删除：{}", GetCurrentWindow()->m_name)), QMessageBox::Yes | QMessageBox::No);
		if (choose == QMessageBox::Yes) {
			m_Windows->erase(m_Windows->begin() + index);
			QMessageBox::information(this, QString::fromStdWString(L"success"), QString::fromStdWString(L"成功删除，请保存更改"));
		}
	}
	on_pushButton_FreshWindowList_clicked();
}

void Settings_New::on_pushButton_FreshWindowList_clicked()
{
	ui.listWidget_Windows->clear();
	for (auto i : (*m_Windows)) {
		auto* item = new QListWidgetItem(QString::fromStdWString(i->m_name));
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.listWidget_Windows->addItem(item);
	}
	ui.listWidget_Windows->setCurrentRow(0);
}

void Settings_New::on_pushButton_reloadWindows_clicked()
{
	dynamic_cast<MainWindow*>(m_pParent)->ReopenWindows();
	on_pushButton_FreshWindowList_clicked();
}

void Settings_New::on_listWidget_Windows_currentRowChanged(int currentRow)
{
	if (ui.listWidget_Windows->count() > 0 && currentRow >= 0)
	{
		auto window = GetCurrentWindow();
		ui.checkBox_AutoOpen->setChecked(window->m_AutoOpen);
		ui.checkBox_AutoResize->setChecked(window->m_autoResize);
		ui.checkBox_TopMost->setChecked(window->m_TopMost);
		ui.checkBox_Moveable->setChecked(window->m_moveable);
		ui.spinBox_FPS->setValue(window->m_maxFPS);
		ui.checkBox_ColorBackGround->setChecked(window->mUseImgAsBackGround);
		ui.spinBox_W->setValue(window->miWindowWeight);
		ui.spinBox_H->setValue(window->miWindowHeight);
		ui.spinBox_X->setValue(window->miWindowX);
		ui.spinBox_Y->setValue(window->miWindowY);
		QColor color(window->miBackGroundColor[0], window->miBackGroundColor[1], window->miBackGroundColor[2], window->miBackGroundColor[3]);
		ui.lineEdit_BackGroundColor->setText(QString::fromStdWString(QColorToHexString(color)));
		ui.spinBox_BackGroundAlpha->setValue(color.alpha());
	}
}


void Settings_New::on_listWidget_Windows_itemChanged(QListWidgetItem *item)
{
	int index = ui.listWidget_Windows->currentRow();
	if (index >= 0 && index < m_Windows->size())
	{
		GetCurrentWindow()->m_name = item->text().toStdWString();
	}
}


void Settings_New::on_listWidget_Lessons_itemChanged(QListWidgetItem *item)
{

}


void Settings_New::on_listWidget_Infos_itemChanged(QListWidgetItem *item)
{

}


void Settings_New::on_spinBox_scrollSpeed_valueChanged(int arg1)
{
	ITEM_SCROLL_SPEED = arg1;
}


void Settings_New::on_comboBox_LessonDay_currentIndexChanged(int index)
{
	on_pushButton_FreshLessonList_clicked();
}

