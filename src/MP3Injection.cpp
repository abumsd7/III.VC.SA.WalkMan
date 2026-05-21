#include "../includes/MP3Injection.h"
#include <filesystem>
#include <vector>

#ifdef GTASA
#include <bass.h>
#else
#include "../includes/MusicPlayer.h"
extern unsigned int* gameHDigDriver;
#endif

using namespace std;

void MP3Injection::InjectPlaylist(int stationIndex) {
    string folderPath = WalkmanState::mp3Stations[stationIndex].playlist;
    Mp3File* previousFile = nullptr;

    vector<string> mp3Files = plugin::GetAllFilesInFolder(folderPath, ".mp3", true);

    for (const string& filePath : mp3Files) {
        Mp3File* file = (Mp3File*)malloc(sizeof(Mp3File));
        strcpy_s(file->filename, sizeof(file->filename), filePath.c_str());
        file->nextFile = 0;
        file->unknown1 = 0;
        file->unknown2 = 0;
        file->prevFile = 0;
        file->album = nullptr;
        file->artist = nullptr;
        file->title = nullptr;

#ifdef GTASA
        HSTREAM streamHandle = BASS_StreamCreateFile(FALSE, file->filename, 0, 0, BASS_STREAM_DECODE);
        if (streamHandle) {
            UpdateMp3InfoWithID3v2Tags(file);
            if (file->title == nullptr || file->title[0] == '\0') {
                UpdateMp3InfoWithID3v1Tags(file);
                if (file->title == nullptr || file->title[0] == '\0') {
                    char* fileNameOnly = strrchr(file->filename, '\\');
                    if (fileNameOnly) fileNameOnly++; else fileNameOnly = file->filename;
                    file->title = _strdup(fileNameOnly);
                }
            }

            QWORD len = BASS_ChannelGetLength(streamHandle, BASS_POS_BYTE);
            double lenSec = BASS_ChannelBytes2Seconds(streamHandle, len);
            file->trackLength = (unsigned int)(lenSec * 1000.0);
            BASS_StreamFree(streamHandle);
#else
        unsigned int streamHandle = MusicPlayer::openStream(*gameHDigDriver, file->filename, 0);
        if (streamHandle) {
            UpdateMp3InfoWithID3v2Tags(file);
            if (file->title == nullptr || file->title[0] == '\0') {
                UpdateMp3InfoWithID3v1Tags(file);
                if (file->title == nullptr || file->title[0] == '\0') {
                    char* fileNameOnly = strrchr(file->filename, '\\');
                    if (fileNameOnly) fileNameOnly++; else fileNameOnly = file->filename;
                    file->title = _strdup(fileNameOnly);
                }
            }

            MusicPlayer::streamMsPosition(streamHandle, &file->trackLength, 0);
            MusicPlayer::closeStream(streamHandle);
#endif

            if (WalkmanState::mp3Stations[stationIndex].mp3Start == nullptr) {
                WalkmanState::mp3Stations[stationIndex].mp3Start = file;
            }
            WalkmanState::mp3Stations[stationIndex].trackCount += 1;

            if (previousFile) {
                file->prevFile = (unsigned int)previousFile;
                previousFile->nextFile = (unsigned int)file;
            }
            previousFile = file;
        }
        else {
            free(file);
        }
    }
}

void MP3Injection::UpdateMp3InfoWithID3v1Tags(Mp3File* file) {
    FILE* mp3 = nullptr;
    fopen_s(&mp3, file->filename, "rb");
    if (mp3) {
        ID3v1 id3tag;
        fseek(mp3, -128, SEEK_END);
        fread(&id3tag, 128, 1, mp3);

        if (id3tag.tag[0] == 'T' && id3tag.tag[1] == 'A' && id3tag.tag[2] == 'G') {
            file->album = (char*)malloc(31);
            memcpy(file->album, id3tag.album, 30);
            file->album[30] = 0;

            file->artist = (char*)malloc(31);
            memcpy(file->artist, id3tag.artist, 30);
            file->artist[30] = 0;

            file->title = (char*)malloc(31);
            memcpy(file->title, id3tag.title, 30);
            file->title[30] = 0;
        }
        fclose(mp3);
    }
}

void MP3Injection::UpdateMp3InfoWithID3v2Tags(Mp3File* file) {
    FILE* f = nullptr;
    fopen_s(&f, file->filename, "rb");
    if (f) {
        unsigned char header[10];
        if (fread(header, 10, 1, f) == 1 && header[0] == 'I' && header[1] == 'D' && header[2] == '3') {
            int tagSize = (int)((header[6] & 0x7F) << 21) | (int)((header[7] & 0x7F) << 14) | (int)((header[8] & 0x7F) << 7) | (int)(header[9] & 0x7F);
            unsigned char* buffer = (unsigned char*)malloc(tagSize);
            fread(buffer, tagSize, 1, f);

            unsigned char* tagPtr = buffer;
            while (tagPtr < buffer + tagSize) {
                char frameID[5] = { 0 };
                memcpy(frameID, tagPtr, 4);
                unsigned int frameSize = (unsigned int)(tagPtr[4] << 24) | (tagPtr[5] << 16) | (tagPtr[6] << 8) | tagPtr[7];
                if (frameSize == 0 || frameSize > (unsigned int)(buffer + tagSize - tagPtr)) break;

                if (frameID[0] == 'T') {
                    unsigned char encoding = tagPtr[10];
                    char tempStr[512] = { 0 };

                    if (encoding == 0 || encoding == 3) { // ISO-8859-1 or UTF-8
                        size_t len = frameSize - 1;
                        if (len > 511) len = 511;
                        memcpy(tempStr, tagPtr + 11, len);
                    }
                    else if (encoding == 1) { // UTF-16 with BOM
                        wchar_t wtemp[512] = { 0 };
                        size_t len = (frameSize - 1) / 2;
                        unsigned char bom1 = tagPtr[11];
                        unsigned char bom2 = tagPtr[12];
                        if (bom1 == 0xFF && bom2 == 0xFE) { // Little Endian
                            size_t maxLen = len > 511 ? 511 : len;
                            memcpy(wtemp, tagPtr + 13, maxLen * 2);
                        }
                        else if (bom1 == 0xFE && bom2 == 0xFF) { // Big Endian
                            size_t maxLen = len > 511 ? 511 : len;
                            for (size_t k = 0; k < maxLen; k++) {
                                wtemp[k] = (tagPtr[13 + k*2] << 8) | tagPtr[13 + k*2 + 1];
                            }
                        }
                        WideCharToMultiByte(CP_ACP, 0, wtemp, -1, tempStr, 512, nullptr, nullptr);
                    }
                    else if (encoding == 2) { // UTF-16BE without BOM
                        wchar_t wtemp[512] = { 0 };
                        size_t len = (frameSize - 1) / 2;
                        size_t maxLen = len > 511 ? 511 : len;
                        for (size_t k = 0; k < maxLen; k++) {
                            wtemp[k] = (tagPtr[11 + k*2] << 8) | tagPtr[11 + k*2 + 1];
                        }
                        WideCharToMultiByte(CP_ACP, 0, wtemp, -1, tempStr, 512, nullptr, nullptr);
                    }

                    if (tempStr[0] != '\0') {
                        if (!strcmp(frameID, "TALB")) file->album = _strdup(tempStr);
                        else if (!strcmp(frameID, "TPE1")) file->artist = _strdup(tempStr);
                        else if (!strcmp(frameID, "TIT2")) file->title = _strdup(tempStr);
                    }
                }
                tagPtr += 10 + frameSize;
            }
            free(buffer);
        }
        fclose(f);
    }
}
