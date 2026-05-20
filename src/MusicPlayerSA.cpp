#ifdef GTASA
#include "../includes/MusicPlayerSA.h"
#include "../includes/MusicPlayerSAMappings.h"
#include "../includes/MusicPlayerSATrackNames.h"
#include "../includes/MusicPlayer.h"
#include "../includes/MP3Injection.h"
#include "../includes/SomeMacros.h"
#include <plugin.h>
#include <CPad.h>
#include <CTimer.h>
#include <CMenuManager.h>
#include <CAudioEngine.h>
#include <CCamera.h>
#include <patch.h>
#include <filesystem>
#include <fstream>
#include <iomanip>

using namespace plugin;
namespace fs = filesystem;

// Static member definitions
DWORD MusicPlayerSA::walkmanStream = 0;
NativeStation MusicPlayerSA::nativeStations[12];
static NativeTrackStream* currentNativeStream = nullptr;
static int currentNativePakIdx = -1;
static int currentNativeTrackIdx = -1;

const unsigned char ENCODE_KEY[16] = {
    0xEA, 0x3A, 0xC4, 0xA1, 0x9A, 0xA8, 0x14, 0xF3,
    0x48, 0xB0, 0xD7, 0x23, 0x9D, 0xE8, 0xFF, 0xF1
};

static BASS_FILEPROCS nativeFileProcs = {
    MusicPlayerSA::NativeTrackCloseProc,
    MusicPlayerSA::NativeTrackLenProc,
    MusicPlayerSA::NativeTrackReadProc,
    MusicPlayerSA::NativeTrackSeekProc
};

const char* gameRadioNames[13] = {
    "Playback FM", "K-Rose", "K-DST", "Bounce FM", "SF-UR", 
    "Radio Los Santos", "Radio X", "CSR 103.9", "K-JAH West", 
    "Master Sounds 98.3", "WCTR", "User Tracks", "Radio Off"
};

void CALLBACK MusicPlayerSA::NativeTrackCloseProc(void* user) {
    // Managed cleanly in StopStreamAndSavePosition / Update
}

QWORD CALLBACK MusicPlayerSA::NativeTrackLenProc(void* user) {
    NativeTrackStream* stream = (NativeTrackStream*)user;
    if (stream) return stream->length;
    return 0;
}

DWORD CALLBACK MusicPlayerSA::NativeTrackReadProc(void* buffer, DWORD length, void* user) {
    NativeTrackStream* stream = (NativeTrackStream*)user;
    if (!stream || !stream->file.is_open()) return 0;
    
    if (stream->currentOffset >= stream->length) return 0;
    DWORD toRead = min(length, stream->length - stream->currentOffset);
    
    stream->file.seekg(stream->startOffset + stream->currentOffset, ios::beg);
    stream->file.read((char*)buffer, toRead);
    DWORD bytesRead = (DWORD)stream->file.gcount();
    
    // On-the-fly XOR decryption
    for (DWORD i = 0; i < bytesRead; i++) {
        DWORD absPos = stream->startOffset + stream->currentOffset + i;
        ((char*)buffer)[i] ^= ENCODE_KEY[absPos % 16];
    }
    
    stream->currentOffset += bytesRead;
    return bytesRead;
}

BOOL CALLBACK MusicPlayerSA::NativeTrackSeekProc(QWORD offset, void* user) {
    NativeTrackStream* stream = (NativeTrackStream*)user;
    if (!stream) return FALSE;
    if (offset > stream->length) return FALSE;
    stream->currentOffset = (DWORD)offset;
    return TRUE;
}

