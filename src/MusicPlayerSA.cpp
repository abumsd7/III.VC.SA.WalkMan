#ifdef GTASA
#include "../includes/MusicPlayerSA.h"
#include "../includes/MusicPlayer.h"
#include "../includes/MP3Injection.h"
#include <plugin.h>
#include <CPad.h>
#include <CTimer.h>
#include <CMenuManager.h>
#include <CCamera.h>
#include <patch.h>
#include <filesystem>

using namespace plugin;
namespace fs = std::filesystem;

// Static member definitions
DWORD MusicPlayerSA::walkmanStream = 0;

#define SA_RADIO_STATION_COUNT 13
#define SA_RADIO_OFF 13

// Global pointers for WalkmanState compatibility

unsigned char* gameDisableKeyboard2 = (unsigned char*)0xBA6815;

const char* gameRadioNames[13] = {
    "Playback FM", "K-Rose", "K-DST", "Bounce FM", "SF-UR", 
    "Radio Los Santos", "Radio X", "CSR 103.9", "K-JAH West", 
    "Master Sounds 98.3", "WCTR", "User Tracks", "Radio Off"
};

void MusicPlayerSA::Initialise() {
    // Initialise BASS
    if (!BASS_Init(-1, 44100, 0, (HWND)RsGlobal.ps->window, NULL)) {
        // Log error
    }

    ReadConfig();
    currentStation = stationCount + SA_RADIO_STATION_COUNT;
    InputHandler::ResetKeyState();
}

void MusicPlayerSA::Update() {
    WalkmanState::ProcessInput();

    bool walkmanActive = (currentStation < stationCount);
    
    if (walkmanActive) {

        // Silence native radio (CAERadioTrackManager::Service)
        //if (patch::GetUChar(0x4EB9A0) != 0xC3) {
        //    patch::SetUChar(0x4EB9A0, 0xC3); // ret
        //}

        if (!walkmanStream) {
            Mp3File* file = GetMp3Track(mp3Stations[currentStation].currentTrack);
            if (file) {
                walkmanStream = BASS_StreamCreateFile(FALSE, file->filename, 0, 0, 0);
                if (walkmanStream) {
                    BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);
                    BASS_ChannelPlay(walkmanStream, FALSE);
                }
            }
        } else {
            // Handle pausing
            if (CTimer::m_UserPause) {
                BASS_ChannelPause(walkmanStream);
            } else {
                BASS_ChannelPlay(walkmanStream, FALSE);
            }

            BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);

            if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_STOPPED) {
                skipping = 1;
                NextTrack();
            }
        }
    } else {
        // Restore native radio
        //if (patch::GetUChar(0x4EB9A0) == 0xC3) {
        //    patch::SetUChar(0x4EB9A0, 0x55); // push ebp
       // }

        if (walkmanStream) {
            BASS_StreamFree(walkmanStream);
            walkmanStream = 0;
        }
    }
}

void MusicPlayerSA::NextTrack() {
    if (walkmanStream) {
        BASS_StreamFree(walkmanStream);
        walkmanStream = 0;
    }

    if (mp3Stations[currentStation].shuffle && mp3Stations[currentStation].trackCount > 0) {
        mp3Stations[currentStation].currentTrack = rand() % mp3Stations[currentStation].trackCount;
    } else {
        mp3Stations[currentStation].currentTrack += skipping;
        if (mp3Stations[currentStation].currentTrack < 0) 
            mp3Stations[currentStation].currentTrack = mp3Stations[currentStation].trackCount - 1;
        else if (mp3Stations[currentStation].currentTrack >= mp3Stations[currentStation].trackCount)
            mp3Stations[currentStation].currentTrack = 0;
    }
}

void MusicPlayerSA::StopStreamAndSavePosition() {
    if (walkmanStream) {
        QWORD pos = BASS_ChannelGetPosition(walkmanStream, BASS_POS_BYTE);
        double posSec = BASS_ChannelBytes2Seconds(walkmanStream, pos);
        if (currentStation < stationCount) {
            mp3Stations[currentStation].lastPositionMs = (unsigned int)(posSec * 1000.0);
        }
        BASS_StreamFree(walkmanStream);
        walkmanStream = 0;
    }
}

void MusicPlayerSA::ChangeMp3Station(int stationIndex) {
    if (stationIndex < stationCount) {
        StopStreamAndSavePosition();

        currentStation = stationIndex;
        listCurrentItem = mp3Stations[stationIndex].currentTrack;

        if (mp3Stations[stationIndex].fade > 0) {
            fade = mp3Stations[stationIndex].fade;
        }
    }
}

void MusicPlayerSA::CycleStation(int dir) {
    int totalStations = stationCount + SA_RADIO_STATION_COUNT + 1; // +1 for OFF
    int nextStation = currentStation + dir;

    if (nextStation < 0) nextStation = totalStations - 1;
    if (nextStation >= totalStations) nextStation = 0;

    if (nextStation < stationCount) {
        ChangeMp3Station(nextStation);
    } else {
        StopStreamAndSavePosition();

        currentStation = nextStation;
        fade = 200;
    }
}

void MusicPlayerSA::LoadPlaylists() {
    for (int i = 0; i < stationCount; i++) {
        MP3Injection::InjectPlaylist(i);
    }
}

void MusicPlayerSA::DeletePlaylists() {
    if (walkmanStream) BASS_StreamFree(walkmanStream);
    BASS_Free();
}

void MusicPlayerSA::ReadConfig() {
    // config.Read() equivalent
}
#endif