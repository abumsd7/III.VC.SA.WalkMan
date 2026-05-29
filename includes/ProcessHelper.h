#pragma once
#include <string>
#include <vector>

namespace ProcessHelper {
    unsigned int GetTime();
    std::vector<std::string> FindDirs(const std::string& parentFolder);
    std::vector<std::string> FindFiles(const std::string& folder, const std::string& ext);
    std::string UnicodeToAnsi(const wchar_t* wstr);
    void* GetFunc(const char* dll, const char* func);
}
