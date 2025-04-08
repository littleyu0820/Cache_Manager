#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include "Main_System.h"
#include <QtConcurrent>
#include <mutex>
#include <QStackedWidget>
std::mutex usageTimeMutex;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
//Main
{
    ui->setupUi(this);
    //配置UI

    //////////////////////////////設置標題//////////////////////////////
    setWindowTitle("Cache Manager");
    QFont titleFont("Arial", 16, QFont::Bold);
    QFont textFont("Segoe UI", 12);
    //////////////////////////////設置標題//////////////////////////////


    //////////////////////////////設置頁面//////////////////////////////
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    //////////////////////////////設置頁面//////////////////////////////


    //////////////////////////////首頁配置//////////////////////////////
    QWidget *WelcomePage = new QWidget(this); //頁面
    QVBoxLayout *WelcomeLayout = new QVBoxLayout(); //布局
    //////////////////////////////首頁配置//////////////////////////////


    //////////////////////////////介紹//////////////////////////////
    QLabel *WelcomeLabel = new QLabel("歡迎來到Cache Manager！\n本程式致力於改使用者的電腦體驗，通過這個小工具，\n讓使用者可以更快的打開自己常用的軟體。\n"
                                      "\n希望大家會喜歡這個程式! ~流雲 2025/04/08", this);
    WelcomeLabel->setFont(textFont);
    WelcomeLabel->setAlignment(Qt::AlignHCenter);
    WelcomeLabel->setStyleSheet("color: blue;");
    //////////////////////////////介紹//////////////////////////////


    //////////////////////////////配置開始按鈕//////////////////////////////
    QPushButton *StartButton = new QPushButton("開始使用", this);
    StartButton->setFont(textFont);
     //////////////////////////////配置開始按鈕//////////////////////////////


    //////////////////////////////UI統整//////////////////////////////
    WelcomeLayout->addWidget(WelcomeLabel); //添加UI
    WelcomeLayout->addWidget(StartButton); //添加UI
    WelcomePage->setLayout(WelcomeLayout); //配置
    //////////////////////////////UI統整//////////////////////////////



    //////////////////////////////配置紀錄頁面//////////////////////////////
    QWidget *RecordPage = new QWidget(this);
    QVBoxLayout *recordLayout = new QVBoxLayout();
    //////////////////////////////配置紀錄頁面//////////////////////////////


    //////////////////////////////過場//////////////////////////////
    QFont logging_font("Roboto", 12);
    QLabel *Label = new QLabel("登錄中...", this);
    Label->setFont(logging_font);
    Label->setAlignment(Qt::AlignLeft);
    Label->setWordWrap(true);
    //////////////////////////////過場//////////////////////////////


    //////////////////////////////配置停止按鈕//////////////////////////////
    QPushButton *StopButton = new QPushButton("停止記錄", this);
    StopButton->setFont(textFont);
    //////////////////////////////配置停止按鈕//////////////////////////////



    //////////////////////////////配置開啟應用程式按鈕//////////////////////////////

    //////////////////////////////配置開啟應用程式按鈕//////////////////////////////



    //////////////////////////////UI統整//////////////////////////////
    recordLayout->addWidget(Label);
    if(!LoadUsageDataFromJson().empty())
    {
        QLabel *AppsLabel;
        QVBoxLayout *AppButtonsLayout;
        QPushButton *OpenApps[5];

        AppsLabel = new QLabel("應用程式快速啟動：", this);
        AppsLabel->setFont(textFont);
        AppButtonsLayout = new QVBoxLayout();
        std::map<std::wstring, unsigned int> Apps_Rank = LoadUsageDataFromJson();
        auto show_apps = AppsRank(Apps_Rank);
        auto show_beg = show_apps.begin();
        for (int i = 0; i < 5; ++i)
        {
            OpenApps[i] = new QPushButton(QString::fromStdWString(show_beg->first));
            OpenApps[i]->setFont(textFont);
            AppButtonsLayout->addWidget(OpenApps[i]);
            ++show_beg;
        }
        recordLayout->addWidget(AppsLabel);
        recordLayout->addLayout(AppButtonsLayout);
    }
    recordLayout->addWidget(StopButton);
    RecordPage->setLayout(recordLayout);
    //////////////////////////////UI統整//////////////////////////////



    //////////////////////////////配置首頁與分頁//////////////////////////////
    stackedWidget->addWidget(WelcomePage);
    stackedWidget->addWidget(RecordPage);
    //////////////////////////////配置首頁與分頁//////////////////////////////



    //////////////////////////////頁面選擇//////////////////////////////
    setCentralWidget(stackedWidget);
    //////////////////////////////頁面選擇//////////////////////////////



    //////////////////////////////讀取資料//////////////////////////////
    //usageTime = LoadUsageDataFromJson();
    //////////////////////////////讀取資料//////////////////////////////




    //////////////////////////////開啟應用程式//////////////////////////////


    // connect(OpenApps[0], &QPushButton::clicked, [=]()
    // {

    // });
    // connect(OpenApps[1], &QPushButton::clicked, [=]()
    // {

    // });
    // connect(OpenApps[2], &QPushButton::clicked, [=]()
    // {

    // });
    // connect(OpenApps[3], &QPushButton::clicked, [=]()
    // {

    // });
    // connect(OpenApps[4], &QPushButton::clicked, [=]()
    // {

    // });
    //////////////////////////////開啟應用程式//////////////////////////////



    //////////////////////////////開始線程//////////////////////////////
    connect(StartButton, &QPushButton::clicked, [=]()
    {
        stackedWidget->setCurrentIndex(1);
        this->resize(480, 600);
        StartTracking(UsageTime, CurrentApp, Label);
    });
    //////////////////////////////開始線程//////////////////////////////



    //////////////////////////////停止線程//////////////////////////////
    connect(StopButton, &QPushButton::clicked, [=]()
    {
        if(StartorNot)
        {
            StopTracking();
            StopButton->setText("重新啟動!");
        }
        else
        {
            StartorNot = true;
            timer->start(1000); //每秒更新
            StopButton->setText("停止紀錄!");
        }

    });
    //////////////////////////////停止線程//////////////////////////////
}


