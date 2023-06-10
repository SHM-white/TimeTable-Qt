#pragma once
#include<string>
#include<string_view>
#include<format>
#include<vector>
#include "resource.h"
#include<Windows.h>
#include"Lesson.h"

enum TextType :int
{
	CurrentTime = 0,
	CurrentLesson = 1,
	CountDownDay = 2,
	Info = 3,
};

class TextFormat {
public:
	TextFormat(int x, int y, int TextSize, const std::string& FontName, const std::string& Text, COLORREF Color,TextType type) {
		mSetTextFormat(x, y, TextSize, FontName, Text, Color,type);
	};
	TextFormat() = default;
	int mSetTextFormat(int x, int y, int TextSize, const std::string& FontName, const std::string& Text, COLORREF Color,TextType type) {
		mpTextLocation.x = x;
		mpTextLocation.y = y;
		miTextSize = TextSize;
		msFontName = FontName;
		msTextFormat = Text;
		color = Color;
		textType = type;
		return 0;
	}
	Json::Value GetJsonValue() const;
	POINT mpTextLocation{ 0,0 };
	int miTextSize{ 20 };
	std::string msFontName{ "Consolas" };
	std::string msTextFormat{ "error:" };
	COLORREF color{ 0x00000000 };
	TextType textType{ 0 };
};