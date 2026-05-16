#ifndef GTASA
#include "../includes/MusicPlayer.h"
#include "../includes/MP3Injection.h"
#include "../includes/SomeMacros.h"
#include <CFont.h>
#include <CHud.h>
#include <CPad.h>
#include <stdio.h>
#include <patch.h>
#include <filesystem>
#include <extensions/Paths.h>
#include <CMenuManager.h>
#include <cDMAudio.h>

#ifdef GTA3
#include <cMusicManager.h>
#endif

using namespace plugin;

p_AIL_close_stream MusicPlayer::closeStream = nullptr;
p_AIL_stream_ms_position MusicPlayer::streamMsPosition = nullptr;
p_AIL_set_stream_ms_position MusicPlayer::setStreamMsPosition = nullptr;
p_AIL_open_stream MusicPlayer::openStream = nullptr;
p_AIL_pause_stream MusicPlayer::pauseStream = nullptr;

// Game variable pointers
#ifdef GTAVC
unsigned int* gameCurrentRadiostation = (unsigned int*)0x9839BC;   // CMusicManager::m_nRadioInCar (Base 0x980038 + Offset 0x3984)
unsigned int* gameMp3Files = (unsigned int*)0x9753E0;   // _pMP3List (reVC)
unsigned int* gameTrackCount = (unsigned int*)0xA108B0;   // _nNumOfMp3Files (reVC)
unsigned int* gameCurrentTrack = (unsigned int*)0x97881C;   // _curMP3Index (reVC)
unsigned int* gameCurrentStream = (unsigned int*)0x978668;   // int gAudioStreams[] in asm, mp3stream (reVC)
unsigned int* gameHDigDriver = (unsigned int*)0x978550;   // int gHDigDriver in asm, _DIG_driver (Miles)
unsigned char* gameDisableKeyboard2 = (unsigned char*)0xA10AE4;  // CPad::m_bMapPadOneToPadTwo
unsigned char* gameUserPause = (unsigned char*)0xA10B36;  // CTimer::m_UserPause
#else
unsigned char* gameCurrentRadiostation = (unsigned char*)0x8F42BC;  // cMusicManager::m_nRadioInCar (Base 0x8F3964 + Offset 0x958)
unsigned int* gameMp3Files = (unsigned int*)0x8E2C7C;   // _pMP3List (1.0 symbols)
unsigned int* gameTrackCount = (unsigned int*)0x95CC00;   // _nNumOfMp3Files (1.0 symbols)
unsigned int* gameCurrentTrack = (unsigned int*)0x8F2558;   // _curMP3Index in re3
unsigned int* gameCurrentStream = (unsigned int*)0x709C50;   // mp3stream in re3
unsigned int* gameHDigDriver = (unsigned int*)0x8F1A24;   // hDigDriver in asm
unsigned char* gameDisableKeyboard2 = (unsigned char*)0x95CD48;  // CPad::m_bMapPadOneToPadTwo
unsigned char* gameUserPause = (unsigned char*)0x95CD7C;  // CTimer::m_UserPause
#endif
p_AIL_start_stream MusicPlayer::startStream = nullptr;
p_AIL_set_stream_volume MusicPlayer::setStreamVolume = nullptr;
p_AIL_stream_status MusicPlayer::streamStatus = nullptr;
p_AIL_service_stream MusicPlayer::serviceStream = nullptr;
unsigned int MusicPlayer::adfLastPosition[12] = { 0 };
unsigned int MusicPlayer::walkmanStream = 0;
unsigned int MusicPlayer::adfSwitchTime[12] = { 0 };



