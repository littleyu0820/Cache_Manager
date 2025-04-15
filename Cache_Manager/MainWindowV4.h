#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include "Main_System.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void SetUpButtons(QVBoxLayout*);
    void StartTracking(std::map<std::wstring, unsigned int>&, std::wstring&, QLabel*);
    void CustomSetting();
    void LoadingCache();
    void SetUpCustomButtons(QVBoxLayout*);

private:
    Ui::MainWindow *ui;
    std::map<std::wstring, unsigned int> UsageTime = LoadUsageDataFromJson();
    std::map<std::wstring, std::wstring> AppInfo = LoadPathDataFromJson();
    std::map<std::wstring, std::wstring> CustomInfo = LoadCustomDataFromJson();
    std::wstring CurrentApp = L"";
    bool StartorNot = true;  // 控制是否記錄
    std::chrono::time_point<std::chrono::steady_clock> Start_Time = std::chrono::steady_clock::now();
    unsigned int interval = 1; // 記錄的時間間隔
    unsigned int saveornot = 0;
    unsigned int saveinterval = 10;
    QTimer* timer = new QTimer(this);
};
#endif // MAINWINDOW_H
