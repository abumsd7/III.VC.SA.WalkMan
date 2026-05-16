#pragma once

#include <plugin.h>
#include <CRGBA.h>
#include "WalkmanState.h"

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

class MusicPlayer : public WalkmanState {
public:
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
    static unsigned int adfLastPosition[12];
    static unsigned int adfSwitchTime[12];

    static void Initialise();
    static void Update();

    static void LoadPlaylists();
    static void NextTrack();
    static void DeletePlaylists();

    static void ReadConfig();
    static void ChangeMp3Station(int stationIndex);
    static void CycleStation(int dir);
};
#endif