int MusicPlayerSA::GetPerfectSoundId(const string& stem, int trackIdx) {
    if (stem == "CH") {
        if (trackIdx < (int)(sizeof(CH_ids)/sizeof(CH_ids[0]))) return CH_ids[trackIdx];
        return 0 + trackIdx;
    }
    if (stem == "CO") {
        if (trackIdx < (int)(sizeof(CO_ids)/sizeof(CO_ids[0]))) return CO_ids[trackIdx];
        return 185 + trackIdx;
    }
    if (stem == "CR") {
        if (trackIdx < (int)(sizeof(CR_ids)/sizeof(CR_ids[0]))) return CR_ids[trackIdx];
        return 315 + trackIdx;
    }
    if (stem == "DS") {
        if (trackIdx < (int)(sizeof(DS_ids)/sizeof(DS_ids[0]))) return DS_ids[trackIdx];
        return 470 + trackIdx;
    }
    if (stem == "HC") {
        if (trackIdx < (int)(sizeof(HC_ids)/sizeof(HC_ids[0]))) return HC_ids[trackIdx];
        return 767 + trackIdx;
    }
    if (stem == "MH") {
        if (trackIdx < (int)(sizeof(MH_ids)/sizeof(MH_ids[0]))) return MH_ids[trackIdx];
        return 946 + trackIdx;
    }
    if (stem == "MR") {
        if (trackIdx < (int)(sizeof(MR_ids)/sizeof(MR_ids[0]))) return MR_ids[trackIdx];
        return 1061 + trackIdx;
    }
    if (stem == "NJ") {
        if (trackIdx < (int)(sizeof(NJ_ids)/sizeof(NJ_ids[0]))) return NJ_ids[trackIdx];
        return 1213 + trackIdx;
    }
    if (stem == "RE") {
        if (trackIdx < (int)(sizeof(RE_ids)/sizeof(RE_ids[0]))) return RE_ids[trackIdx];
        return 1360 + trackIdx;
    }
    if (stem == "RG") {
        if (trackIdx < (int)(sizeof(RG_ids)/sizeof(RG_ids[0]))) return RG_ids[trackIdx];
        return 1490 + trackIdx;
    }
    if (stem == "TK") {
        if (trackIdx < (int)(sizeof(TK_ids)/sizeof(TK_ids[0]))) return TK_ids[trackIdx];
        return 1651 + trackIdx;
    }
    if (stem == "ADVERTS") return 66 + trackIdx;
    return trackIdx;
}

void MusicPlayerSA::ScanNativeStations() {
    const char* strm_files[12] = { "CH", "CO", "CR", "DS", "HC", "MH", "MR", "NJ", "RE", "RG", "TK", "ADVERTS" };
    
    for (int i = 0; i < 12; i++) {
        string path = GAME_PATH((char*)(string("audio\\streams\\") + strm_files[i]).c_str());
        nativeStations[i].filename = path;
        nativeStations[i].currentTrackIndex = -1;
        nativeStations[i].currentTrackType = -1;
        nativeStations[i].currentDisplayTitle = "Connecting...";
        nativeStations[i].currentPakIdx = -1;
        nativeStations[i].lastPositionMs = 0;
        nativeStations[i].lastSwitchTimeMs = GetTickCount();
        
        ifstream file(path, ios::binary | ios::ate);
        if (!file.is_open()) continue;
        
        streampos totalSize = file.tellg();
        file.seekg(0, ios::beg);
        
        DWORD offset = 0;
        int trackIdx = 0;
        char hdr[8068];
        
        while (offset + 8068 <= totalSize) {
            file.seekg(offset, ios::beg);
            file.read(hdr, 8068);
            if (file.gcount() != 8068) break;
            
            for (int k = 0; k < 8068; k++) {
                hdr[k] ^= ENCODE_KEY[(offset + k) % 16];
            }
            
            DWORD length = 0;
            DWORD base = 8000;
            for (int j = 0; j < 8; j++) {
                DWORD l = *(DWORD*)(&hdr[base + j * 8]);
                if (l != 0xCDCDCDCD) {
                    length = l;
                    break;
                }
            }
            
            DWORD start = offset + 8068;
            DWORD end = start + length;
            if (end > totalSize) break;
            
            int sndId = GetPerfectSoundId(strm_files[i], trackIdx);
            nativeStations[i].tracks.push_back({ start, length, sndId });
            
            offset = end;
            trackIdx++;
        }
        file.close();
    }
}

