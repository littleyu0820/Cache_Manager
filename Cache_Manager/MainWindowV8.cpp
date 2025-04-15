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
#include <QLineEdit>
#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include "Icon_Cache.h"
#include <QStringList>
std::mutex usageTimeMutex;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
//Main
{
    LoadingCache();

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
    QWidget* WelcomePage = new QWidget(this); //頁面
    QVBoxLayout* WelcomeLayout = new QVBoxLayout(); //布局
    //////////////////////////////首頁配置//////////////////////////////


    //////////////////////////////介紹//////////////////////////////
    QLabel* WelcomeLabel = new QLabel("歡迎來到Cache Manager！\n本程式致力於改使用者的電腦體驗，通過這個小工具，\n讓使用者可以更快的打開自己常用的軟體。\n"
                                      "\n希望大家會喜歡這個程式! ~流雲 2025/04/08", this);
    WelcomeLabel->setFont(textFont);
    WelcomeLabel->setAlignment(Qt::AlignHCenter);
    WelcomeLabel->setStyleSheet("color: blue;");
    //////////////////////////////介紹//////////////////////////////


    //////////////////////////////配置開始按鈕//////////////////////////////
    QPushButton* StartButton = new QPushButton("開始使用", this);
    StartButton->setFont(textFont);
     //////////////////////////////配置開始按鈕//////////////////////////////


    //////////////////////////////UI統整//////////////////////////////
    WelcomeLayout->addWidget(WelcomeLabel); //添加UI
    WelcomeLayout->addWidget(StartButton); //添加UI
    WelcomePage->setLayout(WelcomeLayout); //配置
    //////////////////////////////UI統整//////////////////////////////



    //////////////////////////////配置紀錄頁面//////////////////////////////
    QWidget* RecordPage = new QWidget(this);
    QVBoxLayout *recordLayout = new QVBoxLayout();
    //////////////////////////////配置紀錄頁面//////////////////////////////


    //////////////////////////////過場//////////////////////////////
    QFont logging_font("Roboto", 12);
    QLabel* Label = new QLabel("登錄中...", this);
    Label->setFont(logging_font);
    Label->setAlignment(Qt::AlignLeft);
    Label->setWordWrap(true);
    //////////////////////////////過場//////////////////////////////


    //////////////////////////////配置開啟應用程式按鈕//////////////////////////////

    //////////////////////////////配置開啟應用程式按鈕//////////////////////////////


    //////////////////////////////配置下一頁按鈕//////////////////////////////
    QPushButton* NextPageBtn = new QPushButton("自定義常用程式", this);
    NextPageBtn->setFont(textFont);
    //////////////////////////////配置下一頁按鈕//////////////////////////////


    //////////////////////////////UI統整//////////////////////////////
    recordLayout->addWidget(Label);
    QVBoxLayout* AppBtnsLayout = new QVBoxLayout();
    SetUpButtons(AppBtnsLayout);

    //recordLayout->addWidget(AppsLabel);

    recordLayout->addLayout(AppBtnsLayout);
    recordLayout->addWidget(NextPageBtn);
    RecordPage->setLayout(recordLayout);
    //////////////////////////////UI統整//////////////////////////////


    //////////////////////////////自定義頁面//////////////////////////////
    // QLabel* Custom_Label = new QLabel("歡迎來到自定義頁面!", this);
    // Custom_Label->setFont(logging_font);
    // Custom_Label->setAlignment(Qt::AlignLeft);
    // Custom_Label->setStyleSheet("color: blue;");
    // Custom_Label->setWordWrap(true);

    QLineEdit *appNameLineEdit = new QLineEdit(this);
    QLineEdit *appPathLineEdit = new QLineEdit(this);
    appPathLineEdit->setReadOnly(true); // 路徑欄位只讀

    QPushButton* BrowseAppsBtn = new QPushButton("選擇程式", this);
    QPushButton* AddAppsBtn = new QPushButton("添加程式", this);

    // 使用 FormLayout 排版名稱與路徑欄位
    QFormLayout *formLayout = new QFormLayout();
    formLayout->addRow("自定義名稱：", appNameLineEdit);
    formLayout->addRow("選擇程式路徑：", appPathLineEdit);

    // 按鈕橫向排列
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(BrowseAppsBtn);
    buttonLayout->addWidget(AddAppsBtn);

    QVBoxLayout* CustomBtnsLayout = new QVBoxLayout();
    SetUpCustomButtons(CustomBtnsLayout);

    // 主垂直排版（包住上面兩個 Layout）
    QVBoxLayout *CustomLayout = new QVBoxLayout();
    CustomLayout->addLayout(formLayout);
    CustomLayout->addLayout(CustomBtnsLayout);
    CustomLayout->addLayout(buttonLayout);
    CustomLayout->setSpacing(15);  // 控制內部元件間距
    CustomLayout->setContentsMargins(20, 20, 20, 20);  // 控制外圍邊距

    QWidget *CustomPage = new QWidget(this);
    CustomPage->setLayout(CustomLayout);


    appPathLineEdit->setReadOnly(true);  // 讓路徑欄位為只讀


    connect(BrowseAppsBtn, &QPushButton::clicked, [=]()
    {
        QString filePath = QFileDialog::getOpenFileName(this, "選擇程式", "", "執行檔 (*.exe)");
        if (!filePath.isEmpty())
        {
            appPathLineEdit->setText(filePath);  // 顯示選擇的程式路徑
        }
    });

    connect(AddAppsBtn, &QPushButton::clicked, [=]()
    {
        QString AppsName = appNameLineEdit->text();
        if(AppsName.isEmpty() || appPathLineEdit->text().isEmpty())
        {
            QMessageBox::warning(this, "錯誤", "請填寫完整的自定義名稱和程式路徑！");
            return;
        }
        CustomInfo[AppsName.toStdWString()] = (appPathLineEdit->text()).toStdWString();
        SaveCustomData_Json(CustomInfo);
        QLayoutItem* item;
        while ((item = CustomBtnsLayout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        SetUpCustomButtons(CustomBtnsLayout);

    });


    //////////////////////////////配置首頁與分頁//////////////////////////////
    stackedWidget->addWidget(WelcomePage);
    stackedWidget->addWidget(RecordPage);
    stackedWidget->addWidget(CustomPage);
    //////////////////////////////配置首頁與分頁//////////////////////////////



    //////////////////////////////頁面選擇//////////////////////////////
    setCentralWidget(stackedWidget);
    //////////////////////////////頁面選擇//////////////////////////////



    //////////////////////////////讀取資料//////////////////////////////
    //usageTime = LoadUsageDataFromJson();
    //////////////////////////////讀取資料//////////////////////////////




    //////////////////////////////開啟應用程式//////////////////////////////

    // if(Apps_Rank.size() > 0)
    // {
    //     connect(OpenApps[0], &QPushButton::clicked, [=]()
    //             {
    //                 ShellExecuteW(NULL, L"runas", AppInfo[show_apps[0].first].c_str(), NULL, NULL, SW_SHOWNORMAL);
    //             });
    //     connect(OpenApps[1], &QPushButton::clicked, [=]()
    //             {
    //                 ShellExecuteW(NULL, L"runas", AppInfo[show_apps[1].first].c_str(), NULL, NULL, SW_SHOWNORMAL);
    //             });
    //     connect(OpenApps[2], &QPushButton::clicked, [=]()
    //             {
    //                 ShellExecuteW(NULL, L"runas", AppInfo[show_apps[2].first].c_str(), NULL, NULL, SW_SHOWNORMAL);
    //             });
    //     connect(OpenApps[3], &QPushButton::clicked, [=]()
    //             {
    //                 ShellExecuteW(NULL, L"runas", AppInfo[show_apps[3].first].c_str(), NULL, NULL, SW_SHOWNORMAL);
    //             });
    //     connect(OpenApps[4], &QPushButton::clicked, [=]()
    //             {
    //                 ShellExecuteW(NULL, L"runas", AppInfo[show_apps[4].first].c_str(), NULL, NULL, SW_SHOWNORMAL);
    //             });
    // }
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
    connect(NextPageBtn, &QPushButton::clicked, [=]()
    {
        stackedWidget->setCurrentIndex(2);
        this->resize(480, 600);
        CustomSetting();
    });
    //////////////////////////////停止線程//////////////////////////////
}


//function
void MainWindow::StartTracking(std::map<std::wstring, unsigned int>& UsageTime, std::wstring& CurrentApp, QLabel* Label)
{

    //////////////////////////////紀錄開始時間//////////////////////////////
    //auto Start_Time = std::chrono::steady_clock::now();
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
                if(AppInfo.find(active) == AppInfo.end())
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
void MainWindow::CustomSetting()
{

}

void MainWindow::LoadingCache()
{

    QStringList  paths;
    for(const auto& element : AppInfo)
    {
        QString path = QString::fromStdWString(element.second);
        paths << path;
    }
    AppIconCache::instance().preload_Icon(paths);


}



void MainWindow::SetUpButtons(QVBoxLayout* AppBtnsLayout)
{
    std::map<std::wstring, unsigned int> Apps_Rank = LoadUsageDataFromJson();
    auto show_apps = AppsRank(Apps_Rank);

    if (!show_apps.empty())
    {
        int index = 0;
        for (auto& app : show_apps)
        {
            if (index >= 5)
                break;

            QPushButton* appButton = new QPushButton(QString::fromStdWString(app.first), this);
            appButton->setIcon(AppIconCache::instance().getIcon(QString::fromStdWString(AppInfo[app.first])));
            appButton->setFont(QFont("Segoe UI", 12));
            AppBtnsLayout->addWidget(appButton);

            // 設置按鈕點擊事件
            connect(appButton, &QPushButton::clicked, [=]()
                    {
                        auto it = AppInfo.find(app.first);
                        if(it != AppInfo.end())
                            ShellExecuteW(NULL, L"runas", AppInfo[app.first].c_str(), NULL, NULL, SW_SHOWNORMAL);
                        else
                            QMessageBox::warning(this, "錯誤", "找不到對應的程式路徑！");
                    });

            ++index;
        }
    }
    else
    {
        QLabel* noAppsLabel = new QLabel("沒有可啟動的應用程式", this);
        AppBtnsLayout->addWidget(noAppsLabel);
    }
}


void MainWindow::SetUpCustomButtons(QVBoxLayout* CustomBtnsLayout)
{
    if(!CustomInfo.empty())
    {
        int index = 0;
        for(auto& customapps : CustomInfo)
        {
            if(index > 9)
                break;
            QPushButton* CustomButton = new QPushButton(QString::fromStdWString(customapps.first), this);
            CustomButton->setIcon(AppIconCache::instance().getIcon(QString::fromStdWString(CustomInfo[customapps.first])));
            CustomButton->setFont(QFont("Segoe UI", 12));
            CustomBtnsLayout->addWidget(CustomButton);
            connect(CustomButton, &QPushButton::clicked, [=]()
                    {
                        auto it = CustomInfo.find(customapps.first);
                        if(it != CustomInfo.end())
                            ShellExecuteW(NULL, L"runas", CustomInfo[customapps.first].c_str(), NULL, NULL, SW_SHOWNORMAL);
                        else
                            QMessageBox::warning(this, "錯誤", "找不到對應的程式路徑！");
                    });



            ++index;
        }
    }

    else
    {
        QLabel* noAppsLabel = new QLabel("沒有可啟動的應用程式", this);
        CustomBtnsLayout->addWidget(noAppsLabel);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

