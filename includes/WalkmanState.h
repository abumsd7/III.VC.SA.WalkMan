#pragma once

#include <plugin.h>
#include <CRGBA.h>
#include <CPad.h>
#include "WalkManConfig.h"
#include "InputHandler.h"

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

class WalkmanState {
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
    static int walkmanVolume;

    // Helper methods for DrawPlayer and shared state queries
    static Mp3File* GetMp3Track(int trackIndex);
    static bool IsRadioOff();
    static const char* GetActiveStationName();
    static void GetTrackPlaybackInfo(unsigned int& current_ms, unsigned int& total_ms);
    static unsigned int GetCurrentTrackIndex();
    static unsigned int GetTrackCount();
    static void SetListActive(bool active);

    // Centralized Input & Playback Bridge
    static void ProcessInput();
    static void HandleKeyPress(int key);

    static void StopStream();
    static void SetCurrentTrackIndex(int idx);
    static void CycleStation(int dir);
    static void ChangeMp3Station(int idx);
    static void NextTrack();
    static void SetStreamVolume(int vol);
};

