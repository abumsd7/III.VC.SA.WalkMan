#include "../includes/WalkmanState.h"
#include "../includes/WalkManConfig.h"
#include "../includes/SomeMacros.h"
#include <filesystem>
#include <extensions/Paths.h>

#ifdef GTASA
#include "../includes/MusicPlayerSA.h"
#include <CPad.h>
#include <bass.h>
#else
#include "../includes/MusicPlayer.h"
#endif

// Static member definitions
Mp3Station WalkmanState::mp3Stations[10];
int WalkmanState::currentStation = 0;
int WalkmanState::stationCount = 0;
int WalkmanState::skipping = 0;
int WalkmanState::fade = 0;
bool WalkmanState::listActive = false;
int WalkmanState::listCurrentItem = 0;
float WalkmanState::listAlpha = 0.0f;
float WalkmanState::listFade = 0.0f;
int WalkmanState::walkmanVolume = 64;

extern unsigned char* gameDisableKeyboard2;
#ifndef GTASA
extern unsigned int* gameTrackCount;
extern unsigned int* gameCurrentTrack;
extern const char* gameRadioNames[];
#else
extern const char* gameRadioNames[13];
#endif

void WalkmanState::Initialise() {
    for (int i = 0; i < 10; i++) {
        mp3Stations[i].name = nullptr;
        mp3Stations[i].playlist = nullptr;
        mp3Stations[i].showTitle = true;
        mp3Stations[i].fade = 200;
        mp3Stations[i].shuffle = false;
        mp3Stations[i].mp3Start = nullptr;
        mp3Stations[i].currentTrack = 0;
        mp3Stations[i].trackCount = 0;
        mp3Stations[i].lastPositionMs = 0;
        mp3Stations[i].lastSwitchTimeMs = GetTickCount();
	}
}

void WalkmanState::ReadConfig() {
    config.Read();

    // Scan mp3_stations folder for subfolders
    stationCount = 0;
    string rootPath = GAME_PATH((char*)"mp3_stations");
    if (filesystem::exists(rootPath)) {
        for (const auto& entry : filesystem::directory_iterator(rootPath)) {
            if (entry.is_directory() && stationCount < 10) {
                mp3Stations[stationCount].name = _strdup(entry.path().filename().string().c_str());
                mp3Stations[stationCount].playlist = _strdup(entry.path().string().c_str()); // Store folder path
                mp3Stations[stationCount].showTitle = true;
                mp3Stations[stationCount].fade = 200; // Default fade
                mp3Stations[stationCount].shuffle = false;
                mp3Stations[stationCount].mp3Start = nullptr;
                mp3Stations[stationCount].trackCount = 0;
                mp3Stations[stationCount].currentTrack = 0;
                mp3Stations[stationCount].lastPositionMs = 0;
                mp3Stations[stationCount].lastSwitchTimeMs = GetTickCount();
                stationCount++;
            }
        }
    }
}

Mp3File* WalkmanState::GetMp3Track(int trackIndex) {
    if (currentStation >= stationCount) return nullptr;
    Mp3File* temp = mp3Stations[currentStation].mp3Start;
    int i = 0;
    while (i < trackIndex && temp) {
        temp = (Mp3File*)temp->nextFile;
        i++;
    }
    return temp;
}

bool WalkmanState::IsRadioOff() {
#ifdef GTASA
    return currentStation == stationCount + GAME_STATION_COUNT;
#elif defined(GTAVC)
    return currentStation == stationCount + GAME_STATION_COUNT;
#elif defined(GTA3)
    return currentStation == stationCount + GAME_STATION_COUNT;
#else
    return false;
#endif
}

const char* WalkmanState::GetActiveStationName() {
    if (IsRadioOff()) {
        return "Radio OFF";
    }
    if (currentStation < stationCount) {
        return mp3Stations[currentStation].name;
    }
    int nativeIdx = currentStation - stationCount;
#ifdef GTASA
    if (nativeIdx >= 0 && nativeIdx < 13) return gameRadioNames[nativeIdx];
#elif defined(GTAVC)
    if (nativeIdx >= 0 && nativeIdx < 10) return gameRadioNames[nativeIdx];
#elif defined(GTA3)
    if (nativeIdx >= 0 && nativeIdx < 12) return gameRadioNames[nativeIdx];
#endif
    return "Unknown";
}

const char* WalkmanState::GetActiveTrackTitle() {
#ifdef GTASA
    return MusicPlayerSA::GetActiveTrackTitle();
#else
    return "";
#endif
}

