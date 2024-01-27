#include "MainWindow.h"


MainWindow::MainWindow(Json::Value& settings, QWidget* parent)
	: BasicWindow(settings, parent)
{
	ui.setupUi(this);
    ui.label_version->setText(QString::fromStdString(std::format("v{}.{}.{}", currentVersion_global[0], currentVersion_global[1], currentVersion_global[2])));
    this->show();
    std::wstring path;
    Json::mGetTextItem(L"ConfigFile", path, DEFAULT_CONFIG_PATH);
    auto value = Json::GetRootJsonValue(path);
    for (auto& i : value["Windows"]) {
        
    }
    Json::mGetTextItem(L"LessonInfoFile", path, DEFAULT_CONFIG_PATH);
}

MainWindow::~MainWindow()
{}

Json::Value MainWindow::SaveJson(Json::Value & value) const
{
	return Json::Value();
}
