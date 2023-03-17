#pragma once
#include<string>
#include<string_view>
#include<format>
#include<vector>
#include "resource.h"
#include<Windows.h>
#include"Lesson.h"

class TextFormat {
public:
	TextFormat(int x, int y, int TextSize, const std::string& FontName, const std::string& Text, COLORREF Color) {
		mSetTextFormat(x, y, TextSize, FontName, Text, Color);
	};
	TextFormat() = default;
	int mSetTextFormat(int x, int y, int TextSize, const std::string& FontName, const std::string& Text, COLORREF Color) {
		mpTextLocation.x = x;
		mpTextLocation.y = y;
		miTextSize = TextSize;
		msFontName = FontName;
		msTextFormat = Text;
		color = Color;
		return 0;
	}
	Json::Value GetJsonValue();
	POINT mpTextLocation{ 0,0 };
	int miTextSize{ 20 };
	std::string msFontName{ "Consolas" };
	std::string msTextFormat{ "error:" };
	COLORREF color{ 0x00000000 };
};