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
    std::ifstream in(filePath);
    if (!in.is_open()) {
        return 0;
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(in, root)){
		return 0;
	}
    const Json::Value Text = root[Item];
    input = Text.asString();

    return 1;
}

std::string Json::mGetTextItem(const std::string& Item, const std::string& filePath, int)
{
    std::string value;
    if (Json::mGetTextItem(Item, value, filePath)) {
        return value;
    } else {
        throw std::runtime_error("error");
    }
}

std::string Json::utf8ToGB2312(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(str);

    std::vector<char> buffer(wide.size() * 2);
    std::wcstombs(buffer.data(), wide.c_str(), buffer.size());

    std::string result(buffer.data(), buffer.data() + std::wcslen(wide.c_str()));

    return result;
}
