#include "../includes/DrawPlayer.h"
#include "../includes/MusicPlayer.h"
#include "../includes/SomeMacros.h"
#include <CFont.h>
#include <CHud.h>
#include <extensions/Screen.h>
#include <stdio.h>
#include <CFileLoader.h>
#include <cmath>

extern const char* gameRadioNames[9];
extern unsigned char* gameDisableKeyboard2;
extern unsigned int* gameTrackCount;
extern unsigned int* gameCurrentTrack;

RwTexDictionary* DrawPlayer::walkmanTxd = nullptr;
CSprite2d DrawPlayer::walkmanSprite;
int DrawPlayer::listStartIndex = 0;
unsigned int DrawPlayer::scrollTimer = 0;
float DrawPlayer::slideOffset = -400.0f;

void DrawPlayer::Initialise()
{
#ifdef GTA3
    walkmanTxd = CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_iii.txd"));
    walkmanSprite.m_pTexture = RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_2001");
#elif defined GTAVC
    walkmanTxd = CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_vc.txd"));
    walkmanSprite.m_pTexture = RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_1986");
#elif defined GTASA
    walkmanTxd = CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_sa.txd"));
    walkmanSprite.m_pTexture = RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_1992");
#endif
}

void DrawPlayer::Draw() {
    scrollTimer++;

    // Slide animation
    bool radioOff = (MusicPlayer::currentStation == MusicPlayer::stationCount + 9);
    bool shouldBeVisible = !radioOff || MusicPlayer::listActive;
    float slideTarget = shouldBeVisible ? 0.0f : -(config.walkmanPos.right + 50.0f);
    slideOffset += (slideTarget - slideOffset) * 0.08f;
    if (fabs(slideTarget - slideOffset) < 0.5f) slideOffset = slideTarget;

    // Fully off screen, skip all rendering
    if (slideOffset <= -(config.walkmanPos.right + 40.0f)) return;

    float screenSlideX = SCREEN_SCALE_X(slideOffset);

    if (MusicPlayer::listActive) {
        if (MusicPlayer::listAlpha > 50.0f) {
            CFont::SetAlphaFade(MusicPlayer::listAlpha);
            RenderList();
            MusicPlayer::listAlpha += MusicPlayer::listFade;
        }
        else {
            MusicPlayer::listActive = false;
            *gameDisableKeyboard2 = MusicPlayer::listActive;
        }
    }

    CFont::SetAlphaFade(255.0f);

    if (config.FadeOutMp3Station && !radioOff && !MusicPlayer::listActive) {
        if (MusicPlayer::fade == 0) return;
        MusicPlayer::fade -= 1;
        CFont::SetAlphaFade(MusicPlayer::fade * 5.0f);
    }
    else {
        if (config.FadeOutMp3Station && MusicPlayer::listActive) {
            MusicPlayer::fade = 51; // reset fade when list opens
        }
        CFont::SetAlphaFade(255.0f);
    }

    DisplayMp3Station();
    CFont::SetAlphaFade(255.0f);
}

void DrawPlayer::RenderList() {
    if (MusicPlayer::currentStation >= MusicPlayer::stationCount) return;

    CFont::SetJustifyOff();
    CFont::SetBackgroundOff();
    CFont::SetBackgroundColor(CRGBA(0, 0, 0, 187));
    CFont::SetPropOn();
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
#ifdef GTAVC
    CFont::SetFontStyle(FONT_STANDARD);
#elif defined(GTA3)
    CFont::SetFontStyle(FONT_BANK);
#endif
    CFont::SetCentreOff();
    CFont::SetScale(SCREEN_SCALE_X(config.TextScale.x), SCREEN_SCALE_Y(config.TextScale.y));
    if (MusicPlayer::currentStation >= MusicPlayer::stationCount) {
        return;
    }

    // Sliding window logic
    if (MusicPlayer::listCurrentItem < listStartIndex) {
        listStartIndex = MusicPlayer::listCurrentItem;
    }
    else if (MusicPlayer::listCurrentItem >= listStartIndex + 5) {
        listStartIndex = MusicPlayer::listCurrentItem - 4;
    }

    float posY = config.ListStartY; // Start printing list from bottom

    // Count how many items to print
    int numItems = 0;
    for (int i = 0; i < 5; i++) {
        if (listStartIndex + i >= (int)*gameTrackCount) break;
        numItems++;
    }

    float screenSlideX = SCREEN_SCALE_X(slideOffset);

    // Print list items backwards
    for (int i = numItems - 1; i >= 0; i--) {
        int trackIndex = listStartIndex + i;
        Mp3File* temp = MusicPlayer::GetMp3Track(trackIndex);
        if (!temp) continue;

        CRGBA textCol = (trackIndex == MusicPlayer::listCurrentItem) ? CRGBA(247, 194, 97, 255) : CRGBA(255, 255, 255, 255);
        CRGBA rectCol = CRGBA(0, 0, 0, 187);

        char buf[512];
        if (temp->artist == nullptr) {
            sprintf(buf, "%s", temp->title);
        }
        else {
            sprintf(buf, "%s - %s", temp->artist, temp->title);
        }

        TextWithBGRect(SCREEN_SCALE_X(config.ListStartX) + screenSlideX, SCREEN_SCALE_FROM_BOTTOM(posY), buf, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.ListItemHeight, config.ListMaxChars);
        posY += config.LineGap;
    }

    // Print track count at the top
    char countBuf[64];
    sprintf(countBuf, "Track %i/%i", MusicPlayer::listCurrentItem + 1, *gameTrackCount);

    CFont::SetScale(SCREEN_SCALE_X(config.TextScale.x), SCREEN_SCALE_Y(config.TextScale.y));
    TextWithBGRect(SCREEN_SCALE_X(config.ListStartX) + screenSlideX, SCREEN_SCALE_FROM_BOTTOM(posY), countBuf, CRGBA(0, 0, 0, 187), CRGBA(255, 255, 255, 255), config.ItemPaddingX, config.ItemPaddingY, config.ListItemHeight, config.ListMaxChars);
}

