#pragma once

#include<string>
#include<string_view>
#include<format>
#include<vector>
#include "resource.h"
#include<Windows.h>
#include"Lesson.h"

class CSVEditor
{
public:
	CSVEditor() = default;
	CSVEditor(const std::string Path) {
		mSetFilePath(Path);
		data.push_back(std::vector<std::string>());
	}
	int mSetFilePath(const std::string Path) {
		mCsvPath = Path;
		return 0;
	}
	int mGetLineCount();
	int mGetCsvData();
	//read:[line][order]
	const std::vector<std::string>& operator[](int line);
private:
	//std::vector<std::string> line;
	std::vector<std::vector<std::string>> data;
	std::string mCsvPath;
};

