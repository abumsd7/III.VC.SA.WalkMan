#pragma once

#include <plugin.h>
#include <CRGBA.h>
#include "WalkManConfig.h"

typedef struct _ID3v1
{
    char tag[3];
    char title[30];
    char artist[30];
    char album[30];
    unsigned int year;
    char comment[30];
    unsigned char genre;

} ID3v1;

struct Mp3File
{
    char filename[260];
    unsigned int trackLength;
    unsigned int prevTrackLength;
    unsigned int nextFile;
    unsigned int unknown1;
    unsigned int unknown2;

    char* title;
    char* artist;
    char* album;
    unsigned int prevFile;
};

struct Mp3Station
{
    char* name;
    char* playlist;
    bool showTitle;
    bool shuffle;
    int fade;

    Mp3File* mp3Start;
    int currentTrack;
    int trackCount;
    unsigned int lastPositionMs;
};

struct KeyConfig
{
    unsigned char next;
    unsigned char prev;
    unsigned char toggleShuffle;
    unsigned char volumeUp;
    unsigned char volumeDown;
    unsigned char toggleList;
    unsigned char listScrollUp;
    unsigned char listScrollDown;
    unsigned char listChoose;
};

#ifndef GTASA
// Miles Sound System function types
typedef void(__stdcall* p_AIL_close_stream)(unsigned int);
typedef void(__stdcall* p_AIL_stream_ms_position)(unsigned int, unsigned int*, unsigned int*);
typedef void(__stdcall* p_AIL_set_stream_ms_position)(unsigned int, unsigned int);
typedef unsigned int(__stdcall* p_AIL_open_stream)(unsigned int, const char*, unsigned int);
typedef void(__stdcall* p_AIL_pause_stream)(unsigned int, unsigned int);
typedef void(__stdcall* p_AIL_start_stream)(unsigned int);
typedef void(__stdcall* p_AIL_set_stream_volume)(unsigned int, int);
typedef int(__stdcall* p_AIL_stream_status)(unsigned int);
typedef void(__stdcall* p_AIL_service_stream)(unsigned int, int);

class MusicPlayer {
public:
    static Mp3Station mp3Stations[10];
    static int currentStation;
    static int stationCount;
    static int skipping;
    static int fade;

    static bool listActive;
    static int listCurrentItem;
    static float listAlpha;
    static float listFade;

    // Miles Sound System pointers
    static p_AIL_close_stream closeStream;
    static p_AIL_stream_ms_position streamMsPosition;
    static p_AIL_set_stream_ms_position setStreamMsPosition;
    static p_AIL_open_stream openStream;
    static p_AIL_pause_stream pauseStream;
    static p_AIL_start_stream startStream;
    static p_AIL_set_stream_volume setStreamVolume;
    static p_AIL_stream_status streamStatus;
    static p_AIL_service_stream serviceStream;

    static unsigned int walkmanStream;
    static int walkmanVolume;
    static unsigned int adfLastPosition[12];
    static unsigned int adfSwitchTime[12];

    static void Initialise();
    static void Update();
    static void HandleKeyPress(int key);

    static void LoadPlaylists();
    static void NextTrack();
    static void DeletePlaylists();

    static void ReadConfig();
    static void ChangeMp3Station(int stationIndex);
    static void InjectPlaylist(int stationIndex);
    static void UpdateMp3InfoWithID3v1Tags(Mp3File* file);
    static void UpdateMp3InfoWithID3v2Tags(Mp3File* file);
    static Mp3File* GetMp3Track(int trackIndex);
    static void CycleStation(int dir);
    static bool GetMouseWheelUpJustDown() { return !!(CPad::GetPad(0)->NewMouseControllerState.wheelUp && !CPad::GetPad(0)->OldMouseControllerState.wheelUp); }
    static bool GetMouseWheelDownJustDown() { return !!(CPad::GetPad(0)->NewMouseControllerState.wheelDown && !CPad::GetPad(0)->OldMouseControllerState.wheelDown); }
    static bool GetMouseWheelUpJustUp() { return !!(!CPad::GetPad(0)->NewMouseControllerState.wheelUp && CPad::GetPad(0)->OldMouseControllerState.wheelUp); }
    static bool GetMouseWheelDownJustUp() { return !!(!CPad::GetPad(0)->NewMouseControllerState.wheelDown && CPad::GetPad(0)->OldMouseControllerState.wheelDown); }
    static bool GetMouseWheelUp() { return CPad::GetPad(0)->NewMouseControllerState.wheelUp; }
    static bool GetMouseWheelDown() { return CPad::GetPad(0)->NewMouseControllerState.wheelDown; }
    static bool GetMouseWheelUpUp() { return !CPad::GetPad(0)->OldMouseControllerState.wheelUp; }
    static bool GetMouseWheelDownUp() { return !CPad::GetPad(0)->OldMouseControllerState.wheelDown; }
    static bool GetChar(int32_t c) { return CPad::GetPad(0)->NewKeyState.standardKeys[c]; }
    static bool GetCharJustDown(int32_t c) { return !!(CPad::GetPad(0)->NewKeyState.standardKeys[c] && !CPad::GetPad(0)->OldKeyState.standardKeys[c]); }
};
#endif