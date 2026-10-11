#include "Translation.h"
#include <QFile>
#include <QJsonDocument>

static void initializeTranslationResources() {
    Q_INIT_RESOURCE(translations);
}

namespace tt {
AppTranslator::AppTranslator(QObject* parent):QTranslator(parent) {
    initializeTranslationResources();
}
bool AppTranslator::activate(const QString& language) {
    if(installed_)QCoreApplication::removeTranslator(this);
    installed_=false;
    catalog_={};
    if(language=="zh_CN" || language=="zh" || language.isEmpty())return true;
    if(language!="en")return false;
    QFile file(":/translations/en.json");
    if(!file.open(QIODevice::ReadOnly))return false;
    const auto document=QJsonDocument::fromJson(file.readAll());
    if(!document.isObject())return false;
    catalog_=document.object();
    installed_=QCoreApplication::installTranslator(this);
    return installed_;
}
QString AppTranslator::translate(const char* context,const char* source,const char*,int count) const {
    if(!context || QByteArray(context)!="TimeTableNext" || !source)return {};
    QString result=catalog_.value(QString::fromUtf8(source)).toString();
    if(count>=0)result.replace("%n",QString::number(count));
    return result;
}
}
