#pragma once
#include "include.h"
#include "resource.h"
#include"Lesson.h"

enum TextType
{
	Default,
	CurrentTime,
	CurrentLesson,
	CountDownDay,
	Info,
	AllLessons,
	News,
	Weather,
};
class TextItem
{
public:
	Json::Value getJsonValue() const;
	std::wstring text;
	TextType type;
};
class TextFormat {
public:
	TextFormat(const Json::Value& value);
	TextFormat() = default;
	Json::Value GetJsonValue() const;
	TextItem getCurrentText() const;
	bool update();
	int miSizeW{ 0 };
	int miSizeH{ 0 };
	POINT mpTextLocation{ 0,0 };
	int miTextSize{ 20 };
	std::vector<TextItem> Texts;
	std::wstring msFontName{ L"Consolas" };
	COLORREF color{ 0x00000000 };
	int updateCounter{ 0 };
private:
	
};