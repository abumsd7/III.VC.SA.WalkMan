#ifndef GTASA
#include "../includes/MusicPlayer.h"
#include "../includes/MP3Injection.h"
#include "../includes/SomeMacros.h"
#include <CFont.h>
#include <CHud.h>
#include <CMenuManager.h>
#include <CPad.h>
#include <cDMAudio.h>
#include <extensions/Paths.h>
#include <filesystem>
#include <patch.h>
#include <stdio.h>

#ifdef GTA3
#include <cMusicManager.h>
#endif

using namespace plugin;

p_AIL_close_stream MusicPlayer::closeStream = nullptr;
p_AIL_stream_ms_position MusicPlayer::streamMsPosition = nullptr;
p_AIL_set_stream_ms_position MusicPlayer::setStreamMsPosition = nullptr;
p_AIL_open_stream MusicPlayer::openStream = nullptr;
p_AIL_pause_stream MusicPlayer::pauseStream = nullptr;

unsigned int MusicPlayer::adfLastPosition[12] = { 0 };
unsigned int MusicPlayer::walkmanStream = 0;
unsigned int MusicPlayer::adfSwitchTime[12] = { 0 };

#ifdef GTAVC
const char* gameRadioNames[10] = { "Wildstyle", "Flash FM", "K-Chat", "Fever 105", "V-Rock", "VCPR", "Espantoso", "Emotion 98.3", "Wave 103", "Radio Off" };
#else
const char* gameRadioNames[12] = { "Head Radio", "Double Clef FM", "Jah Radio", "Rise FM", "Lips 106", "Game Radio", "MSX FM", "Head Radio 2", "Flashback FM", "Chatterbox", "User Tracks", "Radio Off" };
#endif

p_AIL_start_stream MusicPlayer::startStream = nullptr;
p_AIL_set_stream_volume MusicPlayer::setStreamVolume = nullptr;
p_AIL_stream_status MusicPlayer::streamStatus = nullptr;
p_AIL_service_stream MusicPlayer::serviceStream = nullptr;

void MusicPlayer::Initialise() {

  HMODULE hMss = GetModuleHandleA("mss32.dll");
  if (!hMss) hMss = LoadLibraryA("mss32.dll");
  if (hMss) {
    closeStream = (p_AIL_close_stream)GetProcAddress(hMss, "_AIL_close_stream@4");
    streamMsPosition = (p_AIL_stream_ms_position)GetProcAddress(hMss, "_AIL_stream_ms_position@12");
    setStreamMsPosition = (p_AIL_set_stream_ms_position)GetProcAddress(hMss, "_AIL_set_stream_ms_position@8");
    openStream = (p_AIL_open_stream)GetProcAddress(hMss, "_AIL_open_stream@12");
    pauseStream = (p_AIL_pause_stream)GetProcAddress(hMss, "_AIL_pause_stream@8");
    startStream = (p_AIL_start_stream)GetProcAddress(hMss, "_AIL_start_stream@4");
    setStreamVolume = (p_AIL_set_stream_volume)GetProcAddress(hMss, "_AIL_set_stream_volume@8");
    streamStatus = (p_AIL_stream_status)GetProcAddress(hMss, "_AIL_stream_status@4");
    serviceStream = (p_AIL_service_stream)GetProcAddress(hMss, "_AIL_service_stream@8");
  }
  WalkmanState::ReadConfig();
  currentStation = stationCount + GAME_STATION_COUNT; // Initialize to OFF

  if (stationCount > 0) {
#ifdef GTAVC
    // Hook into mp3 processing
    patch::ReplaceFunctionCall(0x5D7EA4,
                               LoadPlaylists); // patching call load_mp3_files
    patch::ReplaceFunctionCall(
        0x5D7340, DeletePlaylists); // patching call delete_mp3_files

    // Force MP3 Radio Channel to be available (0x5D80E0 is
    // IsMP3RadioChannelAvailable in this version)
    patch::SetChar(0x5D80E0, static_cast<char>(0xB8)); // mov eax, 1
    patch::SetInt(0x5D80E1, 1);
    patch::SetChar(0x5D80E5, static_cast<char>(0xC3)); // ret
#else
    // Hook into mp3 processing
    patch::ReplaceFunctionCall(0x566C7D, LoadPlaylists);
    patch::ReplaceFunctionCall(0x566E2D, DeletePlaylists);

    // bRadioOff = true;
    // Force MP3 Radio Channel to be available
    patch::SetChar(0x57A9C0, static_cast<char>(0xB8)); // mov eax, 1
    patch::SetInt(0x57A9C1, 1);
    patch::SetChar(0x57A9C5, static_cast<char>(0xC3)); // ret
#endif
  }

  LoadPlaylists();

  // Seed ADF broadcast clocks so stations "play" from game start
  unsigned int startTime = GetTickCount();
  for (int i = 0; i < GAME_STATION_COUNT; i++)
    adfSwitchTime[i] = startTime;
  InputHandler::ResetKeyState();
}

