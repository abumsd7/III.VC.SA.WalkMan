#include "../includes/DrawPlayer.h"
#include "../includes/SomeMacros.h"
#include "../includes/WalkmanState.h"
#include <CFileLoader.h>
#include <CFont.h>
#include <CHud.h>
#include <cmath>
#include <extensions/Screen.h>
#include <stdio.h>
#include <string>

RwTexDictionary *DrawPlayer::walkmanTxd = nullptr;
CSprite2d DrawPlayer::walkmanSprite;
int DrawPlayer::listStartIndex = 0;
unsigned int DrawPlayer::scrollTimer = 0;
float DrawPlayer::slideOffset = -400.0f;

void DrawPlayer::Initialise() {
#ifdef GTA3
  walkmanTxd =
      CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_iii.txd"));
  walkmanSprite.m_pTexture =
      RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_2001");
#elif defined GTAVC
  walkmanTxd =
      CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_vc.txd"));
  walkmanSprite.m_pTexture =
      RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_1986");
#elif defined GTASA
  walkmanTxd =
      CFileLoader::LoadTexDictionary(GAME_PATH("models\\walkman_sa.txd"));
  walkmanSprite.m_pTexture =
      RwTexDictionaryFindNamedTexture(walkmanTxd, "wm_1992");
#endif
}

void DrawPlayer::Draw() {
  scrollTimer++;

  // Slide animation
  bool radioOff = WalkmanState::IsRadioOff();
  bool shouldBeVisible = !radioOff || WalkmanState::listActive;
  float slideTarget =
      shouldBeVisible ? 0.0f : -(config.walkmanPos.right + 50.0f);
  slideOffset += (slideTarget - slideOffset) * 0.08f;
  if (fabs(slideTarget - slideOffset) < 0.5f)
    slideOffset = slideTarget;

  // Fully off screen, skip all rendering
  if (slideOffset <= -(config.walkmanPos.right + 40.0f))
    return;

  float screenSlideX = SCREEN_SCALE_X(slideOffset);

  if (WalkmanState::listActive) {
    if (WalkmanState::listAlpha > 50.0f) {
      CFont::SetAlphaFade(WalkmanState::listAlpha);
      RenderList();
      WalkmanState::listAlpha += WalkmanState::listFade;
    } else {
      WalkmanState::SetListActive(false);
    }
  }

  CFont::SetAlphaFade(255.0f);

  if (config.FadeOutMp3Station && !radioOff && !WalkmanState::listActive) {
    if (WalkmanState::fade == 0)
      return;
    WalkmanState::fade -= 1;
    CFont::SetAlphaFade(WalkmanState::fade * 5.0f);
  } else {
    if (config.FadeOutMp3Station && WalkmanState::listActive) {
      WalkmanState::fade = 51; // reset fade when list opens
    }
    CFont::SetAlphaFade(255.0f);
  }

  DisplayMp3Station();
  CFont::SetAlphaFade(255.0f);
}