void MusicPlayer::Initialise() {

    // Initialize MSS pointers
#ifdef GTAVC
    closeStream = patch::Get<p_AIL_close_stream>(0x6F25C0);
    streamMsPosition = patch::Get<p_AIL_stream_ms_position>(0x6F25B8);
    setStreamMsPosition = patch::Get<p_AIL_set_stream_ms_position>(0x6F25CC);
    openStream = patch::Get<p_AIL_open_stream>(0x6F25C4);
    pauseStream = patch::Get<p_AIL_pause_stream>(0x6F25BC);
    startStream = patch::Get<p_AIL_start_stream>(0x6F25D0);
    setStreamVolume = patch::Get<p_AIL_set_stream_volume>(0x6F25B0);
    streamStatus = patch::Get<p_AIL_stream_status>(0x6F25AC);
    serviceStream = patch::Get<p_AIL_service_stream>(0x6F25D4);
#else
    closeStream = patch::Get<p_AIL_close_stream>(0x61D5D0);
    streamMsPosition = patch::Get<p_AIL_stream_ms_position>(0x61D5CC);
    setStreamMsPosition = patch::Get<p_AIL_set_stream_ms_position>(0x61D668);
    openStream = patch::Get<p_AIL_open_stream>(0x61D5C8);
    pauseStream = patch::Get<p_AIL_pause_stream>(0x61D5FC);
    startStream = patch::Get<p_AIL_start_stream>(0x61D664);
    setStreamVolume = patch::Get<p_AIL_set_stream_volume>(0x61D66C);
    streamStatus = patch::Get<p_AIL_stream_status>(0x61D674);
    serviceStream = patch::Get<p_AIL_service_stream>(0x61D660);
#endif
    WalkmanState::ReadConfig();
    currentStation = stationCount + GAME_STATION_COUNT; // Initialize to OFF

    if (stationCount > 0) {
#ifdef GTAVC
        // Hook into mp3 processing
        patch::ReplaceFunctionCall(0x5D7EA4, LoadPlaylists); //patching call load_mp3_files
        patch::ReplaceFunctionCall(0x5D7340, DeletePlaylists); //patching call delete_mp3_files

        // Force MP3 Radio Channel to be available (0x5D80E0 is IsMP3RadioChannelAvailable in this version)
        patch::SetChar(0x5D80E0, 0xB8); // mov eax, 1
        patch::SetInt(0x5D80E1, 1);
        patch::SetChar(0x5D80E5, 0xC3); // ret
#else
        // Hook into mp3 processing
        patch::ReplaceFunctionCall(0x566C7D, LoadPlaylists);
        patch::ReplaceFunctionCall(0x566E2D, DeletePlaylists);

        // bRadioOff = true;
        // Force MP3 Radio Channel to be available
        patch::SetChar(0x57A9C0, 0xB8); // mov eax, 1
        patch::SetInt(0x57A9C1, 1);
        patch::SetChar(0x57A9C5, 0xC3); // ret
#endif
    }

    LoadPlaylists();

    // Seed ADF broadcast clocks so stations "play" from game start
    unsigned int startTime = GetTickCount();
    for (int i = 0; i < GAME_STATION_COUNT; i++) adfSwitchTime[i] = startTime;
    InputHandler::ResetKeyState();
}

