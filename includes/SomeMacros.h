#pragma once
#include <extensions/Screen.h>

#define DEFAULT_SCREEN_WIDTH (640.0f)
#define DEFAULT_SCREEN_HEIGHT (448.0f)
#define DEFAULT_ASPECT_RATIO (4.0f/3.0f)

#define SCREEN_ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

#define SCREEN_STRETCH_X(a)   ((a) * (float) SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a)   ((a) * (float) SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT)

#define SCREEN_SCALE_AR(a) ((a) * DEFAULT_ASPECT_RATIO / SCREEN_ASPECT_RATIO)

#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH - SCREEN_SCALE_X(a))
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT - SCREEN_SCALE_Y(a))
#define SCALE_AND_CENTER_X(x) ((SCREEN_WIDTH == DEFAULT_SCREEN_WIDTH) ? (x) : (SCREEN_WIDTH - SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH)) / 2 + SCREEN_SCALE_X((x)))

#ifdef GTAVC
#define GAME_STATION_COUNT 9
#define RADIO_OFF_VAL 10
#elif defined (GTA3)
#define GAME_STATION_COUNT 9
#define RADIO_OFF_VAL 11
#define NO_TRACK 197
#else
#define GAME_STATION_COUNT 11
#endif

template<typename T = void>
inline T* GetSafePtr(unsigned int address) {
    volatile unsigned int key = 0x5A5A5A5A;
    return reinterpret_cast<T*>((address ^ key) ^ key);
}

inline unsigned int GetSafeAddr(unsigned int address) {
    volatile unsigned int key = 0x5A5A5A5A;
    return (address ^ key) ^ key;
}

#ifndef GTASA
#ifdef GTAVC
#define gameCurrentRadiostation (GetSafePtr<unsigned int>(0x9839BC))
#define gameMp3Files (GetSafePtr<unsigned int>(0x9753E0))
#define gameTrackCount (GetSafePtr<unsigned int>(0xA108B0))
#define gameCurrentTrack (GetSafePtr<unsigned int>(0x97881C))
#define gameCurrentStream (GetSafePtr<unsigned int>(0x978668))
#define gameHDigDriver (GetSafePtr<unsigned int>(0x978550))
#define gameDisableKeyboard2 (GetSafePtr<unsigned char>(0xA10AE4))
#define gameUserPause (GetSafePtr<unsigned char>(0xA10B36))
#else
#define gameCurrentRadiostation (GetSafePtr<unsigned char>(0x8F42BC))
#define gameMp3Files (GetSafePtr<unsigned int>(0x8E2C7C))
#define gameTrackCount (GetSafePtr<unsigned int>(0x95CC00))
#define gameCurrentTrack (GetSafePtr<unsigned int>(0x8F2558))
#define gameCurrentStream (GetSafePtr<unsigned int>(0x709C50))
#define gameHDigDriver (GetSafePtr<unsigned int>(0x8F1A24))
#define gameDisableKeyboard2 (GetSafePtr<unsigned char>(0x95CD48))
#define gameUserPause (GetSafePtr<unsigned char>(0x95CD7C))
#endif
#endif