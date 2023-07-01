#include "TextFormat.h"

TextFormat::TextFormat(const Json::Value& value)
{
	//Format[2][0].asInt(), Format[2][1].asInt(), Format[3].asInt(), Format[1].asString(), Format[0].asString(), RGB(Format[4][0].asInt(), Format[4][1].asInt(), Format[4][2].asInt()), (TextType)Format[5].asInt()
	mpTextLocation.x = value[1][0].asInt();
	mpTextLocation.y = value[1][1].asInt();
	miTextSize = value[2].asInt();
	msFontName = value[0].asString();
	
	color = RGB(value[3][0].asInt(), value[3][1].asInt(), value[3][2].asInt());
	
}

Json::Value TextFormat::GetJsonValue() const
{
	Json::Value formatItem;
	Json::Value valueColor;
	Json::Value location;
	Json::Value textItem;
	valueColor.append(GetRValue(color));
	valueColor.append(GetBValue(color));
	valueColor.append(GetGValue(color));
	location.append((int)mpTextLocation.x);
	location.append((int)mpTextLocation.y);
	for (const auto& i : Texts) {
		textItem.append(i.getJsonValue());
	}
	formatItem.append(msFontName);
	formatItem.append(location);
	formatItem.append(miTextSize);
	formatItem.append(valueColor);
	formatItem.append(textItem);
	return formatItem;
}

TextItem TextFormat::getCurrentText() const
{
	if (Texts.empty()) {
		return TextItem();
	}
	return Texts[updateCounter];
}

bool TextFormat::update()
{
	if (!(updateCounter < Texts.size())) {
		updateCounter = 0;
	}
	else {
		++updateCounter;
	}
	return true;
}

Json::Value TextItem::getJsonValue() const
{
	Json::Value value;
	value.append(text);
	value.append((int)type);
	return value;
}
