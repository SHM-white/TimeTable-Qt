#include "include.h"

bool Json::ChangeValue(const std::string& Item, const Json::Value& value, const std::string& filePath)
{
	Json::Value root = GetRootJsonValue(filePath);
	root[Item] = value;
	SaveJson(filePath, root);
    return true;
}

Json::Value Json::GetRootJsonValue(const std::string& TargetPath)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(TargetPath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	reader.parse(in, root);
	return root;
}

int Json::SaveJson(const std::string& TargetPath, const Json::Value& root)
{
	std::fstream os(TargetPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }
	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	return 1;
}

int Json::mGetTextItem(const std::string& Item, std::string& input, const std::string& filePath)
{
	std::ifstream in(filePath, std::ios::in);
	if (!in.is_open())
	{
		return 0;
	};
	Json::Reader reader;
	Json::Value root;
	reader.parse(in, root);
	const Json::Value Text = root[Item];
	input = Text.asString();
	in.close();
	return 1;
}

std::string Json::mGetTextItem(const std::string& Item, const std::string& filePath, int)
{
	std::string value("");
	if (Json::mGetTextItem(Item, value, filePath)) {
		return value;
	}
	else {
		throw std::exception("error");
	}
}