void MusicPlayer::Update() {
    WalkmanState::ProcessInput();

    // Pause handling
    if (*gameUserPause == 1) {
        if (walkmanStream) pauseStream(walkmanStream, 1);
        return; // Don't process playback while paused
    }
    else {
        if (walkmanStream) pauseStream(walkmanStream, 0);
    }

#ifdef GTA3
    // Toggle cMusicManager::ServiceGameMode (0x57D690) based on WalkMan state.
    // When WalkMan is active: patch to 'ret' so native radio can't play.
    // When WalkMan is OFF: restore original byte so native radio works.
    {
        static unsigned char sgmOrigByte = 0;
        if (sgmOrigByte == 0)
            sgmOrigByte = *(unsigned char*)0x57D690;

        bool walkmanActive = (currentStation != stationCount + GAME_STATION_COUNT);
        if (walkmanActive && *(unsigned char*)0x57D690 != 0xC3)
            patch::SetChar(0x57D690, 0xC3);
        else if (!walkmanActive && *(unsigned char*)0x57D690 == 0xC3)
            patch::SetChar(0x57D690, sgmOrigByte);
    }
#endif

    // Independent MSS Playback Logic
    if (currentStation == stationCount + GAME_STATION_COUNT) {
        // Radio is explicitly OFF
        if (walkmanStream) {
            closeStream(walkmanStream);
            walkmanStream = 0;
        }
    }
    else {
        // Radio is ON

#ifdef GTAVC
        CVehicle* playerVeh = FindPlayerVehicle();
        if (playerVeh)
        {
            if (playerVeh->m_nRadioStation != RADIO_OFF_VAL) {
                playerVeh->m_nRadioStation = RADIO_OFF_VAL;
            }
        }
#endif
        if (!walkmanStream) {
            const char* pathToPlay = nullptr;

            if (currentStation < stationCount) {
                // Custom MP3
                Mp3File* file = GetMp3Track(*gameCurrentTrack);
                if (file) pathToPlay = file->filename;
            }
            else {
                // Native Radio Station
                int gameRadioID = currentStation - stationCount;
#ifdef GTAVC
                static const char* nativePaths[] = {
                    "WILD.ADF", "FLASH.ADF", "KCHAT.ADF",
                    "FEVER.ADF", "VROCK.ADF", "VCPR.ADF",
                    "ESPANT.ADF", "EMOTION.ADF", "WAVE.ADF"
                };
#else
                static const char* nativePaths[] = {
                    "CHAT.wav", "RISE.wav", "MSX.wav",
                    "HEAD.wav", "GAME.wav", "CLASS.wav",
                    "LIPS.wav", "KJAH.wav", "FLASH.wav"
                };
#endif
                if (gameRadioID >= 0 && gameRadioID < 9) {
                    static char safeAdfPath[260];
                    // We MUST use a writable buffer because sampman_miles.cpp 
                    // uses strcpy to replace ".ADF" with ".mp3" internally!
                    sprintf(safeAdfPath, "AUDIO\\%s", nativePaths[gameRadioID]);
                    pathToPlay = safeAdfPath;
                }
            }

            if (pathToPlay && *gameHDigDriver != 0) {
                walkmanStream = openStream(*gameHDigDriver, pathToPlay, 0);
                if (walkmanStream) {
                    setStreamVolume(walkmanStream, walkmanVolume);

                    // Restore saved position
                    unsigned int savedPos = 0;
                    if (currentStation < stationCount) {
                        savedPos = mp3Stations[currentStation].lastPositionMs;
                    }
                    else {
                        // ADF: simulate live broadcast
                        int adfIdx = currentStation - stationCount;
                        if (adfIdx >= 0 && adfIdx < 9 && adfSwitchTime[adfIdx] > 0) {
                            unsigned int elapsed = GetTickCount() - adfSwitchTime[adfIdx];
                            savedPos = adfLastPosition[adfIdx] + elapsed;
                        }
                    }

                    startStream(walkmanStream);

                    if (savedPos > 0) {
                        // Get total length to wrap around for ADF looping
                        unsigned int total_ms = 0;
                        streamMsPosition(walkmanStream, &total_ms, nullptr);
                        if (total_ms > 0) savedPos = savedPos % total_ms;
                        setStreamMsPosition(walkmanStream, savedPos);
                    }
                }
            }
        }
        else if (walkmanStream) {
            // Pump the Miles stream so its internal state updates (required for SMP_DONE)
            serviceStream(walkmanStream, 1);

            // Auto-advance: check if Miles reports the stream as finished
            int status = streamStatus(walkmanStream);
            if (status == 2) { // SMP_DONE
                if (currentStation < stationCount) {
                    skipping = +1;
                    NextTrack(); // Advances to next MP3 and closes stream
                }
                else {
                    // Loop the native radio from the start
                    setStreamMsPosition(walkmanStream, 0);
                    startStream(walkmanStream);
                }
            }
        }
    }
}

void MusicPlayer::LoadPlaylists() {
    *gameMp3Files = 0;
    *gameTrackCount = 0;
    *gameCurrentTrack = 0;

    for (int i = 0; i < stationCount; i++) {
        MP3Injection::InjectPlaylist(i);
    }

    *gameMp3Files = (unsigned int)mp3Stations[0].mp3Start;
    *gameTrackCount = mp3Stations[0].trackCount;
}

void MusicPlayer::NextTrack() {
    if (walkmanStream) {
        closeStream(walkmanStream);
        closeStream(walkmanStream);
        walkmanStream = 0;
    }

    if (mp3Stations[currentStation].shuffle && *gameTrackCount > 0) {
        *gameCurrentTrack = rand() % (*gameTrackCount);
    }
    else {
        *gameCurrentTrack = *gameCurrentTrack + skipping;
        if ((int)*gameCurrentTrack == -1) *gameCurrentTrack = *gameTrackCount - 1;
        else if (*gameCurrentTrack == *gameTrackCount) *gameCurrentTrack = 0;
    }
    mp3Stations[currentStation].currentTrack = *gameCurrentTrack;

    if (mp3Stations[currentStation].fade > 0) {
        fade = mp3Stations[currentStation].fade;
    }
}