void MusicPlayer::Update() {
  WalkmanState::ProcessInput();

  // Pause handling
  if (*gameUserPause == 1) {
    if (walkmanStream)
      pauseStream(walkmanStream, 1);
    return; // Don't process playback while paused
  } else {
    if (walkmanStream)
      pauseStream(walkmanStream, 0);
  }

#ifdef GTA3
  // Toggle cMusicManager::ServiceGameMode (0x57D690) based on WalkMan state.
  // When WalkMan is active: patch to 'ret' so native radio can't play.
  // When WalkMan is OFF: restore original byte so native radio works.
  {
    static unsigned char sgmOrigByte = 0;
    if (sgmOrigByte == 0)
      sgmOrigByte = *(unsigned char *)0x57D690;

    bool walkmanActive = (currentStation != stationCount + GAME_STATION_COUNT);
    if (walkmanActive && *(unsigned char *)0x57D690 != 0xC3)
      patch::SetChar(0x57D690, static_cast<char>(0xC3));
    else if (!walkmanActive && *(unsigned char *)0x57D690 == 0xC3)
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
  } else {
    // Radio is ON

#ifdef GTAVC
    CVehicle *playerVeh = FindPlayerVehicle();
    if (playerVeh) {
      if (playerVeh->m_nRadioStation != RADIO_OFF_VAL) {
        playerVeh->m_nRadioStation = RADIO_OFF_VAL;
      }
    }
#endif
    if (!walkmanStream) {
      const char *pathToPlay = nullptr;

      if (currentStation < stationCount) {
        // Custom MP3
        Mp3File *file = GetMp3Track(*gameCurrentTrack);
        if (file)
          pathToPlay = file->filename;
      } else {
        // Native Radio Station
        int gameRadioID = currentStation - stationCount;
#ifdef GTAVC
        static const char *nativePaths[] = {
            "WILD.ADF", "FLASH.ADF",  "KCHAT.ADF",   "FEVER.ADF", "VROCK.ADF",
            "VCPR.ADF", "ESPANT.ADF", "EMOTION.ADF", "WAVE.ADF"};
#else
        static const char *nativePaths[] = {
            "CHAT.wav",  "RISE.wav", "MSX.wav",  "HEAD.wav", "GAME.wav",
            "CLASS.wav", "LIPS.wav", "KJAH.wav", "FLASH.wav"};
#endif
        if (gameRadioID >= 0 && gameRadioID < 9) {
          static char safeAdfPath[260];
          // We MUST use a writable buffer because sampman_miles.cpp
          // uses strcpy to replace ".ADF" with ".mp3" internally!
          snprintf(safeAdfPath, sizeof(safeAdfPath), "AUDIO\\%s",
                   nativePaths[gameRadioID]);
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
          } else {
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
            if (total_ms > 0)
              savedPos = savedPos % total_ms;
            setStreamMsPosition(walkmanStream, savedPos);
          }
        }
      }
    } else if (walkmanStream) {
      // Pump the Miles stream so its internal state updates (required for
      // SMP_DONE)
      serviceStream(walkmanStream, 1);

      // Auto-advance: check if Miles reports the stream as finished
      int status = streamStatus(walkmanStream);
      if (status == 2) { // SMP_DONE
        if (currentStation < stationCount) {
          skipping = +1;
          NextTrack(); // Advances to next MP3 and closes stream
        } else {
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
  } else {
    *gameCurrentTrack = *gameCurrentTrack + skipping;
    if ((int)*gameCurrentTrack == -1)
      *gameCurrentTrack = *gameTrackCount - 1;
    else if (*gameCurrentTrack == *gameTrackCount)
      *gameCurrentTrack = 0;
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

    Mp3File *temp = mp3Stations[i].mp3Start;
    while (temp != nullptr) {
      Mp3File *next = (Mp3File *)temp->nextFile;

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
      } else {
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

void MusicPlayer::CycleStation(int dir) {
  int totalStations = stationCount + GAME_STATION_COUNT + 1; // +1 for OFF
  int nextStation = currentStation + dir;

  if (nextStation < 0)
    nextStation = totalStations - 1;
  if (nextStation >= totalStations)
    nextStation = 0;

  if (nextStation < stationCount) {
    ChangeMp3Station(nextStation);
  } else {
    // Save position before switching
    if (walkmanStream) {
      unsigned int total_ms = 0, current_ms = 0;
      streamMsPosition(walkmanStream, &total_ms, &current_ms);

      if (currentStation < stationCount) {
        mp3Stations[currentStation].lastPositionMs = current_ms;
      } else {
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


