#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#include <string>
#include <map>
#include <windows.h>
#include <vector>

std::wstring GetActiveProcessName();
std::wstring GetProcessUserName();
std::string WStringToString(const std::wstring&);
std::wstring StringToWString(const std::string&);
void SaveData_Json(const std::map<std::wstring, unsigned int>&);
void SaveData_Path(const std::map<std::wstring, std::wstring>&);
std::wstring Use_Path(const std::wstring&);
std::map<std::wstring, unsigned int> LoadUsageDataFromJson();
std::map<std::wstring, std::wstring> LoadPathDataFromJson();
std::vector<std::pair<std::wstring, unsigned int>> AppsRank(const std::map<std::wstring, unsigned int>&);
std::wstring GetPath();
#endif // PROCESSTRACKER_H
