#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <io.h>
#include <fcntl.h>
#include <unordered_set>
#pragma comment(lib, "Psapi.lib")

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

    HANDLE hToken = NULL;
        
    DWORD size = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &size);

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
    //std::wcout << pid << std::endl;

    wchar_t title[256];
    GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));

    HANDLE hProcess = NULL;
    //hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    //hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    //std::wcout << hProcess << std::endl;
    if (!hProcess) 
    {
       //std::wcout << L"測試：" << std::endl;
       return L"";
    }
    //std::wcout << hProcess << std::endl;
    WCHAR name[MAX_PATH];

    HANDLE hToken = NULL;
  
    
    
    //if (GetModuleBaseName(hProcess, NULL, name, MAX_PATH)) 
    //{
    //   // std::wcout << hProcess << std::endl;
    //    CloseHandle(hProcess);
    //    return name;
    //}
    std::wstring user = GetProcessUserName(pid);
    if (!user.empty())
    {
        //std::wcout << user << std::endl;
        return title;
    }

    CloseHandle(hProcess);
    return L"";
}

int main() 
{
    _setmode(_fileno(stdout), _O_U16TEXT);
    std::map<std::wstring, int> usageTime;
    std::wstring currentApp = L"";
    int interval = 1; // 秒數

    while (1) {
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

        // 顯示目前狀態
        system("cls"); //keep updating
        std::wcout << L"使用時間統計：" << std::endl;
        for (auto& used : usageTime) 
        {
            std::wcout << L" - " << used.first << L": " << used.second << L" 秒" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    return 0;
}