void WalkmanState::GetTrackPlaybackInfo(unsigned int& current_ms, unsigned int& total_ms) {
    current_ms = 0;
    total_ms = 0;
#ifdef GTASA
    if (MusicPlayerSA::walkmanStream) {
        QWORD pos = BASS_ChannelGetPosition(MusicPlayerSA::walkmanStream, BASS_POS_BYTE);
        QWORD len = BASS_ChannelGetLength(MusicPlayerSA::walkmanStream, BASS_POS_BYTE);
        double posSec = BASS_ChannelBytes2Seconds(MusicPlayerSA::walkmanStream, pos);
        double lenSec = BASS_ChannelBytes2Seconds(MusicPlayerSA::walkmanStream, len);
        current_ms = (unsigned int)(posSec * 1000.0);
        total_ms = (unsigned int)(lenSec * 1000.0);
    }
#else
    if (MusicPlayer::walkmanStream && MusicPlayer::streamMsPosition) {
        MusicPlayer::streamMsPosition(MusicPlayer::walkmanStream, &total_ms, &current_ms);
    }
#endif
}

unsigned int WalkmanState::GetCurrentTrackIndex() {
#ifdef GTASA
    if (currentStation < stationCount) return mp3Stations[currentStation].currentTrack;
    return 0;
#else
    if (gameCurrentTrack) return *gameCurrentTrack;
    return 0;
#endif
}

unsigned int WalkmanState::GetTrackCount() {
#ifdef GTASA
    if (currentStation < stationCount) return mp3Stations[currentStation].trackCount;
    return 0;
#else
    if (gameTrackCount) return *gameTrackCount;
    return 0;
#endif
}

void WalkmanState::SetListActive(bool active) {
    listActive = active;
#ifdef GTASA
    if (CPad::GetPad(0)) {
        CPad::GetPad(0)->bPlayerSafe = active;
        CPad::GetPad(0)->bDisablePlayerEnterCar = active;
        CPad::GetPad(0)->bDisablePlayerDuck = active;
        CPad::GetPad(0)->bDisablePlayerFireWeapon = active;
        CPad::GetPad(0)->bDisablePlayerCycleWeapon = active;
        CPad::GetPad(0)->bDisablePlayerJump = active;
    }
#else
    if (gameDisableKeyboard2) {
        *gameDisableKeyboard2 = active;
    }
#endif
}

bool InputHandler::oldKeyState[256] = { false };

void WalkmanState::ProcessInput() {
    if (InputHandler::IsKeyJustPressed(config.ToggleList)) HandleKeyPress(config.ToggleList);

    if (listActive) {
#ifdef GTASA
        if (CPad::GetPad(0)) {
            CPad::GetPad(0)->Clear(false, false);
            CPad::GetPad(0)->NewState.LeftStickX = 0;
            CPad::GetPad(0)->NewState.LeftStickY = 0;
            CPad::GetPad(0)->NewState.DPadUp = 0;
            CPad::GetPad(0)->NewState.DPadDown = 0;
            CPad::GetPad(0)->NewState.DPadLeft = 0;
            CPad::GetPad(0)->NewState.DPadRight = 0;
        }
#endif
        if (InputHandler::IsKeyJustPressed(config.ListChoose) || InputHandler::IsKeyJustPressed(VK_RETURN)) HandleKeyPress(VK_RETURN);
        if (InputHandler::IsKeyJustPressed(config.ListScrollUp) || InputHandler::IsKeyJustPressed(VK_UP)) HandleKeyPress(VK_UP);
        if (InputHandler::IsKeyJustPressed(config.ListScrollDown) || InputHandler::IsKeyJustPressed(VK_DOWN)) HandleKeyPress(VK_DOWN);
        if (InputHandler::IsKeyJustPressed(VK_LEFT)) HandleKeyPress(VK_LEFT);
        if (InputHandler::IsKeyJustPressed(VK_RIGHT)) HandleKeyPress(VK_RIGHT);
    }
    else {
        for (int i = 0x31; i <= 0x39; i++) {
            if (InputHandler::IsKeyJustPressed(i)) HandleKeyPress(i);
        }

        if (InputHandler::IsKeyJustPressed(config.PrevTrack)) HandleKeyPress(config.PrevTrack);
        if (InputHandler::IsKeyJustPressed(config.NextTrack)) HandleKeyPress(config.NextTrack);
        if (InputHandler::IsKeyJustPressed(config.ToggleShuffle)) HandleKeyPress(config.ToggleShuffle);
        if (InputHandler::IsKeyJustPressed(config.VolumeUp)) HandleKeyPress(config.VolumeUp);
        if (InputHandler::IsKeyJustPressed(config.VolumeDown)) HandleKeyPress(config.VolumeDown);
    }

    InputHandler::UpdateOldKeyState();
}

