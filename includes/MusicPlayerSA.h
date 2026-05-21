#pragma once
#ifdef GTASA
#include <plugin.h>
#include <CRGBA.h>
#include "WalkmanState.h"
#include <bass.h>
#include <vector>
#include <string>
#include <fstream>

struct NativeTrack {
    DWORD startOffset;
    DWORD length;
    int soundId;
};

struct NativeStation {
    std::string filename;
    std::vector<NativeTrack> tracks;
    int currentTrackIndex;
    int currentTrackType; // 0=Indent, 1=Advert, 2=Banter, 3=Intro, 4=Track, 5=Outro
    int currentSongIndex; // 0..30
    std::string currentDisplayTitle;
    
    // Active Queue
    std::vector<int> trackQueue;          // SoundIDs
    std::vector<int> trackTypeQueue;      // Track types
    std::vector<std::string> displayTitleQueue;// Display titles
    std::vector<int> pakIdxQueue;         // Package index (stationIdx or 11 for adverts)
    
    // Background simulation & Active track tracking
    int currentPakIdx;               // Package index of actively playing track
    unsigned int lastPositionMs;     // Playback position in ms when switched away
    unsigned int lastSwitchTimeMs;   // GetTickCount() when switched away

    // History buffers
    std::vector<int> advertHistory;
    std::vector<int> musicHistory;
    std::vector<int> banterHistory;
    std::vector<int> identHistory;
};

struct NativeTrackStream {
    std::ifstream file;
    DWORD startOffset;
    DWORD length;
    DWORD currentOffset;
};

class MusicPlayerSA : public WalkmanState {
public:
    static DWORD walkmanStream; // HSTREAM
    static NativeStation nativeStations[12]; // 11 stations + Adverts (index 11)

    static void Initialise();
    static void Update();

    static void LoadPlaylists();
    static void NextTrack();
    static void DeletePlaylists();
    static void StopStreamAndSavePosition();
    static void ChangeMp3Station(int stationIndex);
    static void CycleStation(int dir);
    static void DumpRadioTables();

    // Native radio methods
    static void ScanNativeStations();
    static int GetPerfectSoundId(const std::string& stem, int trackIdx);
    static void ChooseTracksForNativeStation(int stationIdx);
    static const char* GetActiveTrackTitle();

    // BASS file callbacks
    static void CALLBACK NativeTrackCloseProc(void* user);
    static QWORD CALLBACK NativeTrackLenProc(void* user);
    static DWORD CALLBACK NativeTrackReadProc(void* buffer, DWORD length, void* user);
    static BOOL CALLBACK NativeTrackSeekProc(QWORD offset, void* user);
};
#endif