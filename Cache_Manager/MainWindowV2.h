#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
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
    void StartTracking(std::map<std::wstring, unsigned int>&, std::wstring&, QLabel*);
    void StopTracking();

private:
    Ui::MainWindow *ui;
    std::map<std::wstring, unsigned int> UsageTime = LoadUsageDataFromJson();
    std::wstring CurrentApp = L"";
    bool StartorNot = true;  // 控制是否記錄
    unsigned int interval = 1; // 記錄的時間間隔
    unsigned int saveornot = 0;
    unsigned int saveinterval = 10;
    QTimer* timer = new QTimer(this);
};
#endif // MAINWINDOW_H
