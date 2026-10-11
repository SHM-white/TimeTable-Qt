#pragma once

#include "ScheduleStore.h"
#include <QMainWindow>
#include <QSet>
#include <functional>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QFrame;
class QLabel;
class QListWidget;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTimer;
class QSystemTrayIcon;
class QVBoxLayout;

namespace tt {
class MainWindow final : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    const ScheduleStore& schedule() const { return store_; }
    QString dataPath() const { return dataPath_; }
    bool switchSchedulePath(const QString& path, QString* error = nullptr);
    bool saveScheduleAs(const QString& path, QString* error = nullptr);
    bool convertImportToJson(const QString& input, const QString& output, QString* error = nullptr) const;
    void setDayInfos(int weekday, const QStringList& messages) {
        if(weekday<1 || weekday>7) return;
        store_.dayInfos[weekday]=messages;
        saveAndRefresh();
    }
    void openPage(int index);
    void setOnScheduleChanged(std::function<void()> callback) { onScheduleChanged_ = std::move(callback); }
    void setOpenOverlaySettings(std::function<void()> callback) { onOpenOverlaySettings_ = std::move(callback); }
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    ScheduleStore store_;
    std::function<void()> onScheduleChanged_;
    std::function<void()> onOpenOverlaySettings_;
    QString dataPath_;
    // A failed load must never be replaced by the default / edited in-memory store.
    // Only an explicitly confirmed, successful import lifts this protection.
    bool canSaveSchedule_=true;
    int shownWeek_=1;
    bool dark_=false;
    bool reallyQuit_=false;

    QStackedWidget* stack_=nullptr;
    QVector<QPushButton*> navigation_;
    QLabel* clockLabel_=nullptr;
    QLabel* heroEyebrow_=nullptr;
    QLabel* heroCourse_=nullptr;
    QLabel* heroDetail_=nullptr;
    QLabel* heroCountdown_=nullptr;
    QLabel* nextLabel_=nullptr;
    QVBoxLayout* todayList_=nullptr;

    QLabel* weekLabel_=nullptr;
    QWidget* weekContent_=nullptr;
    QTableWidget* courseTable_=nullptr;
    QLineEdit* courseSearch_=nullptr;
    QLabel* conflictSummary_=nullptr;
    QListWidget* exceptionList_=nullptr;
    QDateEdit* changeDate_=nullptr;
    QDateEdit* sourceDate_=nullptr;
    QCheckBox* holiday_=nullptr;

    QDateEdit* semesterDate_=nullptr;
    QSpinBox* weeksSpin_=nullptr;
    QCheckBox* alwaysOnTop_=nullptr;
    QCheckBox* darkMode_=nullptr;
    QComboBox* language_=nullptr;
    QTableWidget* periodsTable_=nullptr;
    QSystemTrayIcon* tray_=nullptr;
    QLabel* schedulePathLabel_=nullptr;
    QComboBox* profiles_=nullptr;
    QCheckBox* remindersEnabled_=nullptr;
    QSpinBox* reminderMinutes_=nullptr;
    bool remind_=false;
    int minutesBefore_=10;
    QSet<QString> notifiedReminders_;

    QWidget* createDashboard();
    QWidget* createWeekPage();
    QWidget* createCoursesPage();
    QWidget* createChangesPage();
    QWidget* createSettingsPage();
    void buildUi();
    void buildTray();
    void switchPage(int index);
    void applyTheme();
    void updateDashboard();
    void updateWeek();
    void updateCourseTable();
    void updateChanges();
    void fillSettings();
    void saveAndRefresh(bool persist = true);
    void fillPathSettings();
    void chooseSchedulePath();
    void chooseSaveScheduleAs();
    void registerProfile();
    void convertImport();
    void checkReminders();
    void showConflicts();
    int selectedCourseIndex() const;
    void editCourse(int index=-1);
    void removeCourse();
    void duplicateCourse();
    void addDayChange();
    void removeDayChange();
    void importSchedule();
    void exportSchedule();
    void saveSettings();
    void showMessage(const QString& text);
};
} // namespace tt