//function
void MainWindow::StartTracking(std::map<std::wstring, unsigned int>& UsageTime, std::wstring& CurrentApp, QLabel* Label)
{

    //////////////////////////////紀錄開始時間//////////////////////////////
    auto Start_Time = std::chrono::steady_clock::now();
    //////////////////////////////紀錄開始時間//////////////////////////////


    //////////////////////////////開啟主線程//////////////////////////////
    connect(timer, &QTimer::timeout, [=]() mutable
    {

        //////////////////////////////關閉主線程//////////////////////////////
        if(!StartorNot)
        {
            SaveData_Json(UsageTime);
            timer->stop();
            return;
        }
        //////////////////////////////關閉主線程//////////////////////////////



        //////////////////////////////開啟子線程//////////////////////////////
        QtConcurrent::run([=, &UsageTime, &CurrentApp]()
        {
            //////////////////////////////更新//////////////////////////////
            std::wstring active = GetActiveProcessName();
            //qDebug() << active;
            if (!active.empty() && active != L"Program Manager")
            {
                if(UsageTime.find(active) == UsageTime.end())
                {
                    auto path = GetPath();
                    AppInfo[active] = path;
                    SaveData_Path(AppInfo);
                }

                std::lock_guard<std::mutex> lock(usageTimeMutex);
                if(active == CurrentApp)
                    UsageTime[active] += interval;
                else
                {
                    CurrentApp = active;
                    UsageTime[active] += interval;
                }
            }
            //////////////////////////////更新//////////////////////////////
        });
        //////////////////////////////開啟子線程//////////////////////////////



        //////////////////////////////判斷是否儲存//////////////////////////////
        ++saveornot;
        if (saveornot > saveinterval)
        {
            std::lock_guard<std::mutex> lock(usageTimeMutex);
            SaveData_Json(UsageTime);
            qDebug() << "Data saved!";
            saveornot = 0;
        }
        //////////////////////////////判斷是否儲存//////////////////////////////


        //////////////////////////////排序//////////////////////////////
        std::vector<std::pair<std::wstring, unsigned int>> Sorted_Usage;
        {
            std::lock_guard<std::mutex> lock(usageTimeMutex);
            Sorted_Usage = std::vector<std::pair<std::wstring, unsigned int>>(UsageTime.begin(), UsageTime.end());
        }
        std::sort(Sorted_Usage.begin(), Sorted_Usage.end(), [](auto& a, auto& b){ return a.second > b.second; } ); //降序
        //////////////////////////////排序//////////////////////////////


        auto now = std::chrono::steady_clock::now(); //紀錄現在的時間
        auto Totally_Time = std::chrono::duration_cast<std::chrono::seconds>(now - Start_Time).count(); //與開始時間相減
        QString displayText = "使用時間統計：" + QString::number(Totally_Time) + "秒\n";

        unsigned int ctr = 0; //只取前五名
        std::wstring rank[] = { L"第一名: ", L"第二名: ", L"第三名: ", L"第四名: ", L"第五名: " };

        //////////////////////////////取資料//////////////////////////////
        for (auto& used : Sorted_Usage)
        {
            if (ctr < 5 && used.second >= 10) // 顯示大於等於10秒的應用程式
            {
                displayText += QString::fromStdWString(rank[ctr++]) + QString::fromStdWString(used.first) + ": " + QString::number(used.second) + " 秒\n";
            }
        }
        //////////////////////////////取資料//////////////////////////////


        Label->setText(displayText); //更新
    });


    timer->start(1000); //每秒更新

}
void MainWindow::StopTracking()
{
    StartorNot = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

