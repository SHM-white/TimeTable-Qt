#include "TextFormat.h"

Json::Value TextFormat::GetJsonValue()
{
	Json::Value formatItem;
	Json::Value valueColor;
	Json::Value location;
	valueColor.append(GetRValue(color));
	valueColor.append(GetBValue(color));
	valueColor.append(GetGValue(color));
	location.append((int)mpTextLocation.x);
	location.append((int)mpTextLocation.y);
	formatItem.append(msTextFormat);
	formatItem.append(msFontName);
	formatItem.append(location);
	formatItem.append(miTextSize);
	formatItem.append(valueColor);
	formatItem.append((int)textType);
	return formatItem;
}