void MusicPlayerSA::ChooseTracksForNativeStation(int stationIdx) {
    if (stationIdx < 0 || stationIdx >= 11) return;
    NativeStation& st = nativeStations[stationIdx];

    auto RandomNumInRange = [&](int minVal, int maxVal) -> int {
        if (maxVal <= minVal) return minVal;
        return minVal + (rand() % (maxVal - minVal + 1));
    };

    auto findTrackBySoundId = [&](int pakIdx, int sndId) -> int {
        if (pakIdx < 0 || pakIdx >= 12) return -1;
        for (int i = 0; i < (int)nativeStations[pakIdx].tracks.size(); i++) {
            if (nativeStations[pakIdx].tracks[i].soundId == sndId) return i;
        }
        return -1;
    };

    auto isRecent = [&](const vector<int>& hist, int id) -> bool {
        for (int h : hist) if (h == id) return true;
        return false;
    };
    auto addHist = [&](vector<int>& hist, int id, int maxLen) {
        hist.push_back(id);
        if ((int)hist.size() > maxLen) hist.erase(hist.begin());
    };

    auto queueUp = [&](int sndId, int tType, string dTitle, int pakIdx) {
        st.trackQueue.push_back(sndId);
        st.trackTypeQueue.push_back(tType);
        st.displayTitleQueue.push_back(dTitle);
        st.pakIdxQueue.push_back(pakIdx);
    };

    const auto& sData = stationTable[stationIdx];

    auto queueIdent = [&]() -> bool {
        if (sData.identMin != 1922 && sData.identMax >= sData.identMin) {
            int identId = RandomNumInRange(sData.identMin, sData.identMax);
            if (findTrackBySoundId(stationIdx, identId) != -1) {
                char buf[64];
                sprintf(buf, "Station ID (%d)", identId);
                queueUp(identId, 0, buf, stationIdx);
                addHist(st.identHistory, identId, 8);
                return true;
            }
        }
        return false;
    };

    auto queueAdvert = [&]() -> bool {
        int adId = RandomNumInRange(66, 134);
        if (findTrackBySoundId(11, adId) != -1) {
            char buf[64];
            sprintf(buf, "Commercial (%d)", adId);
            queueUp(adId, 1, buf, 11);
            addHist(st.advertHistory, adId, 25);
            return true;
        }
        return false;
    };

    auto queueBanter = [&]() -> bool {
        if (sData.banterMin != 1922 && sData.banterMax >= sData.banterMin) {
            int bantId = RandomNumInRange(sData.banterMin, sData.banterMax);
            if (findTrackBySoundId(stationIdx, bantId) != -1) {
                char buf[64];
                sprintf(buf, "DJ Banter (%d)", bantId);
                queueUp(bantId, 2, buf, stationIdx);
                addHist(st.banterHistory, bantId, 15);
                return true;
            }
        }
        return false;
    };

    auto chooseSongIndex = [&]() -> int {
        int numTracks = (int)sData.tracks.size();
        if (numTracks > 0) {
            if ((int)st.musicHistory.size() >= numTracks) {
                st.musicHistory.clear();
            }
            int songIdx = RandomNumInRange(0, numTracks - 1);
            int attempts = 0;
            while (attempts < 50 && isRecent(st.musicHistory, songIdx)) {
                songIdx = RandomNumInRange(0, numTracks - 1);
                attempts++;
            }
            if (attempts >= 50) {
                for (int i = 0; i < numTracks; i++) {
                    if (!isRecent(st.musicHistory, i)) {
                        songIdx = i;
                        break;
                    }
                }
            }
            st.musicHistory.push_back(songIdx);
            return songIdx;
        }
        return -1;
    };

    auto queueTrackWithOutro = [&](int songIdx) {
        const auto& tData = sData.tracks[songIdx];
        st.currentSongIndex = songIdx;
        char buf[64];
        sprintf(buf, "Song %d (%d)", songIdx + 1, tData.soundId);
        queueUp(tData.soundId, 4, buf, stationIdx);

        if (tData.outroMin != 1922 && tData.outroMax >= tData.outroMin) {
            int outroId = RandomNumInRange(tData.outroMin, tData.outroMax);
            if (findTrackBySoundId(stationIdx, outroId) != -1) {
                char outroBuf[64];
                sprintf(outroBuf, "Song %d Outro (%d)", songIdx + 1, outroId);
                queueUp(outroId, 5, outroBuf, stationIdx);
            }
        }
    };

    auto queueIntroTrackOutro = [&](int songIdx) {
        const auto& tData = sData.tracks[songIdx];
        st.currentSongIndex = songIdx;
        char buf[128];
        const char* realTitle = GetRealTrackTitle(tData.soundId);
        if (realTitle) {
            sprintf(buf, "%s", realTitle);
        } else {
            sprintf(buf, "Song %d (%d)", songIdx + 1, tData.soundId);
        }

        if (tData.introMin != 1922 && tData.introMax >= tData.introMin) {
            int introId = RandomNumInRange(tData.introMin, tData.introMax);
            if (findTrackBySoundId(stationIdx, introId) != -1) {
                char introBuf[64];
                sprintf(introBuf, "Song %d Intro (%d)", songIdx + 1, introId);
                queueUp(introId, 3, introBuf, stationIdx);
            }
        }

        queueUp(tData.soundId, 4, buf, stationIdx);

        if (tData.outroMin != 1922 && tData.outroMax >= tData.outroMin) {
            int outroId = RandomNumInRange(tData.outroMin, tData.outroMax);
            if (findTrackBySoundId(stationIdx, outroId) != -1) {
                char outroBuf[64];
                sprintf(outroBuf, "Song %d Outro (%d)", songIdx + 1, outroId);
                queueUp(outroId, 5, outroBuf, stationIdx);
            }
        }
    };

    // State Machine based on previous track type (0=Indent, 1=Advert, 2=Banter, 3=Intro, 4=Track, 5=Outro)
    if (st.currentTrackType == 1) { // Advert -> Ident / Intro / DJ Banter
        int r = RandomNumInRange(0, 2);
        if (r == 0 && queueIdent()) return;
        if (r == 1) {
            int songIdx = chooseSongIndex();
            if (songIdx != -1) { queueIntroTrackOutro(songIdx); return; }
        }
        if (queueBanter()) return;
        if (queueIdent()) return;
    } else if (st.currentTrackType == 0) { // Ident -> Advert / Intro / DJ Banter
        int r = RandomNumInRange(0, 2);
        if (r == 0 && queueAdvert()) return;
        if (r == 1) {
            int songIdx = chooseSongIndex();
            if (songIdx != -1) { queueIntroTrackOutro(songIdx); return; }
        }
        if (queueBanter()) return;
        if (queueAdvert()) return;
    } else if (st.currentTrackType == 2) { // DJ Banter -> Intro / Advert / Ident
        int r = RandomNumInRange(0, 2);
        if (r == 0) {
            int songIdx = chooseSongIndex();
            if (songIdx != -1) { queueIntroTrackOutro(songIdx); return; }
        }
        if (r == 1 && queueAdvert()) return;
        if (queueIdent()) return;
        if (queueAdvert()) return;
    } else if (st.currentTrackType == 3) { // Intro -> Song Track
        int songIdx = chooseSongIndex();
        if (songIdx != -1) { queueTrackWithOutro(songIdx); return; }
    } else if (st.currentTrackType == 4) { // Song Track -> Outro
        int songIdx = st.currentSongIndex;
        if (songIdx >= 0 && songIdx < (int)sData.tracks.size()) {
            const auto& tData = sData.tracks[songIdx];
            if (tData.outroMin != 1922 && tData.outroMax >= tData.outroMin) {
                int outroId = RandomNumInRange(tData.outroMin, tData.outroMax);
                if (findTrackBySoundId(stationIdx, outroId) != -1) {
                    char outroBuf[64];
                    sprintf(outroBuf, "Song %d Outro (%d)", songIdx + 1, outroId);
                    queueUp(outroId, 5, outroBuf, stationIdx);
                    return;
                }
            }
        }
        if (queueIdent()) return;
        if (queueAdvert()) return;
    } else if (st.currentTrackType == 5) { // Outro -> Ident / Advert
        int r = RandomNumInRange(0, 1);
        if (r == 0 && queueIdent()) return;
        if (queueAdvert()) return;
        if (queueIdent()) return;
    } else {
        // Initial random start: can be advert, ident, banter, or song
        int r = RandomNumInRange(0, 3);
        if (r == 0 && queueAdvert()) return;
        if (r == 1 && queueIdent()) return;
        if (r == 2 && queueBanter()) return;
        int songIdx = chooseSongIndex();
        if (songIdx != -1) { queueIntroTrackOutro(songIdx); return; }
    }

    // Ultimate fallback if anything fails
    int songIdx = chooseSongIndex();
    if (songIdx != -1) {
        queueTrackWithOutro(songIdx);
        return;
    }
    if (!st.tracks.empty()) {
        int randomFallback = RandomNumInRange(0, st.tracks.size() - 1);
        char buf[128];
        const char* realTitle = GetRealTrackTitle(st.tracks[randomFallback].soundId);
        if (realTitle) {
            sprintf(buf, "%s", realTitle);
        } else {
            sprintf(buf, "Track %d (%d)", randomFallback + 1, st.tracks[randomFallback].soundId);
        }
        queueUp(st.tracks[randomFallback].soundId, 4, buf, stationIdx);
    }
}