void DrawPlayer::DisplayMp3Station() {
    float currentAlpha = (config.FadeOutMp3Station && MusicPlayer::currentStation < MusicPlayer::stationCount + 9) ? (MusicPlayer::fade * 5.0f) : 255.0f;
    if (currentAlpha > 255.0f) currentAlpha = 255.0f;
    unsigned char alphaByte = (unsigned char)currentAlpha;
    unsigned char rectAlphaByte = (unsigned char)(187 * (currentAlpha / 255.0f));

    float screenSlideX = SCREEN_SCALE_X(slideOffset);
    float oxf = SCREEN_SCALE_X(config.walkmanOffset.x);
    float oyf = SCREEN_SCALE_Y(config.walkmanOffset.y);
    CRect walkmanBounds(
        SCREEN_SCALE_X(config.walkmanPos.left) + oxf + screenSlideX,
        SCREEN_SCALE_FROM_BOTTOM(config.walkmanPos.top) - oyf,
        SCREEN_SCALE_X(config.walkmanPos.right) + oxf + screenSlideX,
        SCREEN_SCALE_FROM_BOTTOM(config.walkmanPos.bottom) - oyf
    );
    int vertexAlphaState;
    void* raster;
    RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlphaState);
    RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &raster);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
    walkmanSprite.Draw(walkmanBounds, CRGBA(255, 255, 255, alphaByte));

    CFont::SetJustifyOff();
    CFont::SetBackgroundOff();
    CFont::SetBackgroundColor(CRGBA(0, 0, 0, 187));
    CFont::SetPropOn();
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
#ifdef GTAVC
    CFont::SetFontStyle(FONT_STANDARD);
#elif defined(GTA3)
    CFont::SetFontStyle(FONT_BANK);
#endif
    CFont::SetCentreOff();
    CFont::SetScale(SCREEN_SCALE_X(config.StationTextScale.x), SCREEN_SCALE_Y(config.StationTextScale.y));

    float stationX = SCREEN_SCALE_X(config.StationStartX) + screenSlideX;
    float posY = config.StationStartY;
    CRGBA textCol = CRGBA(247, 194, 97, alphaByte);
    CRGBA rectCol = CRGBA(0, 0, 0, rectAlphaByte);

    bool radioOff = (MusicPlayer::currentStation == MusicPlayer::stationCount + 9);

    if (radioOff) {
        // Radio OFF: just show station name
        TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Radio OFF", rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
    }
    else if (MusicPlayer::currentStation < MusicPlayer::stationCount) {
        // MP3 Logic
        Mp3File* file = MusicPlayer::mp3Stations[MusicPlayer::currentStation].mp3Start;
        int i = 0;
        while (i < (int)*gameCurrentTrack && file) {
            file = (Mp3File*)(file->nextFile);
            i++;
        }

        if (!file) return;

        // 1. Title (Bottom)
        if (file->title && file->title[0] != '\0') {
            TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->title, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
        }
        else {
            TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Info unavailable", rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
        }
        posY += config.StationLineGap;

        // 2. Album
        if (file->album && file->album[0] != '\0') {
            TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->album, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
            posY += config.StationLineGap;
        }

        // 3. Artist
        if (file->artist && file->artist[0] != '\0') {
            TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->artist, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
        }
        else {
            TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Info unavailable", rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
        }
        posY += config.StationLineGap;

        // 4. Station Name (Top)
        const char* stationName = MusicPlayer::mp3Stations[MusicPlayer::currentStation].name;
        TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), stationName, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
    }
    else {
        // ADF Logic
        if (MusicPlayer::walkmanStream) {
            unsigned int total_ms = 0, current_ms = 0;
            MusicPlayer::streamMsPosition(MusicPlayer::walkmanStream, &total_ms, &current_ms);

            if (total_ms > 0) {
                int totalMin = (total_ms / 1000) / 60;
                int totalSec = (total_ms / 1000) % 60;
                int currentMin = (current_ms / 1000) / 60;
                int currentSec = (current_ms / 1000) % 60;

                char durationBuf[64];
                sprintf(durationBuf, "%02d:%02d / %02d:%02d", currentMin, currentSec, totalMin, totalSec);
                TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), durationBuf, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
            }
            else {
                TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Info unavailable", rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
            }
            posY += config.StationLineGap;
        }

        // ADF Station Name (Top)
        const char* stationName = gameRadioNames[MusicPlayer::currentStation - MusicPlayer::stationCount];
        TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), stationName, rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
    }

    // Helper text below walkman
    CRGBA helperTextCol = CRGBA(200, 200, 200, alphaByte);
    CRGBA helperRectCol = CRGBA(0, 0, 0, rectAlphaByte);
    const char* helperStr = nullptr;

    if (radioOff) {
        helperStr = "Press Left or Right to switch channels.";
    }
    else if (MusicPlayer::currentStation >= MusicPlayer::stationCount) {
        helperStr = "Playing in-game radio stations...";
    }
    else if (MusicPlayer::listActive) {
        helperStr = "Press RET to play song.";
    }
    else {
        helperStr = "Press L to show list.";
    }

    float helperX = walkmanBounds.left + SCREEN_SCALE_X(config.HelperTextOffset.x);
    float helperY = walkmanBounds.bottom + SCREEN_SCALE_Y(config.HelperTextOffset.y);
    TextWithBGRect(helperX, helperY, helperStr, helperRectCol, helperTextCol, config.ItemPaddingX, config.ItemPaddingY, config.StationItemHeight, config.StationMaxChars);
}

