#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#include <string>
#include <map>
#include <windows.h>
std::wstring GetActiveProcessName();
std::wstring GetProcessUserName(DWORD);
std::string WStringToString(const std::wstring&);
std::wstring StringToWString(const std::string&);
void SaveData_Json(const std::map<std::wstring, unsigned int>&);
std::map<std::wstring, unsigned int> LoadUsageDataFromJson();

#endif // PROCESSTRACKER_H
