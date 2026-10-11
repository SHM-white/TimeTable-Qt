#pragma once
#include <QDateTime>
#include <QByteArray>
#include "ElementLayout.h"
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QRect>
#include <QSet>
#include <QString>
#include <QVector>

class QDialog;
class QListWidget;
class QWidget;

namespace tt {
class MainWindow;
class OverlayCard;

// Independently persistent desktop windows; the full planner remains an optional detail view.
struct OverlayConfig {
    QString id;
    QString title;
    QString type = "current";
    QString theme = "light";
    QString note;
    QList<ElementDefinition> elements;
    QString backgroundImage;
    QByteArray backgroundAsset; // Portable backups retain image bytes without external files.
    QColor backgroundColor = QColor("#FFFFFF");
    bool imageBackground = false;
    bool autoSize = false;
    QRect geometry;
    QRect compactGeometry;
    QRect expandedGeometry;
    bool collapsed = true;
    bool compactShowTitle = false;
    QString combinedItems = "clock,current,next";
    bool cycleItems = false;
    int cycleSeconds = 5;
    QDateTime target;
    bool visible = true;
    bool topmost = true;
    bool locked = false;
    bool showDetailsOnClick = true;
    int fontSize = 13;
    int opacity = 94;
    int fps = 60; // 0 disables continuous animation; schedule/model updates continue.
    bool debug = false;
    bool showFPS = false;
    int elementInset = 5;
    int cornerRadius = 17;
    bool acrylic = false;
};

class DesktopOverlays final : public QObject {
public:
    explicit DesktopOverlays(MainWindow* planner);
    ~DesktopOverlays() override;
    void start();
    void openConfiguration();
    void openPlanner(int page = 0);
    void refresh();
    void showDetail(const QString& id, const QPoint& anchor);
    void contextMenu(const QString& id, const QPoint& anchor);
    void moved(const QString& id, const QRect& geometry);
    void toggleCollapsed(const QString& id);
    void editDutyNotes();
    void importLegacyWindows();
    void editElements(const QString& id);
    void showAll();
    void hideAll();
    void setStartupVisible(const QString& id, bool visible);
    QString configurationPath() const;
    bool exportLayout(const QString& path, QString* error = nullptr) const;
    bool restoreLayout(const QString& path, QString* error = nullptr);
    bool setConfigurationPath(const QString& path, QString* error = nullptr);
    const QVector<OverlayConfig>& configurations() const { return configs_; }
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    friend class OverlayCard;
    MainWindow* planner_;
    QVector<OverlayConfig> configs_;
    QHash<QString, OverlayCard*> windows_;
    QPointer<QDialog> configurationDialog_;
    QPointer<QListWidget> configurationList_;
    QString configurationPath_;
    bool temporarilyHidden_ = false;
    bool temporarilyShowAll_ = false;
    QSet<QString> temporarilyHiddenWindows_;
    bool storageWritable_ = true;

    int indexOf(const QString& id) const;
    OverlayConfig defaultConfig(const QString& type) const;
    void load();
    void save() const;
    void rebuild();
    void edit(const QString& id = {});
    void remove(const QString& id);
    void refreshConfigurationList();
    void syncVisibility();
    bool shouldShow(const OverlayConfig& config) const;
    bool hasRecoverableCards() const;
    void autoResized(const QString& id, const QRect& geometry);
    QString detailsText(const OverlayConfig& config) const;
};
} // namespace tt
