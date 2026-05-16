#pragma once

#include <plugin.h>
#include "WalkmanState.h"

class MP3Injection {
public:
    static void InjectPlaylist(int stationIndex);
    static void UpdateMp3InfoWithID3v1Tags(Mp3File* file);
    static void UpdateMp3InfoWithID3v2Tags(Mp3File* file);
};
