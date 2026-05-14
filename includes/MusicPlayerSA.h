#pragma once
#ifdef GTASA
#include <plugin.h>
#include <CRGBA.h>
#include "WalkManConfig.h"
#include <bass.h>

class MusicPlayerSA {
public:
    static Mp3Sstation mp3Stations[10];
    static int currentStation;
    static int stationCount;
    static int skipping;
    static int fade;

    static bool listActive;
    static int listCurrentItem;
    static float listAlpha;
    static float listFade;

    static DWORD walkmanStream; // HSTREAM
    static int walkmanVolume;

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
    
    // Input helpers
    static bool GetChar(int32_t c);
    static bool GetCharJustDown(int32_t c);
};
#endif