void DrawPlayer::RenderList() {
  if (WalkmanState::currentStation >= WalkmanState::stationCount)
    return;
#ifndef GTASA
  CFont::SetJustifyOff();
  CFont::SetBackgroundOff();
  CFont::SetCentreOff();
  CFont::SetPropOn();
#else
  CFont::SetJustify(false);
  CFont::SetBackground(false, false);
  CFont::SetOrientation(ALIGN_LEFT);
  CFont::SetProportional(true);
#endif
  CFont::SetBackgroundColor(CRGBA(0, 0, 0, 187));
  CFont::SetDropShadowPosition(1);
  CFont::SetDropColor(CRGBA(0, 0, 0, 255));
#ifdef GTAVC
  CFont::SetFontStyle(FONT_STANDARD);
#elif defined(GTA3)
  CFont::SetFontStyle(FONT_BANK);
#else
  CFont::SetFontStyle(FONT_SUBTITLES);
  CFont::SetWrapx(999999.0f);
#endif
  CFont::SetScale(SCREEN_SCALE_X(config.TextScale.x),
                  SCREEN_SCALE_Y(config.TextScale.y));
  if (WalkmanState::currentStation >= WalkmanState::stationCount) {
    return;
  }

  // Sliding window logic
  if (WalkmanState::listCurrentItem < listStartIndex) {
    listStartIndex = WalkmanState::listCurrentItem;
  } else if (WalkmanState::listCurrentItem >= listStartIndex + 5) {
    listStartIndex = WalkmanState::listCurrentItem - 4;
  }

  float posY = config.ListStartY; // Start printing list from bottom

  // Count how many items to print
  int numItems = 0;
  unsigned int trackCount = WalkmanState::GetTrackCount();
  for (int i = 0; i < 5; i++) {
    if (listStartIndex + i >= (int)trackCount)
      break;
    numItems++;
  }

  float screenSlideX = SCREEN_SCALE_X(slideOffset);

  // Print list items backwards
  for (int i = numItems - 1; i >= 0; i--) {
    int trackIndex = listStartIndex + i;
    Mp3File *temp = WalkmanState::GetMp3Track(trackIndex);
    if (!temp)
      continue;

    CRGBA textCol = (trackIndex == WalkmanState::listCurrentItem)
                        ? CRGBA(247, 194, 97, 255)
                        : CRGBA(255, 255, 255, 255);
    CRGBA rectCol = CRGBA(0, 0, 0, 187);

    char buf[512];
    if (temp->artist == nullptr) {
      snprintf(buf, sizeof(buf), "%s", temp->title);
    } else {
      snprintf(buf, sizeof(buf), "%s - %s", temp->artist, temp->title);
    }

    TextWithBGRect(SCREEN_SCALE_X(config.ListStartX) + screenSlideX,
                   SCREEN_SCALE_FROM_BOTTOM(posY), buf, rectCol, textCol,
                   config.ItemPaddingX, config.ItemPaddingY,
                   config.ListItemHeight, config.ListMaxChars);
    posY += config.LineGap;
  }

  // Print track count at the top
  char countBuf[64];
  snprintf(countBuf, sizeof(countBuf), "Track %i/%i", WalkmanState::listCurrentItem + 1,
          trackCount);

  CFont::SetScale(SCREEN_SCALE_X(config.TextScale.x),
                  SCREEN_SCALE_Y(config.TextScale.y));
  TextWithBGRect(SCREEN_SCALE_X(config.ListStartX) + screenSlideX,
                 SCREEN_SCALE_FROM_BOTTOM(posY), countBuf, CRGBA(0, 0, 0, 187),
                 CRGBA(255, 255, 255, 255), config.ItemPaddingX,
                 config.ItemPaddingY, config.ListItemHeight,
                 config.ListMaxChars);
}

void DrawPlayer::DisplayMp3Station() {
  float currentAlpha = (config.FadeOutMp3Station && !WalkmanState::IsRadioOff())
                           ? (WalkmanState::fade * 5.0f)
                           : 255.0f;
  if (currentAlpha > 255.0f)
    currentAlpha = 255.0f;
  unsigned char alphaByte = (unsigned char)currentAlpha;
  unsigned char rectAlphaByte = (unsigned char)(187 * (currentAlpha / 255.0f));

  float screenSlideX = SCREEN_SCALE_X(slideOffset);
  float oxf = SCREEN_SCALE_X(config.walkmanOffset.x);
  float oyf = SCREEN_SCALE_Y(config.walkmanOffset.y);
  CRect walkmanBounds(
      SCREEN_SCALE_X(config.walkmanPos.left) + oxf + screenSlideX,
      SCREEN_SCALE_FROM_BOTTOM(config.walkmanPos.top) - oyf,
      SCREEN_SCALE_X(config.walkmanPos.right) + oxf + screenSlideX,
      SCREEN_SCALE_FROM_BOTTOM(config.walkmanPos.bottom) - oyf);
  int vertexAlphaState;
  void *raster;
  RwRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &vertexAlphaState);
  RwRenderStateGet(rwRENDERSTATETEXTURERASTER, &raster);
  RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
  RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
  walkmanSprite.Draw(walkmanBounds, CRGBA(255, 255, 255, alphaByte));

#ifndef GTASA
  CFont::SetJustifyOff();
  CFont::SetBackgroundOff();
  CFont::SetCentreOff();
  CFont::SetPropOn();
#else
  CFont::SetJustify(false);
  CFont::SetBackground(false, false);
  CFont::SetOrientation(ALIGN_LEFT);
  CFont::SetProportional(true);
#endif
  CFont::SetBackgroundColor(CRGBA(0, 0, 0, 187));
  CFont::SetDropShadowPosition(1);
  CFont::SetDropColor(CRGBA(0, 0, 0, 255));
#ifdef GTAVC
  CFont::SetFontStyle(FONT_STANDARD);
#elif defined(GTA3)
  CFont::SetFontStyle(FONT_BANK);
#else
  CFont::SetFontStyle(FONT_SUBTITLES);
  CFont::SetWrapx(999999.0f);
