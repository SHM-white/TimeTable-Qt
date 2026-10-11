#pragma once
#include "DesktopOverlays.h"
#include <QJsonDocument>

namespace tt {
// Validates the entire document before replacing state; writes use QSaveFile.
class DesktopLayoutStore {
public:
    static QString defaultPath();
    static QString resolveBackground(const QString& image, const QString& configPath,
                                     const QString& executableDirectory = {});
    static QJsonObject toJson(const OverlayConfig& config);
    static bool parse(const QByteArray& bytes, QVector<OverlayConfig>* configs,
                      QString* error = nullptr, const QString& sourcePath = {});
    static bool read(const QString& path, QVector<OverlayConfig>* configs,
                     QString* error = nullptr);
    static bool write(const QString& path, const QVector<OverlayConfig>& configs,
                      QString* error = nullptr, bool embedImages = false);
    static bool fromLegacy(const QJsonObject& root, const QString& sourcePath,
                           QVector<OverlayConfig>* configs, int* skipped = nullptr,
                           QString* error = nullptr);
};
}
