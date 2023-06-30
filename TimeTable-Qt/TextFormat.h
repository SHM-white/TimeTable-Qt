#pragma once
#include "include.h"
#include "resource.h"
#include"Lesson.h"

enum TextType :int
{
	CurrentTime = 0,
	CurrentLesson = 1,
	CountDownDay = 2,
	Info = 3,
	Weather = 4,
	Default = 5
};
class TextItem
{
public:
	Json::Value getJsonValue() const;
	std::string text;
	TextType type;
};
class TextFormat {
public:
	TextFormat(const Json::Value& value);
	TextFormat() = default;
	Json::Value GetJsonValue() const;
	POINT mpTextLocation{ 0,0 };
	int miTextSize{ 20 };
	std::vector<TextItem> Texts;
	std::string msFontName{ "Consolas" };
	COLORREF color{ 0x00000000 };
private:
	
};