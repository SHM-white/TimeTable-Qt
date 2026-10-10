#pragma once

#include "ScheduleStore.h"
#include <QMainWindow>

class QCheckBox;
class QDateEdit;
class QFrame;
class QLabel;
class QListWidget;
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
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    ScheduleStore store_;
    QString dataPath_;
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
    QListWidget* exceptionList_=nullptr;
    QDateEdit* changeDate_=nullptr;
    QDateEdit* sourceDate_=nullptr;
    QCheckBox* holiday_=nullptr;

    QDateEdit* semesterDate_=nullptr;
    QSpinBox* weeksSpin_=nullptr;
    QCheckBox* alwaysOnTop_=nullptr;
    QCheckBox* darkMode_=nullptr;
    QTableWidget* periodsTable_=nullptr;
    QSystemTrayIcon* tray_=nullptr;

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
    void saveAndRefresh();
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
