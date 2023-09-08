#include "include.h"

bool Json::ChangeValue(const std::wstring& Item, const Json::Value& value, const std::wstring& filePath)
{
	Json::Value root = GetRootJsonValue(filePath);
    root[wtu8(Item)] = value;
	SaveJson(filePath, root);
    return true;
}

Json::Value Json::GetRootJsonValue(const std::wstring& TargetPath)
{
	Json::Reader reader;
	Json::Value root;
	std::ifstream in(TargetPath, std::ios::in);
	if ((!in.is_open())||(!reader.parse(in, root)))
	{
        root = Json::Value(Json::nullValue);
        return root;
	};
	return root;
}

int Json::SaveJson(const std::wstring& TargetPath, const Json::Value& root)
{
	std::fstream os(TargetPath, std::ios::out | std::ios::trunc);
	if (!os.is_open()) { return 0; }
	Json::StyledWriter sw;
	os << sw.write(root);
	os.close();
	return 1;
}


int Json::mGetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath)
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
    const Json::Value Text = root[wtu8(Item)];
    input = u8tw(Text.asString());

    return 1;
}

std::wstring Json::mGetTextItem(const std::wstring& Item, const std::wstring& filePath, int)
{
    std::wstring value;
    if (Json::mGetTextItem(Item, value, filePath)) {
        return value;
    } else {
        throw std::runtime_error("error");
    }
}

std::string ToolFunctions::wstringToU8string(const std::wstring& wstring)
{
    // Create a wstring converter for UTF-8 encoding
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    // Convert the wstring to UTF-8 encoded string and return it
    return converter.to_bytes(wstring);
}

std::wstring ToolFunctions::u8stringToWstring(const std::string& u8string)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(u8string);
}