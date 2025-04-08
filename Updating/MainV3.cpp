#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <map>
#include <string>
#include <codecvt>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <unordered_set>
#include <algorithm>
#include <ctime>
#include <fstream>
#include "json.hpp"
#pragma comment(lib, "Psapi.lib")

using json = nlohmann::json;
std::wstring GetActiveProcessName();
std::wstring GetProcessUserName(DWORD);
std::string WStringToString(const std::wstring&);
std::wstring StringToWString(const std::string&);
void SaveData_Json(const std::map<std::wstring, unsigned int>&);
std::map<std::wstring, unsigned int> LoadUsageDataFromJson();
int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::map<std::wstring, unsigned int> usageTime = LoadUsageDataFromJson();
    std::wstring currentApp = L"";
    int interval = 1; // 秒數
    auto Start_Time = std::chrono::steady_clock::now();

    while (1) 
    {
        std::wstring active = GetActiveProcessName();
        
        if (!active.empty()) {
            if (active == currentApp)
            {
                usageTime[active] += interval;
            }
            else
            {
                currentApp = active;
                usageTime[active] += interval;
            }
        }

        std::vector<std::pair<std::wstring, unsigned int>> Sorted_Usage(usageTime.begin(), usageTime.end());
        std::sort(Sorted_Usage.begin(), Sorted_Usage.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; } //auto = std::pair<std::wstring, int>&
        );
        auto now = std::chrono::steady_clock::now();
        auto Totally_Time = std::chrono::duration_cast<std::chrono::seconds>(now - Start_Time).count();
        
        // 顯示目前狀態
        system("cls"); //keep updating
        std::wcout << L"使用時間統計：" << Totally_Time << L" 秒" << std::endl;
        unsigned int ctr = 0;
        std::wstring rank[] = { L"第一名: ", L"第二名: ", L"第三名: ", L"第四名: ", L"第五名: " };
        for (auto& used : Sorted_Usage)
        {
            if(ctr < 5 && used.second >= 10)
                std::wcout << L" - " << rank[ctr++] << used.first << L": " << used.second << L" 秒" << std::endl;
        }
        SaveData_Json(usageTime);
        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    return 0;
}


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
    _setmode(_fileno(stdout), _O_U16TEXT);
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
    json j;
    for (const auto& data : usage)
        j[WStringToString(data.first)] = data.second;

    std::ofstream output("Usage_Time.json");
    output << std::setw(4) << j << std::endl;
}

std::map<std::wstring, unsigned int> LoadUsageDataFromJson()
{
    std::ifstream input("Usage_Time.json");
    std::map<std::wstring, unsigned int> usage;
    if(input)
    {
        json j;
        input >> j;
        for (auto& data : j.items())
            usage[StringToWString(data.key())] = data.value();
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
