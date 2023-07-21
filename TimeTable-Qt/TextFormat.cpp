#include "TextFormat.h"

TextFormat::TextFormat(const Json::Value& value)
{
	mpTextLocation.x = value[1][0].asInt();
	mpTextLocation.y = value[1][1].asInt();
	miTextSize = value[2].asInt();
	msFontName = value[0].asString();
	color = RGB(value[3][0].asInt(), value[3][1].asInt(), value[3][2].asInt());
	miSizeW = value[1][2].asInt();
	miSizeH = value[1][3].asInt();
	for (const auto& i : value[4]) {
		TextItem item;
		item.text = i[0].asString();
		item.type = (TextType)i[1].asInt();
		Texts.push_back(item);
	}
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
	location.append((int)miSizeW);
	location.append((int)miSizeH);
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
	if (Texts.empty()) {
		return false;
	}
	static int i = 0;
	i = (i + 1) % 20;
	if(i==0){
		++updateCounter;
	}
	if(updateCounter>=Texts.size()){
		updateCounter=0;
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
