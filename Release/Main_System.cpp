#include "Main_System.h"
#include <windows.h>
#include <psapi.h>
#include "json.hpp"
#include <codecvt>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>

using json = nlohmann::json;


//////////////////////////////取得視窗標題//////////////////////////////
std::wstring GetProcessUserName()
{
    std::wstring username = L"Unknown";
    HANDLE hToken = NULL;
    DWORD size = 0;

    //////////////////////////////擴展緩衝區//////////////////////////////
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        BYTE* buffer = new BYTE[size];
        if (GetTokenInformation(hToken, TokenUser, buffer, size, &size))
        {
            TOKEN_USER* tokenUser = (TOKEN_USER*)buffer;
            WCHAR name[256], domain[256];
            DWORD nameLen = 256, domainLen = 256;
            SID_NAME_USE sidType;
            if (LookupAccountSidW(NULL, tokenUser->User.Sid, name, &nameLen, domain, &domainLen, &sidType))
            {
                username = domain;
                username += L"\\";
                username += name;
            }

            delete[] buffer;
        }

    }
    CloseHandle(hToken);

    return username;
}

std::wstring GetActiveProcessName()
{

    HWND hwnd = GetForegroundWindow(); //取得前景視窗
    if (!hwnd)
    {
        return L"";
    }

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid); //取得前景視窗ID


    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t)); //擷取視窗標題
    // HANDLE hProcess = NULL;
    // hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    // if (!hProcess)
    // {
    //     return L"";
    // }


    std::wstring user = GetProcessUserName();
    if (!user.empty())
    {
        return title;
    }


    //CloseHandle(hProcess);
    return L"";
}
//////////////////////////////取得視窗標題//////////////////////////////



//////////////////////////////儲存/存取路徑//////////////////////////////
std::wstring GetPath()
{
    HWND hwnd = GetForegroundWindow();
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) return L"";

    WCHAR path[MAX_PATH];
    if (GetModuleFileNameExW(hProcess, NULL, path, MAX_PATH))
    {
        CloseHandle(hProcess);
        return std::wstring(path);
    }

    CloseHandle(hProcess);
    return L"";
}

void SaveData_Path(const std::map<std::wstring, std::wstring>& appinfo)
{

    QJsonObject jsonObj;
    for (const auto& data : appinfo)
        jsonObj[QString::fromStdWString(data.first)] = QString::fromStdWString(data.second);

    QJsonDocument doc(jsonObj);

    QFile file("AppPath.json");
    file.setTextModeEnabled(true);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file for writing";
        return;
    }

    qint64 bytesWritten = file.write(doc.toJson());
    if (bytesWritten == -1)
    {
        qDebug() << "Failed to write to file";
    }
    else
    {
        qDebug() << "Successfully save path";
    }
    file.close();
}


std::map<std::wstring, std::wstring> LoadPathDataFromJson()
{
    std::map<std::wstring, std::wstring> appinfo;

    QFile file("AppPath.json");
    file.setTextModeEnabled(true);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file for reading";
        return appinfo;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return appinfo;
    }

    QJsonObject jsonObj = doc.object();
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it)
    {
        std::wstring key = it.key().toStdWString();
        std::wstring value = it.value().toString().toStdWString();
        appinfo[key] = value;
    }

    return appinfo;
}

std::map<std::wstring, std::wstring> LoadCustomDataFromJson()
{
    std::map<std::wstring, std::wstring> custominfo;

    QFile file("CustomApp.json");
    file.setTextModeEnabled(true);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file for reading";
        return custominfo;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qDebug() << "JSON parse error:" << parseError.errorString();
        return custominfo;
    }

    QJsonObject jsonObj = doc.object();
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it)
    {
        std::wstring key = it.key().toStdWString();
        std::wstring value = it.value().toString().toStdWString();
        custominfo[key] = value;
    }

    return custominfo;
}


void SaveCustomData_Json(const std::map<std::wstring, std::wstring>& custom_data)
{
    QJsonObject jsonObj;
    for (const auto& data : custom_data)
        jsonObj[QString::fromStdWString(data.first)] = QString::fromStdWString(data.second);

    QJsonDocument doc(jsonObj);

    QFile file("CustomApp.json");
    file.setTextModeEnabled(true);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file for writing";
        return;
    }

    qint64 bytesWritten = file.write(doc.toJson());
    if (bytesWritten == -1)
    {
        qDebug() << "Failed to write to file";
    }
    else
    {
        qDebug() << "Successfully save path";
    }
    file.close();
}



void SaveData_Json(const std::map<std::wstring, unsigned int>& usage)
{

    QJsonObject jsonObj;

    for (const auto& data : usage)
        jsonObj[QString::fromStdWString(data.first)] = static_cast<int>(data.second);


    QJsonDocument doc(jsonObj);

    QFile file("Usage_Time.json");
    file.setTextModeEnabled(true);
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file for writing";
        return;
    }


    qint64 bytesWritten = file.write(doc.toJson());
    if (bytesWritten == -1)
    {
        qDebug() << "Failed to write to file";
    }
    else
    {
        qDebug() << "Successfully wrote" << bytesWritten << "bytes to file";
    }
    file.close();
}




std::map<std::wstring, unsigned int> LoadUsageDataFromJson()
{
    std::map<std::wstring, unsigned int> usage;

    QFile file("Usage_Time.json");
    file.setTextModeEnabled(true);
    if (!file.exists())  // 檔案不存在的情況
    {
        qDebug() << "File not found!";
        return usage;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open file for reading";
        return usage;
    }


    QByteArray data = file.readAll(); //存資料
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data); //存資料
    if (doc.isNull())
    {
        qDebug() << "Invalid JSON format!";
        return usage;
    }

    QJsonObject jsonObj = doc.object(); //取資料
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it)
    {
        std::wstring key = it.key().toStdWString(); //轉換
        unsigned int value = it.value().toInt(); //轉換
        usage[key] = value;
    }

    return usage;
}

//////////////////////////////儲存/存取路徑//////////////////////////////


//////////////////////////////排列//////////////////////////////
std::vector<std::pair<std::wstring, unsigned int>> AppsRank(const std::map<std::wstring, unsigned int>& usage)
{
    std::vector<std::pair<std::wstring, unsigned int>> Sorted_Usage;
    {
        Sorted_Usage = std::vector<std::pair<std::wstring, unsigned int>>(usage.begin(), usage.end());
    }
    std::sort(Sorted_Usage.begin(), Sorted_Usage.end(), [](auto& a, auto& b){ return a.second > b.second; } ); //降序
    return Sorted_Usage;
}



//////////////////////////////字體轉換(非必須)//////////////////////////////
std::string WStringToString(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}


std::wstring StringToWString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}
//////////////////////////////字體轉換(非必須)//////////////////////////////
