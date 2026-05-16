#ifdef GTASA
#include "../includes/MusicPlayerSA.h"
#include "../includes/MusicPlayer.h"
#include "../includes/MP3Injection.h"
#include "../includes/SomeMacros.h"
#include <plugin.h>
#include <CPad.h>
#include <CTimer.h>
#include <CMenuManager.h>
#include <CCamera.h>
#include <patch.h>
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace plugin;
namespace fs = filesystem;

// Static member definitions
DWORD MusicPlayerSA::walkmanStream = 0;

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

    WalkmanState::ReadConfig();
    LoadPlaylists();
    currentStation = stationCount + GAME_STATION_COUNT;
    InputHandler::ResetKeyState();
    DumpRadioTables();
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
        }
        else {
            // Handle pausing
            if (CTimer::m_UserPause) {
                BASS_ChannelPause(walkmanStream);
            }
            else {
                BASS_ChannelPlay(walkmanStream, FALSE);
            }

            BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);

            if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_STOPPED) {
                skipping = 1;
                NextTrack();
            }
        }
    }
    else {
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
    }
    else {
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
    int totalStations = stationCount + GAME_STATION_COUNT + 1; // +1 for OFF
    int nextStation = currentStation + dir;

    if (nextStation < 0) nextStation = totalStations - 1;
    if (nextStation >= totalStations) nextStation = 0;

    if (nextStation < stationCount) {
        ChangeMp3Station(nextStation);
    }
    else {
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

void MusicPlayerSA::DumpRadioTables() {
    std::ofstream out("radio_sound_tables_dump.txt");
    if (!out.is_open()) return;

    out << "=========================================================================\n";
    out << "              GTA SAN ANDREAS NATIVE RADIO SOUND TABLES DUMP             \n";
    out << "=========================================================================\n\n";

    const char* stationNames[13] = {
        "Playback FM", "K-Rose", "K-DST", "Bounce FM", "SF-UR",
        "Radio Los Santos", "Radio X", "CSR 103.9", "K-JAH West",
        "Master Sounds 98.3", "WCTR", "User Tracks", "Radio Off"
    };

    struct MinMax { int minId; int maxId; };

    auto dumpTableExact = [&](const char* tableName, MinMax* table, int count) {
        out << "--- " << tableName << " ---\n";
        for (int i = 0; i < count; i++) {
            out << std::setw(20) << std::left << stationNames[i] << " [Min: " << std::setw(5) << table[i].minId << ", Max: " << std::setw(5) << table[i].maxId << "] Exact IDs: ";
            if (table[i].minId == 0 && table[i].maxId == 0) {
                out << "NONE\n";
                continue;
            }
            if (table[i].minId == 1922) { // 0x782 is the game's internal "none" / dummy ID
                out << "NONE (1922)\n";
                continue;
            }
            for (int id = table[i].minId; id <= table[i].maxId; id++) {
                out << id << (id < table[i].maxId ? ", " : "");
            }
            out << "\n";
        }
        out << "\n";
        };

    // Dump Idents and ALL 10 Banter Tables
    dumpTableExact("gRadioIdents (Station Bumpers / Idents)", (MinMax*)0x8C8FB0, 13);
    dumpTableExact("gRadioDJBanterST (Station Startup Banter)", (MinMax*)0x8C8BF0, 12);
    dumpTableExact("gRadioDJBanterSP (Special DJ Banter)", (MinMax*)0x8C8C50, 12);
    dumpTableExact("gRadioDJBanterBC (General DJ Banter)", (MinMax*)0x8C8CB0, 12);
    dumpTableExact("gRadioDJBanterAF (Afternoon DJ Banter)", (MinMax*)0x8C8D10, 12);
    dumpTableExact("gRadioDJBanterEV (Evening DJ Banter)", (MinMax*)0x8C8D70, 12);
    dumpTableExact("gRadioDJBanterMO (Morning DJ Banter)", (MinMax*)0x8C8DD0, 12);
    dumpTableExact("gRadioDJBanterTN (Night DJ Banter)", (MinMax*)0x8C8E30, 12);
    dumpTableExact("gRadioDJBanterWE_SUNNY (Sunny Weather Banter)", (MinMax*)0x8C8E90, 12);
    dumpTableExact("gRadioDJBanterWE_RAINY (Rainy Weather Banter)", (MinMax*)0x8C8EF0, 12);
    dumpTableExact("gRadioDJBanterWE_FOGGY (Foggy Weather Banter)", (MinMax*)0x8C8F50, 12);

    // gRadioAdverts: 0x8C8B88 (8 bytes)
    MinMax* adverts = (MinMax*)0x8C8B88;
    out << "--- gRadioAdverts (Global Commercials) ---\n";
    out << "Global Adverts Range: MinSoundID = " << adverts->minId << ", MaxSoundID = " << adverts->maxId << "\n";
    out << "Exact Advert IDs: ";
    for (int id = adverts->minId; id <= adverts->maxId; id++) {
        out << id << (id < adverts->maxId ? ", " : "");
    }
    out << "\n\n";

    // Music Intros, Tracks, Outros (12 stations, 31 tracks each)
    out << "--- gRadioMusicIntros, gRadioMusicTracks, gRadioMusicOutros ---\n";
    MinMax* intros = (MinMax*)0x8C9610;
    int* tracks = (int*)0x8C9040;
    MinMax* outros = (MinMax*)0x8CA1B0;

    for (int s = 0; s < 12; s++) { // Station 12 is User Tracks, not in these tables
        out << "=== " << stationNames[s] << " ===\n";
        for (int t = 0; t < 31; t++) {
            int idx = s * 31 + t;
            if (tracks[idx] == 0 && intros[idx].minId == 0 && outros[idx].minId == 0) {
                continue; // Skip empty slots
            }
            out << "  Track " << std::setw(2) << t << ": TrackSoundID = " << std::setw(6) << tracks[idx]
                << " | Intro [Min = " << std::setw(5) << intros[idx].minId << ", Max = " << std::setw(5) << intros[idx].maxId << "]"
                << " | Outro [Min = " << std::setw(5) << outros[idx].minId << ", Max = " << std::setw(5) << outros[idx].maxId << "]\n";
        }
        out << "\n";
    }

    out.close();
}
#endif