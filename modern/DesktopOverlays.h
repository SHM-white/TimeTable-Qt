#pragma once
#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QRect>
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
    QRect geometry;
    QRect compactGeometry;
    QRect expandedGeometry;
    bool collapsed = true;
    bool compactShowTitle = false;
    QString combinedItems = "clock,current,next";
    QDateTime target;
    bool visible = true;
    bool topmost = true;
    bool locked = false;
    bool showDetailsOnClick = true;
    int fontSize = 13;
    int opacity = 94;
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

private:
    friend class OverlayCard;
    MainWindow* planner_;
    QVector<OverlayConfig> configs_;
    QHash<QString, OverlayCard*> windows_;
    QPointer<QDialog> configurationDialog_;
    QPointer<QListWidget> configurationList_;

    int indexOf(const QString& id) const;
    OverlayConfig defaultConfig(const QString& type) const;
    void load();
    void save() const;
    void rebuild();
    void edit(const QString& id = {});
    void remove(const QString& id);
    void refreshConfigurationList();
    QString detailsText(const OverlayConfig& config) const;
};
} // namespace tt
