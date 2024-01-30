#include "SingleItemUIElementBase.h"

QSize SingleItemUIElementBase::getNeededSize() const
{
    return QSize();
}

Json::Value SingleItemUIElementBase::SaveAsJson(Json::Value& value) const
{
    return Json::Value();
}
