#pragma once
#include <plugin.h>

class WalkManConfig {
public:
    WalkManConfig();

    // Keybinds
    int NextTrack;
    int PrevTrack;
    int ToggleShuffle;
    int VolumeUp;
    int VolumeDown;
    int ToggleList;
    int ListScrollUp;
    int ListScrollDown;
    int ListChoose;
    int ReloadKey;
	int ControlDisableToggle;

    // UI - List
    float ListStartX;
    float ListStartY;
    float LineGap;
    CVector2D TextScale;
    float ListItemHeight;
    int ListMaxChars;

    // UI - Common
    CRect walkmanPos;
    CVector2D walkmanOffset;
    float ItemPaddingX;
    float ItemPaddingY;
    int TextScrollSpeed;
    CVector2D HelperTextOffset;

    // UI - Station
    float StationStartX;
    float StationStartY;
    float StationLineGap;
    CVector2D StationTextScale;
    float StationItemHeight;
    int StationMaxChars;
    int FadeOutMp3Station;

    void SetDefaults();
    void Read();
    void Write();
};

extern WalkManConfig config;
