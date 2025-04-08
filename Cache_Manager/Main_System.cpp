#include "Main_System.h"
#include <windows.h>
#include <psapi.h>
#include <fstream>
#include "json.hpp"
#include <codecvt>
#include <io.h>
#include <fcntl.h>

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
    if (input)
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