#endif
  CFont::SetScale(SCREEN_SCALE_X(config.StationTextScale.x),
                  SCREEN_SCALE_Y(config.StationTextScale.y));

  float stationX = SCREEN_SCALE_X(config.StationStartX) + screenSlideX;
  float posY = config.StationStartY;
  CRGBA textCol = CRGBA(247, 194, 97, alphaByte);
  CRGBA rectCol = CRGBA(0, 0, 0, rectAlphaByte);

  bool radioOff = WalkmanState::IsRadioOff();

  if (radioOff) {
    // Radio OFF: just show station name
    TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Radio OFF",
                   rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                   config.StationItemHeight, config.StationMaxChars);
  } else if (WalkmanState::currentStation < WalkmanState::stationCount) {
    // MP3 Logic
    Mp3File *file =
        WalkmanState::mp3Stations[WalkmanState::currentStation].mp3Start;
    int i = 0;
    unsigned int curTrack = WalkmanState::GetCurrentTrackIndex();
    while (i < (int)curTrack && file) {
      file = (Mp3File *)(file->nextFile);
      i++;
    }

    if (!file)
      return;

    // 1. Duration (Bottom)
    unsigned int total_ms = 0, current_ms = 0;
    WalkmanState::GetTrackPlaybackInfo(current_ms, total_ms);

    if (total_ms > 0) {
      int totalMin = (total_ms / 1000) / 60;
      int totalSec = (total_ms / 1000) % 60;
      int currentMin = (current_ms / 1000) / 60;
      int currentSec = (current_ms / 1000) % 60;

      char durationBuf[64];
      snprintf(durationBuf, sizeof(durationBuf), "%02d:%02d / %02d:%02d", currentMin, currentSec,
              totalMin, totalSec);
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), durationBuf,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
      posY += config.StationLineGap;
    }

    // 2. Title
    if (file->title && file->title[0] != '\0') {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->title,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
    } else {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY),
                     "Info unavailable", rectCol, textCol, config.ItemPaddingX,
                     config.ItemPaddingY, config.StationItemHeight,
                     config.StationMaxChars);
    }
    posY += config.StationLineGap;

    // 2. Album
    if (file->album && file->album[0] != '\0') {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->album,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
      posY += config.StationLineGap;
    }

    // 3. Artist
    if (file->artist && file->artist[0] != '\0') {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), file->artist,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
    } else {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY),
                     "Info unavailable", rectCol, textCol, config.ItemPaddingX,
                     config.ItemPaddingY, config.StationItemHeight,
                     config.StationMaxChars);
    }
    posY += config.StationLineGap;

    // 4. Station Name (Top)
    const char *stationName =
        WalkmanState::mp3Stations[WalkmanState::currentStation].name;
    TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), stationName,
                   rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                   config.StationItemHeight, config.StationMaxChars);
  } else {
    // Native Radio Logic
    unsigned int total_ms = 0, current_ms = 0;
    WalkmanState::GetTrackPlaybackInfo(current_ms, total_ms);

    if (total_ms > 0) {
      int totalMin = (total_ms / 1000) / 60;
      int totalSec = (total_ms / 1000) % 60;
      int currentMin = (current_ms / 1000) / 60;
      int currentSec = (current_ms / 1000) % 60;

      char durationBuf[64];
      snprintf(durationBuf, sizeof(durationBuf), "%02d:%02d / %02d:%02d", currentMin, currentSec,
              totalMin, totalSec);
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), durationBuf,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
    } else {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY),
                     "Info unavailable", rectCol, textCol, config.ItemPaddingX,
                     config.ItemPaddingY, config.StationItemHeight,
                     config.StationMaxChars);
    }
    posY += config.StationLineGap;

    // Native Track Title (Middle)
    const char *trackTitle = WalkmanState::GetActiveTrackTitle();
    if (trackTitle && trackTitle[0] != '\0') {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), trackTitle,
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
    } else {
      TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), "Connecting...",
                     rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                     config.StationItemHeight, config.StationMaxChars);
    }
    posY += config.StationLineGap;

    // Native Station Name (Top)
    const char *stationName = WalkmanState::GetActiveStationName();
    TextWithBGRect(stationX, SCREEN_SCALE_FROM_BOTTOM(posY), stationName,
                   rectCol, textCol, config.ItemPaddingX, config.ItemPaddingY,
                   config.StationItemHeight, config.StationMaxChars);
  }

  // Helper text below walkman
  CRGBA helperTextCol = CRGBA(200, 200, 200, alphaByte);
  CRGBA helperRectCol = CRGBA(0, 0, 0, rectAlphaByte);
  const char *helperStr = nullptr;

  if (radioOff) {
    helperStr = "Press Left or Right to switch channels.";
  } else if (WalkmanState::currentStation >= WalkmanState::stationCount) {
    helperStr = "Playing in-game radio stations...";
  } else if (WalkmanState::listActive) {
    helperStr = "Press RET to play song.";
  } else {
    helperStr = "Press L to show list.";
  }

  float helperX =
      walkmanBounds.left + SCREEN_SCALE_X(config.HelperTextOffset.x);
  float helperY =
      walkmanBounds.bottom + SCREEN_SCALE_Y(config.HelperTextOffset.y);
  TextWithBGRect(helperX, helperY, helperStr, helperRectCol, helperTextCol,
                 config.ItemPaddingX, config.ItemPaddingY,
                 config.StationItemHeight, config.StationMaxChars);
}

