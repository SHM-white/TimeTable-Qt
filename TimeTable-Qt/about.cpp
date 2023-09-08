#include "about.h"
#include "ui_about.h"
#include "TimeTableQt.h"
#include "include.h"
About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    TimeTableQt* p = (TimeTableQt*)parentWidget();
    std::wstring about;
    p->windowsettings->mGetTextItem(L"About", about);
    QString qabout{ QString::fromStdWString(about) };
    this->ui->textEdit->setText(qabout);
    this->ui->label_3->setText(QString::fromStdWString(std::format(L"v{}.{}.{}", p->windowsettings->currentVersion[0], p->windowsettings->currentVersion[1], p->windowsettings->currentVersion[2])));
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
#ifdef QT_DEBUG

    if (ui->textEdit->toPlainText().length() == 0) {
        TimeTableQt* pParent = (TimeTableQt*)parentWidget();
        std::wstring origin1 = pParent->windowsettings->mChangeConfigPath(L".\\test_config.json");
        std::wstring origin2;
        try
        {
            QMessageBox::information(this, QString("Test"), QString("开始测试所有函数"), QMessageBox::Ok);
            pParent->timetable->mGetCurrentTime(L"");
            QMessageBox::information(this, QString("Test"), QString("timetable->mGetCurrentTime(\"%0\")"), QMessageBox::Ok);
            pParent->windowsettings->mGetWindowSettings();
            origin2 = pParent->timetable->mReplacePath(pParent->windowsettings->msLessonInfoFile);
            QMessageBox::information(this, QString("Test"), QString("windowsettings->mGetWindowSettings()"), QMessageBox::Ok);
            pParent->windowsettings->save();
            QMessageBox::information(this, QString("Test"), QString("windowsettings->save()"), QMessageBox::Ok);
            pParent->timetable->mAddLesson(Lesson(L"Mon", L"6", 1000, 1100));
            pParent->timetable->mAddLesson(Lesson(L"Mon", L"2", 1000, 1100));
            pParent->timetable->mAddLesson(Lesson(L"Mon", L"8", 1000, 1100));
            pParent->timetable->mAddLesson(Lesson(L"Mon", L"4", 1000, 1100));
            QMessageBox::information(this, QString("Test"), QString("timetable->mAddLesson"), QMessageBox::Ok);
            pParent->timetable->changeLesson(0, L"Mon", Lesson(L"Mon", L"16", 1000, 1100));
            QMessageBox::information(this, QString("Test"), QString("timetable->changeLesson()"), QMessageBox::Ok);

            pParent->timetable->sortLessons();
            QMessageBox::information(this, QString("Test"), QString("timetable->sortLessons()"), QMessageBox::Ok);

            pParent->timetable->deleteLesson(1, L"Mon");
            QMessageBox::information(this, QString("Test"), QString("timetable->deleteLesson()"), QMessageBox::Ok);

            pParent->timetable->mAddMoreInfo(L"Mon", L"asdfgjzxcvbn test" + TimeTable::mGetCurrentTime(L"%H%M"));
            QMessageBox::information(this, QString("Test"), QString("timetable->mAddMoreInfo()"), QMessageBox::Ok);

        }
        catch(std::exception& ex){
            QMessageBox::warning(this, QString("error"), QString::fromStdString(ex.what()));
            pParent->windowsettings->mChangeConfigPath(origin1);
            pParent->timetable->mReplacePath(origin2);
            throw;
        }
        pParent->windowsettings->mChangeConfigPath(origin1);
        pParent->timetable->mReplacePath(origin2);
        QMessageBox::information(this, QString("Test"), QString("success!"), QMessageBox::Ok);

    }
    else {
#endif // DEBUG

        close();
#ifdef QT_DEBUG
    }
#endif // DEBUG
 
}


void About::on_commandLinkButton_clicked()
{
    QDesktopServices::openUrl(QUrl(QString("https://github.com/SHM-white/TimeTable")));
}


void About::on_commandLinkButton_2_clicked()
{
    QDesktopServices::openUrl(QUrl(QString("https://github.com/SHM-white/TimeTable-Qt")));
}

