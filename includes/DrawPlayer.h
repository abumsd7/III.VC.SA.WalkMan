#pragma once

#include <plugin.h>
#include <CSprite2d.h>
#include "WalkManConfig.h"

class DrawPlayer {
public:
    static RwTexDictionary* walkmanTxd;
    static CSprite2d walkmanSprite;
    
    static int listStartIndex;
    static unsigned int scrollTimer;
    static float slideOffset;

    static void Initialise();
    static void Draw();
    static void RenderList();
    static void DisplayMp3Station();
    static void TextWithBGRect(float x, float y, const char* str, CRGBA rectCol, CRGBA textCol, float paddingX = 2.0f, float paddingY = 2.0f, float itemHeight = 14.0f, int maxChars = 0, bool rightAlign = false, float extraPaddingLeft = 0.0f, float heightOverride = -1.0f, float extraPaddingRight = 0.0f);
    static void Shutdown();
};
