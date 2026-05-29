#include "../includes/ConfigHelper.h"
#include <windows.h>
#include <stdio.h>

ConfigHelper::ConfigHelper(const std::string& iniPath) : path(iniPath) {}

bool ConfigHelper::Exists() {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

int ConfigHelper::ReadInt(const char* sec, const char* key, int def) {
    return GetPrivateProfileIntA(sec, key, def, path.c_str());
}

float ConfigHelper::ReadFloat(const char* sec, const char* key, float def) {
    char buf[64];
    if (GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), path.c_str()) > 0) {
        return (float)atof(buf);
    }
    return def;
}

CVector2D ConfigHelper::ReadVec(const char* sec, const char* key, CVector2D def) {
    char buf[128];
    if (GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), path.c_str()) > 0) {
        float x = 0.0f, y = 0.0f;
        if (sscanf_s(buf, "%f %f", &x, &y) == 2) {
            return CVector2D(x, y);
        }
    }
    return def;
}

CRect ConfigHelper::ReadRect(const char* sec, const char* key, CRect def) {
    char buf[256];
    if (GetPrivateProfileStringA(sec, key, "", buf, sizeof(buf), path.c_str()) > 0) {
        float left = 0.0f, top = 0.0f, right = 0.0f, bottom = 0.0f;
        if (sscanf_s(buf, "%f %f %f %f", &left, &top, &right, &bottom) == 4) {
            return CRect(left, top, right, bottom);
        }
    }
    return def;
}

std::string ConfigHelper::ReadString(const char* sec, const char* key, const std::string& def) {
    char buf[512];
    if (GetPrivateProfileStringA(sec, key, def.c_str(), buf, sizeof(buf), path.c_str()) > 0) {
        return std::string(buf);
    }
    return def;
}

void ConfigHelper::WriteInt(const char* sec, const char* key, int val) {
    char buf[32];
    sprintf_s(buf, "%d", val);
    WritePrivateProfileStringA(sec, key, buf, path.c_str());
}

void ConfigHelper::WriteFloat(const char* sec, const char* key, float val) {
    char buf[32];
    sprintf_s(buf, "%.2f", val);
    WritePrivateProfileStringA(sec, key, buf, path.c_str());
}

void ConfigHelper::WriteVec(const char* sec, const char* key, CVector2D val) {
    char buf[64];
    sprintf_s(buf, "%.2f %.2f", val.x, val.y);
    WritePrivateProfileStringA(sec, key, buf, path.c_str());
}

void ConfigHelper::WriteRect(const char* sec, const char* key, CRect val) {
    char buf[128];
    sprintf_s(buf, "%.2f %.2f %.2f %.2f", val.left, val.top, val.right, val.bottom);
    WritePrivateProfileStringA(sec, key, buf, path.c_str());
}

void ConfigHelper::WriteString(const char* sec, const char* key, const std::string& val) {
    WritePrivateProfileStringA(sec, key, val.c_str(), path.c_str());
}

void ConfigHelper::Format(const std::string& headerComments) {
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "r");
    if (!f) return;
    
    std::string content;
    if (!headerComments.empty()) {
        content += headerComments;
        if (headerComments.back() != '\n') {
            content += "\n";
        }
    }
    
    char line[512];
    bool firstSection = true;
    while (fgets(line, sizeof(line), f)) {
        std::string l = line;
        if (l[0] == ';' || l[0] == '#') {
            continue;
        }
        if (l[0] == '[') {
            if (!firstSection) {
                content += "\n";
            }
            firstSection = false;
        }
        content += l;
    }
    fclose(f);
    
    fopen_s(&f, path.c_str(), "w");
    if (f) {
        fputs(content.c_str(), f);
        fclose(f);
    }
}
