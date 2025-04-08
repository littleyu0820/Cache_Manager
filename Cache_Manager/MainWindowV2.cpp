#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <map>
#include <string>
#include <algorithm>
#include "Main_System.h"
#include <windows.h>
#include <psapi.h>
#include <QtConcurrent>
#include <mutex>
std::mutex usageTimeMutex;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 設定顯示區域
    QVBoxLayout *layout = new QVBoxLayout();
    QLabel *timeLabel = new QLabel(this);
    layout->addWidget(timeLabel);

    QPushButton *startButton = new QPushButton("開始記錄!", this);
    layout->addWidget(startButton);

    QWidget *container = new QWidget();
    container->setLayout(layout);
    setCentralWidget(container);

    std::map<std::wstring, unsigned int> usageTime = LoadUsageDataFromJson();
    std::wstring currentApp = L"";

    unsigned int interval = 1; // 秒數
    auto Start_Time = std::chrono::steady_clock::now();
    QTimer *timer = new QTimer(this);
    unsigned int saveornot = 0, saveinterval = 10;



    connect(timer, &QTimer::timeout, [=]() mutable
    {

        QtConcurrent::run([=, &usageTime, &currentApp]()
        {
            //qDebug() << "Timer triggered!";
            std::wstring active = GetActiveProcessName();
            if (!active.empty())
            {
                std::lock_guard<std::mutex> lock(usageTimeMutex); // 上鎖
                if(active == currentApp)
                {
                    usageTime[active] += interval;
                    //qDebug() << QString::fromStdWString(active);
                }
                else
                {
                    qDebug() << QString::fromStdWString(active);
                    currentApp = active;
                    usageTime[active] += interval;
                }
            }

        });
        ++saveornot;
        if (saveornot > saveinterval)
        {
            std::lock_guard<std::mutex> lock(usageTimeMutex);
            SaveData_Json(usageTime);
            qDebug() << "Data saved!";
            saveornot = 0;
        }


        std::vector<std::pair<std::wstring, unsigned int>> Sorted_Usage;
        {
            std::lock_guard<std::mutex> lock(usageTimeMutex); // 保證安全讀取
            Sorted_Usage = std::vector<std::pair<std::wstring, unsigned int>>(usageTime.begin(), usageTime.end());
        }
        std::sort(Sorted_Usage.begin(), Sorted_Usage.end(),
                  [](auto& a, auto& b) { return a.second > b.second; } //auto = std::pair<std::wstring, int>&
                  );

        auto now = std::chrono::steady_clock::now();
        auto Totally_Time = std::chrono::duration_cast<std::chrono::seconds>(now - Start_Time).count();
        QString displayText = "使用時間統計：" + QString::number(Totally_Time) + "秒\n";

        unsigned int ctr = 0;
        std::wstring rank[] = { L"第一名: ", L"第二名: ", L"第三名: ", L"第四名: ", L"第五名: " };
        for (auto& used : Sorted_Usage)
        {
            //qDebug() << QString::fromStdWString(used.first) << " : " << used.second;
            if (ctr < 5 && used.second >= 10)
            {
                displayText += QString::fromStdWString(rank[ctr++]) + QString::fromStdWString(used.first) + ": " + QString::number(used.second) + " 秒\n";
            }
            //timeLabel->setText(displayText);
        }
        timeLabel->setText(displayText);

    });

    connect(startButton, &QPushButton::clicked, [=]()
    {
        timer->start(1000);  // 每秒更新
    });
}
MainWindow::~MainWindow()
{

    delete ui;
}


