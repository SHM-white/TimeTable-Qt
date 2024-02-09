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
	ui.comboBox_InfoDay->setCurrentText(QString::fromStdWString(m_TimeTable->GetCurrentTime(L"%a")));
	ui.comboBox_LessonDay->setCurrentText(QString::fromStdWString(m_TimeTable->GetCurrentTime(L"%a")));
	on_pushButton_FreshInfo_clicked();
	on_pushButton_FreshLessonList_clicked();
	on_pushButton_FreshWindowList_clicked();
	on_lineEdit_BackGroundColor_editingFinished();
	on_pushButton_restoreConfigPath_clicked();
	on_pushButton_restoreLessonPath_clicked();
	//on_listWidget_Windows_currentRowChanged(0);
	_ASSERTE(_CrtCheckMemory());

}

Settings_New::~Settings_New()
{
}

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
	for (auto i : comboBoxes) {
		delete i;
	}
	ListsInitialized = false;
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
	
	ListsInitialized = false;
	ui.tableWidget_Lessons->clear();
	while (ui.tableWidget_Lessons->rowCount()!=0)
	{
		ui.tableWidget_Lessons->removeRow(0);
	}
	for (auto i : comboBoxes) {
		delete i;
	}
	comboBoxes.clear();
	ui.tableWidget_Lessons->setColumnCount(3);
	ui.tableWidget_Lessons->setHorizontalHeaderLabels(QStringList() << QString::fromStdWString(L"名称") << QString::fromStdWString(L"开始时间") << QString::fromStdWString(L"结束时间"));
	auto lessons = m_TimeTable->GetTodayLessons(ui.comboBox_LessonDay->currentText().toStdWString());
	ui.tableWidget_Lessons->setRowCount(lessons.size());
	for (int i = 0; i < lessons.size();++i) {
		auto currentLesson = lessons[i];
		auto* comboBox = new QComboBox();
		comboBox->addItem("语文");
		comboBox->addItem("数学");
		comboBox->addItem("英语");
		comboBox->addItem("物理");
		comboBox->addItem("化学");
		comboBox->addItem("生物");
		comboBox->addItem("历史");
		comboBox->addItem("政治");
		comboBox->addItem("地理");
		comboBox->addItem("班会");
		comboBox->addItem("自习");
		comboBox->setEditable(true);
		comboBox->view()->parentWidget()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
		comboBox->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground);
		comboBox->setCurrentText(QString::fromStdWString(currentLesson.mGetName()));
		connect(comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(on_LessonComboBox_TextChanged(QString)));
		auto* item2 = new QTableWidgetItem(QString::fromStdWString(currentLesson.GetBeginTimeAsString()));
		auto* item3 = new QTableWidgetItem(QString::fromStdWString(currentLesson.GetEndTimeAsString()));
		ui.tableWidget_Lessons->setCellWidget(i, 0, comboBox);
		ui.tableWidget_Lessons->setItem(i, 1, item2);
		ui.tableWidget_Lessons->setItem(i, 2, item3);
	}
	_ASSERTE(_CrtCheckMemory());

	ListsInitialized = true;
}


void Settings_New::on_pushButton_SortLessons_clicked()
{
	m_TimeTable->sortLessons(ui.comboBox_LessonDay->currentText().toStdWString());
	on_pushButton_FreshLessonList_clicked();
}


void Settings_New::on_pushButton_DeleteLesson_clicked()
{
	int row = ui.tableWidget_Lessons->currentRow();
	if (row >= 0 && row < ui.tableWidget_Lessons->rowCount()) {
		m_TimeTable->deleteLesson(row, ui.comboBox_LessonDay->currentText().toStdWString());
	}
	on_pushButton_FreshLessonList_clicked();
}


void Settings_New::on_pushButton_AddLesson_clicked()
{
	m_TimeTable->AddLesson(Lesson(ui.comboBox_LessonDay->currentText().toStdWString(), std::wstring(L"Null"), 0, 0));
	on_pushButton_FreshLessonList_clicked();
}


void Settings_New::on_pushButton_AddInfo_clicked()
{
	m_TimeTable->AddMoreInfo(ui.comboBox_InfoDay->currentText().toStdWString(), L"Null");
	on_pushButton_FreshInfo_clicked();
}


void Settings_New::on_pushButton_DeleteInfo_clicked()
{
	m_TimeTable->deleteInfo(ui.listWidget_Infos->currentRow(), ui.comboBox_InfoDay->currentText().toStdWString());
	on_pushButton_FreshInfo_clicked();
}


void Settings_New::on_pushButton_FreshInfo_clicked()
{
	ListsInitialized = false;
	ui.listWidget_Infos->clear();
	std::vector<std::wstring> in;
	m_TimeTable->GetTodayMoreInfo(in, ui.comboBox_LessonDay->currentText().toStdWString());
	for (auto& i : in) {
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdWString(i));
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.listWidget_Infos->addItem(item);
	}
	ListsInitialized = true;
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
	ui.lineEdit_BackGroundColor->setEnabled(arg1);
	ui.pushButton_ChooseColor->setEnabled(arg1);
	ui.horizontalSlider_BackGroundAlpha->setEnabled(arg1);
	ui.spinBox_BackGroundAlpha->setEnabled(arg1);
	ui.lineEdit_BackgroundImg->setEnabled(!arg1);
	ui.pushButton_ChooseBackgroundImg->setEnabled(!arg1);
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
	_ASSERTE(_CrtCheckMemory());

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
	ListsInitialized = false;
	ui.listWidget_Windows->clear();
	for (auto i : (*m_Windows)) {
		auto* item = new QListWidgetItem(QString::fromStdWString(i->m_name));
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.listWidget_Windows->addItem(item);
	}
	ListsInitialized = true;
	ui.listWidget_Windows->setCurrentRow(0);
}

