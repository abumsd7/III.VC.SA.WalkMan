#pragma once
#include <plugin.h>
#include <string>
#include <vector>

class ConfigHelper {
    std::string path;
public:
    ConfigHelper(const std::string& iniPath);

    int ReadInt(const char* sec, const char* key, int def);
    float ReadFloat(const char* sec, const char* key, float def);
    CVector2D ReadVec(const char* sec, const char* key, CVector2D def);
    CRect ReadRect(const char* sec, const char* key, CRect def);
    std::string ReadString(const char* sec, const char* key, const std::string& def);

    void WriteInt(const char* sec, const char* key, int val);
    void WriteFloat(const char* sec, const char* key, float val);
    void WriteVec(const char* sec, const char* key, CVector2D val);
    void WriteRect(const char* sec, const char* key, CRect val);
    void WriteString(const char* sec, const char* key, const std::string& val);

    bool Exists();
    void Format(const std::string& headerComments = "");
};
