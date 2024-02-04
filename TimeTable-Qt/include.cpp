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


int Json::GetTextItem(const std::wstring& Item, std::wstring& input, const std::wstring& filePath)
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

std::wstring Json::GetTextItem(const std::wstring& Item, const std::wstring& filePath, int)
{
    std::wstring value;
    if (Json::GetTextItem(Item, value, filePath)) {
        return value;
    } else {
        throw std::runtime_error("error");
    }
}

Json::Value Json::GetJsonValueFromWStr(const std::wstring& str)
{
	return GetJsonValueFromStr(wtu8(str));
}

Json::Value Json::GetJsonValueFromStr(const std::string& str)
{
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(str, root)) {
		return Json::nullValue;
	}
	else
	{
		return root;
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

std::string ToolFunctions::wstringToAnsiString(const std::wstring& u8wstring)
{
    
    char ansiChar[1024];
    WideCharToMultiByte(CP_ACP, 0, u8wstring.c_str(), u8wstring.size(), ansiChar, sizeof(ansiChar) / sizeof(char), NULL, NULL);
    return ansiChar;
}

std::wstring GetCurrentVersion()
{
	return std::format(L"v{}.{}.{} Beta", currentVersion_global[0], currentVersion_global[1], currentVersion_global[2]);
}

std::wstring GetCompileTime()
{
	return u8tw(std::format("{} {}", __DATE__, __TIME__));
}

std::wstring GetWStrDay(Days day)
{
	std::wstring strDay;
	switch (day)
	{
	case Mon:
		strDay = L"Mon";
		break;
	case Tue:
		strDay = L"Tue";
		break;
	case Wed:
		strDay = L"Wed";
		break;
	case Thu:
		strDay = L"Thu";
		break;
	case Fri:
		strDay = L"Fri";
		break;
	case Sat:
		strDay = L"Sat";
		break;
	case Sun:
		strDay = L"Sun";
		break;
	case Null:
	default:
		strDay = L"Null";
		break;
	}
    return strDay;
}

COLORREF HexStringToColorRef(const std::wstring& input)
{
	int red;
	int blue;
	int green;
	int i = swscanf_s(input.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
	return RGB(red, green, blue);
}

std::wstring ColorRefToHexString(COLORREF color)
{
	int red = GetRValue(color);
	int blue = GetBValue(color);
	int green = GetGValue(color);
	return std::wstring(std::format(L"#{:02x}{:02x}{:02x}", red, green, blue));
}

QColor ColorRefToQColor(COLORREF color)
{
	return QColor(GetRValue(color), GetGValue(color), GetBValue(color));
}

COLORREF QColorToColorRef(QColor color)
{
	return RGB(color.red(),color.green(),color.blue());
}

QColor HexStringToQColor(const std::wstring& input)
{
	return ColorRefToQColor(HexStringToColorRef(input));
}

std::wstring QColorToHexString(QColor color)
{
	return ColorRefToHexString(QColorToColorRef(color));
}