void WalkmanState::HandleKeyPress(int key) {
    skipping = 0;

    if (key == config.ToggleList) {
        SetListActive(!listActive);

        if (listActive) {
            listCurrentItem = GetCurrentTrackIndex();
            listAlpha = 255.0f;
            listFade = 0.0f;
        }
    }
    else if (listActive && (key == config.ListChoose || key == VK_RETURN)) {
        if (currentStation < stationCount) {
            if (GetCurrentTrackIndex() != listCurrentItem) {
                SetCurrentTrackIndex(listCurrentItem);
                StopStream();
            }
        }
        listFade = -20.0f;
    }
    else if (listActive && (key == config.ListScrollUp || key == VK_UP)) {
        if (listCurrentItem > 0) listCurrentItem -= 1;
    }
    else if (listActive && (key == config.ListScrollDown || key == VK_DOWN)) {
        if (listCurrentItem < (int)GetTrackCount() - 1) listCurrentItem += 1;
    }
    else if (listActive && key == VK_LEFT) {
        CycleStation(-1);
    }
    else if (listActive && key == VK_RIGHT) {
        CycleStation(1);
    }
    else if (key >= 0x31 && key <= 0x39) {
        ChangeMp3Station(key - 0x31);
    }
    else if (key == config.PrevTrack) {
        StopStream();
        skipping = -1;
        NextTrack();
    }
    else if (key == config.NextTrack) {
        StopStream();
        skipping = +1;
        NextTrack();
    }
    else if (key == config.ToggleShuffle) {
        if (currentStation < stationCount)
            mp3Stations[currentStation].shuffle = !mp3Stations[currentStation].shuffle;
    }
    else if (key == config.VolumeUp) {
        if (walkmanVolume < 127) walkmanVolume += 2;
        if (walkmanVolume > 127) walkmanVolume = 127;
        SetStreamVolume(walkmanVolume);
    }
    else if (key == config.VolumeDown) {
        if (walkmanVolume > 0) walkmanVolume -= 2;
        if (walkmanVolume < 0) walkmanVolume = 0;
        SetStreamVolume(walkmanVolume);
    }
}

void WalkmanState::StopStream() {
#ifdef GTASA
    if (MusicPlayerSA::walkmanStream) {
        BASS_StreamFree(MusicPlayerSA::walkmanStream);
        MusicPlayerSA::walkmanStream = 0;
    }
#else
    if (MusicPlayer::walkmanStream && MusicPlayer::closeStream) {
        MusicPlayer::closeStream(MusicPlayer::walkmanStream);
        MusicPlayer::walkmanStream = 0;
    }
#endif
}

void WalkmanState::SetCurrentTrackIndex(int idx) {
#ifdef GTASA
    if (currentStation < stationCount) mp3Stations[currentStation].currentTrack = idx;
#else
    if (gameCurrentTrack) *gameCurrentTrack = idx;
#endif
}

void WalkmanState::CycleStation(int dir) {
#ifdef GTASA
    MusicPlayerSA::CycleStation(dir);
#else
    MusicPlayer::CycleStation(dir);
#endif
}

void WalkmanState::ChangeMp3Station(int idx) {
#ifdef GTASA
    MusicPlayerSA::ChangeMp3Station(idx);
#else
    MusicPlayer::ChangeMp3Station(idx);
#endif
}

void WalkmanState::NextTrack() {
#ifdef GTASA
    MusicPlayerSA::NextTrack();
#else
    MusicPlayer::NextTrack();
#endif
}

void WalkmanState::SetStreamVolume(int vol) {
#ifdef GTASA
    if (MusicPlayerSA::walkmanStream) {
        BASS_ChannelSetAttribute(MusicPlayerSA::walkmanStream, BASS_ATTRIB_VOL, vol / 127.0f);
    }
#else
    if (MusicPlayer::walkmanStream && MusicPlayer::setStreamVolume) {
        MusicPlayer::setStreamVolume(MusicPlayer::walkmanStream, vol);
    }
#endif
}

