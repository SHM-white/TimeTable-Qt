#pragma once

#include "include.h"
#include "Lesson.h"

class CSVEditor
{
public:
	CSVEditor() = default;
	CSVEditor(const std::wstring Path) {
		mSetFilePath(Path);
		data.push_back(std::vector<std::wstring>());
	}
	int mSetFilePath(const std::wstring Path) {
		mCsvPath = Path;
		return 0;
	}
	int mGetLineCount();
	int mGetCsvData();
	//read:[line][order]
	const std::vector<std::wstring>& operator[](int line);
private:
	//std::vector<std::wstring> line;
	std::vector<std::vector<std::wstring>> data;
	std::wstring mCsvPath;
};