void MusicPlayer::DeletePlaylists() {
    for (int i = 0; i < stationCount; i++) {
        free(mp3Stations[i].name);
        free(mp3Stations[i].playlist);

        Mp3File* temp = mp3Stations[i].mp3Start;
        while (temp != nullptr) {
            Mp3File* next = (Mp3File*)temp->nextFile;

            free(temp->album);
            free(temp->artist);
            free(temp->title);
            free(temp);

            temp = next;
        }
    }
}


void MusicPlayer::ChangeMp3Station(int stationIndex) {
    if (stationIndex < stationCount) {
        if (currentStation < stationCount && currentStation != stationIndex) {
            unsigned int totalMs;
            if (*gameCurrentStream) {
                streamMsPosition(*gameCurrentStream, &totalMs, 0);
                setStreamMsPosition(*gameCurrentStream, totalMs - 10);
            }
            mp3Stations[currentStation].currentTrack = *gameCurrentTrack;
        }

        currentStation = stationIndex;
        *gameMp3Files = (unsigned int)mp3Stations[stationIndex].mp3Start;
        *gameTrackCount = mp3Stations[stationIndex].trackCount;
        *gameCurrentTrack = mp3Stations[stationIndex].currentTrack;
        listCurrentItem = mp3Stations[stationIndex].currentTrack;

        if (mp3Stations[stationIndex].fade > 0) {
            fade = mp3Stations[stationIndex].fade;
        }

        // Save position before switching
        if (walkmanStream) {
            unsigned int total_ms = 0, current_ms = 0;
            streamMsPosition(walkmanStream, &total_ms, &current_ms);

            if (currentStation < stationCount) {
                mp3Stations[currentStation].lastPositionMs = current_ms;
            }
            else {
                int adfIdx = currentStation - stationCount;
                if (adfIdx >= 0 && adfIdx < 9) {
                    adfLastPosition[adfIdx] = current_ms;
                    adfSwitchTime[adfIdx] = GetTickCount();
                }
            }

            closeStream(walkmanStream);
            walkmanStream = 0;
        }
    }
}
#ifdef GTAVC
const char* gameRadioNames[] = {
    "Wildstyle", "Flash FM", "K-Chat", "Fever 105", "V-Rock", "VCPR",
    "Radio Espantoso", "Emotion 98.3", "Wave 103", "Radio Off"
};
#elif defined(GTA3)
const char* gameRadioNames[] = {
    "Head Radio","Double Clef FM","K - Jah","Rise FM","Lips 106",
    "Game FM","MSX FM","Flashback 95.6","Chatterbox FM","MP3 Player",
    "CD Player(XBox)","Radio Off"
};
#else
#endif
void MusicPlayer::CycleStation(int dir) {
    int totalStations = stationCount + GAME_STATION_COUNT + 1; // +1 for OFF
    int nextStation = currentStation + dir;

    if (nextStation < 0) nextStation = totalStations - 1;
    if (nextStation >= totalStations) nextStation = 0;

    if (nextStation < stationCount) {
        ChangeMp3Station(nextStation);
    }
    else {
        // Save position before switching
        if (walkmanStream) {
            unsigned int total_ms = 0, current_ms = 0;
            streamMsPosition(walkmanStream, &total_ms, &current_ms);

            if (currentStation < stationCount) {
                mp3Stations[currentStation].lastPositionMs = current_ms;
            }
            else {
                int adfIdx = currentStation - stationCount;
                if (adfIdx >= 0 && adfIdx < GAME_STATION_COUNT) {
                    adfLastPosition[adfIdx] = current_ms;
                    adfSwitchTime[adfIdx] = GetTickCount();
                }
            }

            closeStream(walkmanStream);
            walkmanStream = 0;
        }

        currentStation = nextStation;
        fade = 200;
    }
}
#endif