void Settings_New::on_pushButton_reloadWindows_clicked()
{
	dynamic_cast<MainWindow*>(m_pParent)->ReopenWindows();
	on_pushButton_FreshWindowList_clicked();
}

void Settings_New::on_listWidget_Windows_currentRowChanged(int currentRow)
{
	if (ListsInitialized == false) {
		return;
	}
	if (ui.listWidget_Windows->count() > 0 && currentRow >= 0)
	{
		auto window = GetCurrentWindow();
		ui.checkBox_AutoOpen->setChecked(window->m_AutoOpen);
		ui.checkBox_AutoResize->setChecked(window->m_autoResize);
		ui.checkBox_TopMost->setChecked(window->m_TopMost);
		ui.checkBox_Moveable->setChecked(window->m_moveable);
		ui.spinBox_FPS->setValue(window->m_maxFPS);
		ui.checkBox_ColorBackGround->setChecked(!window->mUseImgAsBackGround);
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
	if (ListsInitialized == false) {
		return;
	}
	int index = ui.listWidget_Windows->currentRow();
	if (index >= 0 && index < m_Windows->size())
	{
		GetCurrentWindow()->m_name = item->text().toStdWString();
	}
}


void Settings_New::on_listWidget_Infos_itemChanged(QListWidgetItem *item)
{
	if (ListsInitialized == false) {
		return;
	}
	m_TimeTable->changeInfo(ui.listWidget_Infos->row(item), ui.comboBox_InfoDay->currentText().toStdWString(), item->text().toStdWString());
}


void Settings_New::on_spinBox_scrollSpeed_valueChanged(int arg1)
{
	ITEM_SCROLL_SPEED = arg1;
}


void Settings_New::on_comboBox_LessonDay_currentIndexChanged(int index)
{
	on_pushButton_FreshLessonList_clicked();
}

void Settings_New::on_LessonComboBox_TextChanged(QString arg)
{
	if (ListsInitialized == false) {
		return;
	}

	QComboBox* comBox_ = dynamic_cast<QComboBox*>(this->sender());
	if (NULL == comBox_)
	{
		return;
	}
	int x = comBox_->frameGeometry().x();
	int y = comBox_->frameGeometry().y();
	QModelIndex index = ui.tableWidget_Lessons->indexAt(QPoint(x, y));
	int row = index.row();
	LessonChangeRow(row);
}


void Settings_New::on_tableWidget_Lessons_itemChanged(QTableWidgetItem *item)
{
	LessonChangeRow(item->row());
}

void Settings_New::LessonChangeRow(int row)
{
	if (!ListsInitialized) {
		return;
	}
	auto lessonDay = ui.comboBox_LessonDay->currentText().toStdWString();
	auto comboBox = dynamic_cast<QComboBox*>(ui.tableWidget_Lessons->cellWidget(row, 0));
	auto lesson = Lesson(lessonDay, comboBox->currentText().toStdWString(), 
		ui.tableWidget_Lessons->item(row, 1)->text().toStdString(), 
		ui.tableWidget_Lessons->item(row, 2)->text().toStdString()
	);
	m_TimeTable->changeLesson(row, lessonDay, lesson);
	_ASSERTE(_CrtCheckMemory());

}


void Settings_New::on_pushButton_ChangeConfigPath_clicked()
{
	QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择需要打开json"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"JSON文件(*.json);;All(*.*)"));
	if (openFileName.isEmpty()) {
		QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
	}
	else {
		ui.lineEdit->setText(openFileName);
	}
}


void Settings_New::on_pushButton_ChangeLessonPath_clicked()
{
	QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择需要打开json"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"JSON文件(*.json);;All(*.*)"));
	if (openFileName.isEmpty()) {
		QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
	}
	else {
		ui.lineEdit_2->setText(openFileName);
	}
}


void Settings_New::on_pushButton_ApplyConfigPath_clicked()
{
	Json::ChangeValue(L"ConfigFile", ui.lineEdit->text().toStdString(), DEFAULT_CONFIG_PATH);
	QMessageBox::information(this, QString::fromStdWString(L"success"), QString::fromStdWString(L"success"));
}


void Settings_New::on_pushButton_ApplyLessonPath_clicked()
{
	Json::ChangeValue(L"LessonInfoFile", ui.lineEdit_2->text().toStdString(), DEFAULT_CONFIG_PATH);
	QMessageBox::information(this, QString::fromStdWString(L"success"), QString::fromStdWString(L"success"));
}


void Settings_New::on_pushButton_restoreConfigPath_clicked()
{
	std::wstring path;
	Json::GetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
	ui.lineEdit->setText(QString::fromStdWString(path));
}


void Settings_New::on_pushButton_restoreLessonPath_clicked()
{
	std::wstring path;
	Json::GetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
	ui.lineEdit_2->setText(QString::fromStdWString(path));
}


void Settings_New::on_lineEdit_BackgroundImg_textChanged(const QString &arg1)
{
    GetCurrentWindow()->msBackGroundImg=arg1.toStdWString();
}


void Settings_New::on_pushButton_ChooseBackgroundImg_clicked()
{
    QString openFileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"请选择图片"), QString::fromStdWString(L".\\"), QString::fromStdWString(L"图片文件(*.jpg;*.png);;All(*.*)"));
    if (openFileName.isEmpty()) {
        QMessageBox::warning(this, QString::fromStdWString(L"提示"), QString::fromStdWString(L"未选择文件"));
    }
    else {
        ui.lineEdit_BackgroundImg->setText(openFileName);
    }

}


void Settings_New::on_checkBox_debug_stateChanged(int arg1)
{
	m_debug = arg1;
}

