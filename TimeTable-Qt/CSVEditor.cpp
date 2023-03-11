#include "CSVEditor.h"
#include "Timetable.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "include\json\json.h"
#include <format>

//获取csv行数
int CSVEditor::mGetLineCount()
{
	return (int)data.size();
}
//读取csv数据并存入成员二维数组中
int CSVEditor::mGetCsvData()
{
    std::fstream in;
	std::string line;
    in.open(mCsvPath, std::ios::in);
    if (!in.is_open()) {
		in.close();
        return 0;
    }
	data.clear();
	while (std::getline(in,line)) {
		std::string columElement;
		std::stringstream stream(line);
		data.push_back(std::vector<std::string>());
		while (getline(stream, columElement, ',')) {
			data.back().push_back(columElement);
		}
		//添加最后一个元素。由于读取最后一个空元素会跳出循环,例如a，空    只能读取到a，空出处没读到
		if (!stream && columElement.empty()) {
			data.back().push_back("");
		}
	}
	in.close();
    return 1;
}
//重载下标运算符
const std::vector<std::string>& CSVEditor::operator[](int line)
{
    return data[line];
}
