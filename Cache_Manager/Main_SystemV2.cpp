#include "Main_System.h"
#include <windows.h>
#include <psapi.h>
#include <fstream>
#include "json.hpp"
#include <codecvt>
#include <io.h>
#include <fcntl.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
using json = nlohmann::json;

std::wstring GetProcessUserName(DWORD processID)
{
    std::wstring username = L"Unknown";
    HANDLE hToken = NULL;
    DWORD size = 0;

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



    HWND hwnd = GetForegroundWindow();

    if (!hwnd)
    {
        return L"";
    }

    DWORD pid;

    GetWindowThreadProcessId(hwnd, &pid);


    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));

    HANDLE hProcess = NULL;
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

    if (!hProcess)
    {
        return L"";
    }


    std::wstring user = GetProcessUserName(pid);
    if (!user.empty())
    {
        //std::wcout << user << std::endl;
        return title;
    }

    CloseHandle(hProcess);
    return L"";
}


void SaveData_Json(const std::map<std::wstring, unsigned int>& usage)
{
    if (usage.empty())
    {
        qDebug() << "usage is empty!";
    }

    // 創建 QJsonObject
    QJsonObject jsonObj;
    for (const auto& data : usage)
    {
        jsonObj[QString::fromStdWString(data.first)] = static_cast<int>(data.second);
    }

    // 創建 QJsonDocument
    QJsonDocument doc(jsonObj);

    // 使用 QFile 儲存 JSON 文件
    QFile file("Usage_Time.json");
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file for writing";
        return;
    }

    // 寫入 JSON 格式資料
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

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qDebug() << "Invalid JSON format!";
        return usage;
    }

    QJsonObject jsonObj = doc.object();
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it)
    {
        std::wstring key = it.key().toStdWString();
        unsigned int value = it.value().toInt();
        usage[key] = value;
    }

    return usage;
}

// wstring 轉 string 的輔助函數
std::string WStringToString(const std::wstring& wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// string 轉 wstring 的輔助函數
std::wstring StringToWString(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

