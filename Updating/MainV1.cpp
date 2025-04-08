#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include <io.h>
#include <fcntl.h>
#include <algorithm>
#include <sddl.h>  // for ConvertSidToStringSid
#include <psapi.h> // for GetModuleFileNameEx
#include <set>
#include <unordered_set>
#pragma comment(lib, "advapi32.lib")

// 儲存擁有 GUI 的 Process ID
std::unordered_set<DWORD> guiProcessIds;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) 
{

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);

    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));

    // 只取頂層可見視窗
    if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL && wcslen(title) > 0) {
        guiProcessIds.insert(pid);
    }

    return TRUE;
}

std::wstring GetProcessUserName(DWORD processID) 
{
    std::wstring username = L"Unknown";

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
    if (hProcess) {
        HANDLE hToken = NULL;
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) 
        {
            DWORD size = 0;
            GetTokenInformation(hToken, TokenUser, NULL, 0, &size);

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
            {
                BYTE* buffer = new BYTE[size];
                if (GetTokenInformation(hToken, TokenUser, buffer, size, &size)) {
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
                }
                delete[] buffer;
            }
            CloseHandle(hToken);
        }
        CloseHandle(hProcess);
    }
    return username;
}

// 檢查是否為常見系統背景行程
bool IsSystemProcess(const std::wstring& name) {
    static const std::vector<std::wstring> systemProcesses = 
    {
        L"System",
        L"Idle",
        L"svchost.exe",
        L"winlogon.exe",
        L"csrss.exe",
        L"smss.exe",
        L"services.exe",
        L"lsass.exe",
        L"Registry",
        L"Memory Compression"
    };

    for (const auto& sys : systemProcesses) 
    {
        if (_wcsicmp(name.c_str(), sys.c_str()) == 0) 
        {
            return true;
        }
    }
    return false;
}

// 取得目前正在執行的 process 名稱（排除系統）
std::vector<std::wstring> GetRunningProcesses()
{
    EnumWindows(EnumWindowsProc, 0);  // 掃描目前有視窗的 Process ID

    std::vector<std::wstring> processes;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) return processes;

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    const std::unordered_set<std::wstring> blacklist = {
    L"ApplicationFrameHost.exe",
    L"CalculatorApp.exe",
    L"Microsoft.Media.Player.exe",
    L"NVIDIA Share.exe"
    };

    if (Process32First(hSnapshot, &pe)) 
    {
        do 
        {
            std::wstring user = GetProcessUserName(pe.th32ProcessID);

            if (blacklist.find(pe.szExeFile) != blacklist.end()) continue;
            
            // 如果這個 Process 沒有 GUI 視窗，就跳過
            if (guiProcessIds.find(pe.th32ProcessID) == guiProcessIds.end()) continue;

            // 排除系統帳號
            if (user.find(L"SYSTEM") != std::wstring::npos ||
                user.find(L"LOCAL SERVICE") != std::wstring::npos ||
                user.find(L"NETWORK SERVICE") != std::wstring::npos) {
                continue;
            }
            if (!IsSystemProcess(pe.szExeFile)) 
            {
                processes.push_back(pe.szExeFile); // 儲存非系統 process 名稱
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return processes;
}

int main() 
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    unsigned int ctr = 0;
    auto processes = GetRunningProcesses();
    std::sort(processes.begin(), processes.end());
    auto deleted = std::unique(processes.begin(), processes.end());
    processes.erase(deleted, processes.end());
    std::wcout << L"使用者可能啟動的應用程式：" << std::endl;
    for (const auto& name : processes) 
    {
        std::wcout << L" - " << name << std::endl;
        ++ctr;
    }
    std::wcout << L" 總共: " << ctr << std::endl;
    return 0;
}

 