const char* MusicPlayerSA::GetActiveTrackTitle() {
    if (currentStation >= stationCount && currentStation < stationCount + GAME_STATION_COUNT) {
        int stationIdx = currentStation - stationCount;
        return nativeStations[stationIdx].currentDisplayTitle.c_str();
    }
    return "";
}

void MusicPlayerSA::Initialise() {
    if (!BASS_Init(-1, 44100, 0, (HWND)RsGlobal.ps->window, NULL)) {
        // Log error
    }

    MusicPlayerSaTrackNames::Read();
    WalkmanState::ReadConfig();
    LoadPlaylists();
    ScanNativeStations();
    currentStation = stationCount + GAME_STATION_COUNT;
    InputHandler::ResetKeyState();
    DumpRadioTables();
}

void MusicPlayerSA::Update() {
    WalkmanState::ProcessInput();

    bool walkmanActive = (currentStation < stationCount);
    bool nativeActive = (currentStation >= stationCount && currentStation < stationCount + GAME_STATION_COUNT);
    
    if (walkmanActive) {
        if (!walkmanStream) {
            Mp3File* file = GetMp3Track(mp3Stations[currentStation].currentTrack);
            if (file) {
                walkmanStream = BASS_StreamCreateFile(FALSE, file->filename, 0, 0, BASS_STREAM_PRESCAN);
                if (walkmanStream) {
                    unsigned int savedPos = mp3Stations[currentStation].lastPositionMs;

                    if (mp3Stations[currentStation].lastSwitchTimeMs > 0) {
                        unsigned int elapsedMs = GetTickCount() - mp3Stations[currentStation].lastSwitchTimeMs;
                        savedPos += elapsedMs;
                    }

                    double totalSec = BASS_ChannelBytes2Seconds(walkmanStream, BASS_ChannelGetLength(walkmanStream, BASS_POS_BYTE));
                    unsigned int trackLenMs = (unsigned int)(totalSec * 1000.0);

                    if (trackLenMs > 0 && savedPos > 0) {
                        savedPos = savedPos % trackLenMs; 
                    }

                    if (savedPos > 0) {
                        QWORD seekByte = BASS_ChannelSeconds2Bytes(walkmanStream, savedPos / 1000.0);
                        BASS_ChannelSetPosition(walkmanStream, seekByte, BASS_POS_BYTE);
                    }
                    mp3Stations[currentStation].lastPositionMs = 0;
                    mp3Stations[currentStation].lastSwitchTimeMs = 0;

                    BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);
                    BASS_ChannelPlay(walkmanStream, FALSE);
                }
            }
        } else {
            if (CTimer::m_UserPause) {
                BASS_ChannelPause(walkmanStream);
            } else {
                if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_PAUSED) {
                    BASS_ChannelPlay(walkmanStream, FALSE);
                }
            }

            BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);

            if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_STOPPED) {
                skipping = 1;
                NextTrack();
            }
        }
    } else if (nativeActive) {

        if (AudioEngine.IsRadioOn())
        {
            AudioEngine.StopRadio(nullptr, true);
        }
        int stationIdx = currentStation - stationCount;
        NativeStation& st = nativeStations[stationIdx];
        if (!walkmanStream) {
            if (currentNativePakIdx == -1 || currentNativeTrackIdx == -1) {
                if (st.currentPakIdx != -1 && st.currentTrackIndex != -1) {
                    currentNativePakIdx = st.currentPakIdx;
                    currentNativeTrackIdx = st.currentTrackIndex;
                } else {
                    if (st.trackQueue.empty()) {
                        ChooseTracksForNativeStation(stationIdx);
                    }
                    if (!st.trackQueue.empty()) {
                        int sndId = st.trackQueue[0];
                        int tType = st.trackTypeQueue[0];
                        string dTitle = st.displayTitleQueue[0];
                        int pakIdx = st.pakIdxQueue[0];
                        
                        st.trackQueue.erase(st.trackQueue.begin());
                        st.trackTypeQueue.erase(st.trackTypeQueue.begin());
                        st.displayTitleQueue.erase(st.displayTitleQueue.begin());
                        st.pakIdxQueue.erase(st.pakIdxQueue.begin());
                        
                        int tIdx = -1;
                        for (int i = 0; i < (int)nativeStations[pakIdx].tracks.size(); i++) {
                            if (nativeStations[pakIdx].tracks[i].soundId == sndId) {
                                tIdx = i;
                                break;
                            }
                        }
                        if (tIdx != -1) {
                            currentNativePakIdx = pakIdx;
                            currentNativeTrackIdx = tIdx;
                            st.currentPakIdx = pakIdx;
                            st.currentTrackIndex = tIdx;
                            st.currentTrackType = tType;
                            st.currentDisplayTitle = dTitle;
                        }
                    }
                }
            }
            if (currentNativePakIdx != -1 && currentNativeTrackIdx != -1) {
                NativeTrack& nt = nativeStations[currentNativePakIdx].tracks[currentNativeTrackIdx];
                
                if (currentNativeStream) {
                    if (currentNativeStream->file.is_open()) currentNativeStream->file.close();
                    delete currentNativeStream;
                    currentNativeStream = nullptr;
                }
                
                currentNativeStream = new NativeTrackStream();
                currentNativeStream->file.open(nativeStations[currentNativePakIdx].filename, ios::binary);
                currentNativeStream->startOffset = nt.startOffset;
                currentNativeStream->length = nt.length;
                currentNativeStream->currentOffset = 0;
                
                if (currentNativeStream->file.is_open()) {
                    walkmanStream = BASS_StreamCreateFileUser(STREAMFILE_NOBUFFER, 0, &nativeFileProcs, currentNativeStream);
                    if (walkmanStream) {
                        double totalSec = BASS_ChannelBytes2Seconds(walkmanStream, BASS_ChannelGetLength(walkmanStream, BASS_POS_BYTE));
                        unsigned int trackDurationMs = (unsigned int)(totalSec * 1000.0);
                        
                        if (st.lastSwitchTimeMs > 0) {
                            unsigned int elapsedMs = GetTickCount() - st.lastSwitchTimeMs;
                            st.lastSwitchTimeMs = 0; // Reset switch time
                            unsigned int targetPosMs = st.lastPositionMs + elapsedMs;
                            
                            if (targetPosMs < trackDurationMs) {
                                st.lastPositionMs = targetPosMs;
                                QWORD seekByte = BASS_ChannelSeconds2Bytes(walkmanStream, targetPosMs / 1000.0);
                                BASS_ChannelSetPosition(walkmanStream, seekByte, BASS_POS_BYTE);
                                BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);
                                BASS_ChannelPlay(walkmanStream, FALSE);
                            } else {
                                unsigned int remainingElapsedMs = targetPosMs - trackDurationMs;
                                BASS_StreamFree(walkmanStream);
                                walkmanStream = 0;
                                currentNativePakIdx = -1;
                                currentNativeTrackIdx = -1;
                                st.currentPakIdx = -1;
                                st.currentTrackIndex = -1;
                                st.lastPositionMs = remainingElapsedMs;
                                st.lastSwitchTimeMs = GetTickCount(); // Keep advancing in background on next frame
                            }
                        } else {
                            if (st.lastPositionMs > 0 && st.lastPositionMs < trackDurationMs) {
                                QWORD seekByte = BASS_ChannelSeconds2Bytes(walkmanStream, st.lastPositionMs / 1000.0);
                                BASS_ChannelSetPosition(walkmanStream, seekByte, BASS_POS_BYTE);
                            }
                            st.lastPositionMs = 0;
                            BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);
                            BASS_ChannelPlay(walkmanStream, FALSE);
                        }
                    }
                }
            }
        } else {
            if (CTimer::m_UserPause) {
                BASS_ChannelPause(walkmanStream);
            } else {
                if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_PAUSED) {
                    BASS_ChannelPlay(walkmanStream, FALSE);
                }
            }

            BASS_ChannelSetAttribute(walkmanStream, BASS_ATTRIB_VOL, walkmanVolume / 127.0f);

            if (BASS_ChannelIsActive(walkmanStream) == BASS_ACTIVE_STOPPED) {
                BASS_StreamFree(walkmanStream);
                walkmanStream = 0;
                currentNativePakIdx = -1;
                currentNativeTrackIdx = -1;
                st.currentPakIdx = -1;
                st.currentTrackIndex = -1;
                st.lastPositionMs = 0;
            }
        }
    } else {
        if (walkmanStream) {
            BASS_StreamFree(walkmanStream);
            walkmanStream = 0;
        }
        if (currentNativeStream) {
            if (currentNativeStream->file.is_open()) currentNativeStream->file.close();
            delete currentNativeStream;
            currentNativeStream = nullptr;
        }
        currentNativePakIdx = -1;
        currentNativeTrackIdx = -1;
    }
}

