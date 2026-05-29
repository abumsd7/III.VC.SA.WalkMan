#include "../includes/ProcessHelper.h"
#include <windows.h>

unsigned int ProcessHelper::GetTime() {
    return GetTickCount();
}

std::vector<std::string> ProcessHelper::FindDirs(const std::string& parentFolder) {
    std::vector<std::string> dirs;
    std::string searchPath = parentFolder + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                strcmp(findData.cFileName, ".") != 0 &&
                strcmp(findData.cFileName, "..") != 0) {
                dirs.push_back(findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return dirs;
}

std::vector<std::string> ProcessHelper::FindFiles(const std::string& folder, const std::string& ext) {
    std::vector<std::string> files;
    std::string searchPath = folder + "\\*" + ext;
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return files;
}

std::string ProcessHelper::UnicodeToAnsi(const wchar_t* wstr) {
    char buf[512] = {0};
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, buf, sizeof(buf), nullptr, nullptr);
    return std::string(buf);
}

void* ProcessHelper::GetFunc(const char* dll, const char* func) {
    HMODULE hMod = GetModuleHandleA(dll);
    if (hMod) {
        return (void*)GetProcAddress(hMod, func);
    }
    return nullptr;
}