void DrawPlayer::Shutdown() {
    if (walkmanTxd) {
        RwTexDictionaryDestroy(walkmanTxd);
        walkmanTxd = nullptr;
        walkmanSprite.m_pTexture = nullptr;
    }
}

void DrawPlayer::TextWithBGRect(float x, float y, const char* str, CRGBA rectCol, CRGBA textCol, float paddingX, float paddingY, float itemHeight, int maxChars, bool rightAlign, float extraPaddingLeft, float heightOverride, float extraPaddingRight) {
    if (!str || str[0] == '\0') return;

    // Sanitize: strip non-printable / non-ASCII chars
    char cleanBuf[512];
    int j = 0;
    for (int i = 0; str[i] && j < 510; i++) {
        unsigned char c = (unsigned char)str[i];
        if (c >= 32 && c <= 126) {
            cleanBuf[j++] = str[i];
        }
    }
    cleanBuf[j] = '\0';
    if (j == 0) return;

    const char* displayStr = cleanBuf;
    char scrollBuf[512];
    int len = j;

    if (maxChars > 0 && len > maxChars) {
        int scrollRange = len - maxChars;
        int speed = config.TextScrollSpeed > 0 ? config.TextScrollSpeed : 8;
        int pauseFrames = 60; // pause at each end

        // Full cycle: pause at start -> scroll right -> pause at end -> scroll left
        int totalCycle = pauseFrames + scrollRange * speed + pauseFrames + scrollRange * speed;
        int frame = (int)(scrollTimer % (unsigned int)totalCycle);

        int offset = 0;
        if (frame < pauseFrames) {
            offset = 0;
        }
        else if (frame < pauseFrames + scrollRange * speed) {
            offset = (frame - pauseFrames) / speed;
        }
        else if (frame < pauseFrames * 2 + scrollRange * speed) {
            offset = scrollRange;
        }
        else {
            offset = scrollRange - (frame - pauseFrames * 2 - scrollRange * speed) / speed;
        }

        if (offset < 0) offset = 0;
        if (offset > scrollRange) offset = scrollRange;

        strncpy(scrollBuf, str + offset, maxChars);
        scrollBuf[maxChars] = '\0';
        displayStr = scrollBuf;
    }

    float textWidth = 0.0f;
#if defined(GTASA)
    textWidth = CFont::GetStringWidth((char*)displayStr, true, false);
#else
    wchar_t wstr[512];
    AsciiToUnicode(displayStr, wstr);
    textWidth = CFont::GetStringWidth(wstr, true);
#endif

    float scaledPadX = SCREEN_SCALE_X(paddingX);
    float scaledPadY = SCREEN_SCALE_Y(paddingY);

    float rectLeft = rightAlign ? (x - textWidth - scaledPadX - extraPaddingLeft) : (x - scaledPadX - extraPaddingLeft);
    float rectRight = rectLeft + textWidth + 2.0f * scaledPadX + extraPaddingLeft + extraPaddingRight;
    float rectBottom = (heightOverride != -1.0f) ? (y + heightOverride) : (y + SCREEN_SCALE_Y(itemHeight) + scaledPadY);

    CSprite2d::DrawRect(CRect(rectLeft, y - scaledPadY, rectRight, rectBottom), rectCol);

    CFont::SetColor(textCol);
    if (rightAlign) CFont::SetRightJustifyOn(); else CFont::SetRightJustifyOff();

    CFont::PrintString(x, y, (char*)displayStr);
}