void MusicPlayerSA::NextTrack() {
    if (walkmanStream) {
        BASS_StreamFree(walkmanStream);
        walkmanStream = 0;
    }
    if (currentNativeStream) {
        if (currentNativeStream->file.is_open()) currentNativeStream->file.close();
        delete currentNativeStream;
        currentNativeStream = nullptr;
    }
    currentNativePakIdx = -1;
    currentNativeTrackIdx = -1;

    if (currentStation < stationCount) {
        if (mp3Stations[currentStation].shuffle && mp3Stations[currentStation].trackCount > 0) {
            mp3Stations[currentStation].currentTrack = rand() % mp3Stations[currentStation].trackCount;
        } else {
            mp3Stations[currentStation].currentTrack += skipping;
            if (mp3Stations[currentStation].currentTrack < 0) 
                mp3Stations[currentStation].currentTrack = mp3Stations[currentStation].trackCount - 1;
            else if (mp3Stations[currentStation].currentTrack >= mp3Stations[currentStation].trackCount)
                mp3Stations[currentStation].currentTrack = 0;
        }
    } else if (currentStation >= stationCount && currentStation < stationCount + GAME_STATION_COUNT) {
        int stationIdx = currentStation - stationCount;
        NativeStation& st = nativeStations[stationIdx];
        st.currentPakIdx = -1;
        st.currentTrackIndex = -1;
        st.lastPositionMs = 0;
        st.lastSwitchTimeMs = 0;
    }
}