void DrawPlayer::Shutdown() {
  if (walkmanTxd) {
    RwTexDictionaryDestroy(walkmanTxd);
    walkmanTxd = nullptr;
    walkmanSprite.m_pTexture = nullptr;
  }
}

void DrawPlayer::TextWithBGRect(float x, float y, const char *str,
                                CRGBA rectCol, CRGBA textCol, float paddingX,
                                float paddingY, float itemHeight, int maxChars,
                                bool rightAlign, float extraPaddingLeft,
                                float heightOverride, float extraPaddingRight) {
  if (!str || str[0] == '\0')
    return;

  // Sanitize: strip non-printable / non-ASCII chars
  char cleanBuf[512];
  int j = 0;
  for (int i = 0; str[i] && j < 510; i++) {
    unsigned char c = (unsigned char)str[i];
    if (c >= 32 && c != 127) {
      cleanBuf[j++] = str[i];
    }
  }
  cleanBuf[j] = '\0';
  if (j == 0)
    return;

  std::string scrollStr;
  const char *displayStr = cleanBuf;
  int len = j;

  if (maxChars > 0 && len > maxChars) {
    int scrollRange = len - maxChars;
    int speed = config.TextScrollSpeed > 0 ? config.TextScrollSpeed : 8;
    int pauseFrames = 60; // pause at each end

    // Full cycle: pause at start -> scroll right -> pause at end -> scroll left
    int totalCycle =
        pauseFrames + scrollRange * speed + pauseFrames + scrollRange * speed;
    int frame = (int)(scrollTimer % (unsigned int)totalCycle);

    int offset = 0;
    if (frame < pauseFrames) {
      offset = 0;
    } else if (frame < pauseFrames + scrollRange * speed) {
      offset = (frame - pauseFrames) / speed;
    } else if (frame < pauseFrames * 2 + scrollRange * speed) {
      offset = scrollRange;
    } else {
      offset =
          scrollRange - (frame - pauseFrames * 2 - scrollRange * speed) / speed;
    }

    if (offset < 0)
      offset = 0;
    if (offset > scrollRange)
      offset = scrollRange;

    scrollStr = std::string(cleanBuf).substr(offset, maxChars);
    displayStr = scrollStr.c_str();
  }

  float textWidth = 0.0f;
#if defined(GTASA)
  textWidth = CFont::GetStringWidth((char *)displayStr, true, false);
#else
  wchar_t wstr[512];
  AsciiToUnicode(displayStr, wstr);
  textWidth = CFont::GetStringWidth(wstr, true);
#endif

  float scaledPadX = SCREEN_SCALE_X(paddingX);
  float scaledPadY = SCREEN_SCALE_Y(paddingY);

  float rectLeft = rightAlign ? (x - textWidth - scaledPadX - extraPaddingLeft)
                              : (x - scaledPadX - extraPaddingLeft);
  float rectRight = rectLeft + textWidth + 2.0f * scaledPadX +
                    extraPaddingLeft + extraPaddingRight;
  float rectBottom = (heightOverride != -1.0f)
                         ? (y + heightOverride)
                         : (y + SCREEN_SCALE_Y(itemHeight) + scaledPadY);

  CSprite2d::DrawRect(CRect(rectLeft, y - scaledPadY, rectRight, rectBottom),
                      rectCol);

  CFont::SetColor(textCol);
#ifndef GTASA
  if (rightAlign)
    CFont::SetRightJustifyOn();
  else
    CFont::SetRightJustifyOff();
#else
  CFont::SetOrientation(rightAlign ? ALIGN_RIGHT : ALIGN_LEFT);
  CFont::SetJustify(rightAlign);
#endif
  CFont::PrintString(x, y, (char *)displayStr);
}