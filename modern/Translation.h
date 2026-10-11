#pragma once
#include <QCoreApplication>
#include <QJsonObject>
#include <QTranslator>

namespace tt {
inline QString translate(const char* source) {
    return QCoreApplication::translate("TimeTableNext",source);
}

// Chinese source strings remain the fallback; the English catalog is bundled.
class AppTranslator final : public QTranslator {
public:
    explicit AppTranslator(QObject* parent=nullptr);
    bool activate(const QString& language);
    bool isEmpty() const override { return catalog_.isEmpty(); }
    QString translate(const char* context,const char* source,
                      const char* disambiguation=nullptr,int count=-1) const override;
private:
    QJsonObject catalog_;
    bool installed_=false;
};
}