void MusicPlayerSA::StopStreamAndSavePosition() {
    if (walkmanStream) {
        QWORD pos = BASS_ChannelGetPosition(walkmanStream, BASS_POS_BYTE);
        double posSec = BASS_ChannelBytes2Seconds(walkmanStream, pos);
        if (currentStation < stationCount) {
            mp3Stations[currentStation].lastPositionMs = (unsigned int)(posSec * 1000.0);
            mp3Stations[currentStation].lastSwitchTimeMs = GetTickCount();
        } else if (currentStation >= stationCount && currentStation < stationCount + GAME_STATION_COUNT) {
            int stationIdx = currentStation - stationCount;
            nativeStations[stationIdx].lastPositionMs = (unsigned int)(posSec * 1000.0);
            nativeStations[stationIdx].lastSwitchTimeMs = GetTickCount();
        }
        BASS_StreamFree(walkmanStream);
        walkmanStream = 0;
    }
    if (currentNativeStream) {
        if (currentNativeStream->file.is_open()) currentNativeStream->file.close();
        delete currentNativeStream;
        currentNativeStream = nullptr;
    }
    currentNativePakIdx = -1;
    currentNativeTrackIdx = -1;
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

void MusicPlayerSA::DumpRadioTables() {
    ofstream out("radio_sound_tables_dump.txt");
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
            out << setw(20) << left << stationNames[i] << " [Min: " << setw(5) << table[i].minId << ", Max: " << setw(5) << table[i].maxId << "] Exact IDs: ";
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
            out << "  Track " << setw(2) << t << ": TrackSoundID = " << setw(6) << tracks[idx]
                << " | Intro [Min = " << setw(5) << intros[idx].minId << ", Max = " << setw(5) << intros[idx].maxId << "]"
                << " | Outro [Min = " << setw(5) << outros[idx].minId << ", Max = " << setw(5) << outros[idx].maxId << "]\n";
        }
        out << "\n";
    }

    out.close();
}
#endif