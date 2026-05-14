#ifdef GTASA
#include "../includes/MusicPlayerSA.h"
#include <plugin.h>
#include <CPad.h>
#include <CTimer.h>
#include <CMenuManager.h>
#include <CCamera.h>
#include <patch.h>
#include <filesystem>

namespace fs = std::filesystem;

// Static member definitions
Mp3Station MusicPlayerSA::mp3Stations[10];
int MusicPlayerSA::currentStation = 0;
int MusicPlayerSA::stationCount = 0;
int MusicPlayerSA::skipping = 0;
int MusicPlayerSA::fade = 0;
bool MusicPlayerSA::listActive = false;
int MusicPlayerSA::listCurrentItem = 0;
float MusicPlayerSA::listAlpha = 0.0f;
float MusicPlayerSA::listFade = 0.0f;
DWORD MusicPlayerSA::walkmanStream = 0;
int MusicPlayerSA::walkmanVolume = 64;

#define SA_RADIO_STATION_COUNT 13
#define SA_RADIO_OFF 13

// Global pointers for DrawPlayer.cpp compatibility
static unsigned int curTrackCount = 0;
static unsigned int curTrackIndex = 0;
unsigned int* gameTrackCount = &curTrackCount;
unsigned int* gameCurrentTrack = &curTrackIndex;
unsigned char* gameDisableKeyboard2 = (unsigned char*)0xBA6815;
unsigned char* gameUserPause = (unsigned char*)0xB7CB49;

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
}

void MusicPlayerSA::Update() {
    bool walkmanActive = (currentStation < stationCount);
    
    if (walkmanActive) {
        // Sync info for DrawPlayer
        curTrackCount = mp3Stations[currentStation].trackCount;
        curTrackIndex = mp3Stations[currentStation].currentTrack;

        // Silence native radio (CAERadioTrackManager::Service)
        if (patch::GetUChar(0x4EB9A0) != 0xC3) {
            patch::SetUChar(0x4EB9A0, 0xC3); // ret
        }

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
            if (*gameUserPause) {
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
        if (patch::GetUChar(0x4EB9A0) == 0xC3) {
            patch::SetUChar(0x4EB9A0, 0x55); // push ebp
        }

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

Mp3File* MusicPlayerSA::GetMp3Track(int trackIndex) {
    if (currentStation >= stationCount) return nullptr;
    Mp3File* temp = mp3Stations[currentStation].mp3Start;
    int i = 0;
    while (i < trackIndex && temp) {
        temp = (Mp3File*)temp->nextFile;
        i++;
    }
    return temp;
}

void MusicPlayerSA::LoadPlaylists() {
    for (int i = 0; i < stationCount; i++) {
        InjectPlaylist(i);
    }
}

void MusicPlayerSA::InjectPlaylist(int stationIndex) {
    // Basic implementation for now
}

void MusicPlayerSA::DeletePlaylists() {
    if (walkmanStream) BASS_StreamFree(walkmanStream);
    BASS_Free();
}

void MusicPlayerSA::ReadConfig() {
    // config.Read() equivalent
}

void MusicPlayerSA::HandleKeyPress(int key) {
    // Key handling logic
}

bool MusicPlayerSA::GetChar(int32_t c) {
    return CPad::GetPad(0)->NewKeyState.standardKeys[c];
}

bool MusicPlayerSA::GetCharJustDown(int32_t c) {
    return !!(CPad::GetPad(0)->NewKeyState.standardKeys[c] && !CPad::GetPad(0)->OldKeyState.standardKeys[c]);
}
#endif