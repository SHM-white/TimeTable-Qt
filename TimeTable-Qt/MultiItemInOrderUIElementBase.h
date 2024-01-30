#pragma once
#include "UIElementBase.h"
class MultiItemInOrderUIElementBase :
    public UIElementBase
{
    // 通过 UIElementBase 继承
    QSize getNeededSize() const override;
    Json::Value SaveAsJson(Json::Value& value )const override;
};

