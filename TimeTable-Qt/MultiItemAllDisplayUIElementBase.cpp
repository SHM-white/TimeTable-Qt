#include "MultiItemAllDisplayUIElementBase.h"

QSize MultiItemAllDisplayUIElementBase::getNeededSize() const
{
    return QSize();
}

Json::Value MultiItemAllDisplayUIElementBase::SaveAsJson(Json::Value& value) const
{
    return value;
}
