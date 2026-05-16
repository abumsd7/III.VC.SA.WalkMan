#pragma once
#ifdef GTASA
#include <plugin.h>
#include <CRGBA.h>
#include "WalkmanState.h"
#include <bass.h>

class MusicPlayerSA : public WalkmanState {
public:
    static DWORD walkmanStream; // HSTREAM

    static void Initialise();
    static void Update();

    static void LoadPlaylists();
    static void NextTrack();
    static void DeletePlaylists();
    static void StopStreamAndSavePosition();

    static void ReadConfig();
    static void ChangeMp3Station(int stationIndex);
    static void CycleStation(int dir);
};
#endif