#include "../includes/WalkManState.h"
#include "../includes/WalkManConfig.h"
#include <extensions/Config.h>
#include <extensions/Paths.h>
#include <fstream>

using namespace std;

WalkManConfig config;

WalkManConfig::WalkManConfig() {
    SetDefaults();
}

void WalkManConfig::SetDefaults() {
    NextTrack = VK_NEXT;
    PrevTrack = VK_PRIOR;
    ToggleShuffle = 'S';
    VolumeUp = VK_ADD;
    VolumeDown = VK_SUBTRACT;
    ToggleList = 'L';
    ListScrollUp = VK_UP;
    ListScrollDown = VK_DOWN;
    ListChoose = VK_RETURN;
    ReloadKey = VK_LCONTROL;
	ControlDisableToggle = 1;

    ListStartX = 25.0f;
    ListStartY = 265.0f;
    LineGap = 15.0f;
    TextScale = CVector2D(0.25f, 0.5f);
    ListItemHeight = 10.0f;
    ListMaxChars = 35;

    walkmanPos = CRect(20.0f, 300.0f, 200.0f, 120.0f);
    walkmanOffset = CVector2D(-5.0f, -50.0f);
    ItemPaddingX = 2.0f;
    ItemPaddingY = 2.0f;
    TextScrollSpeed = 8;
    HelperTextOffset = CVector2D(9.0f, -65.0f);

    StationStartX = 72.0f;
    StationStartY = 155.0f;
    StationLineGap = 15.0f;
    StationTextScale = CVector2D(0.25f, 0.5f);
    StationItemHeight = 10.0f;
    StationMaxChars = 20;
    FadeOutMp3Station = 0;
}

void WalkManConfig::Read() {
    SetDefaults();
    
    string path = GAME_PATH("\\scripts\\walkman.ini");
    ifstream f(path);
    if (!f.good()) {
        f.close();
        Write();
        return;
    }
    f.close();

    plugin::config_file ini(path);

    // Keybinds
    NextTrack = ini["NEXT_TRACK"].asInt(NextTrack);
    PrevTrack = ini["PREVIOUS_TRACK"].asInt(PrevTrack);
    ToggleShuffle = ini["TOGGLE_SHUFFLE"].asInt(ToggleShuffle);
    VolumeUp = ini["VOLUME_UP"].asInt(VolumeUp);
    VolumeDown = ini["VOLUME_DOWN"].asInt(VolumeDown);
    ToggleList = ini["BROWSE_LIST"].asInt(ToggleList);
    ListScrollUp = ini["LIST_SCROLL_UP"].asInt(ListScrollUp);
    ListScrollDown = ini["LIST_SCROLL_DOWN"].asInt(ListScrollDown);
    ListChoose = ini["LIST_CHOOSE"].asInt(ListChoose);
    ReloadKey = ini["RELOAD_CONFIG_KEY"].asInt(ReloadKey);
    ControlDisableToggle = ini["CONTROL_DISABLE_TOGGLE"].asInt(ControlDisableToggle);

    // UI - List
    ListStartX = ini["LIST_START_X"].asFloat(ListStartX);
    ListStartY = ini["LIST_START_Y"].asFloat(ListStartY);
    LineGap = ini["LINE_GAP"].asFloat(LineGap);
    TextScale = ini["TEXT_SCALE"].asVec2d(TextScale);
    ListItemHeight = ini["LIST_ITEM_HEIGHT"].asFloat(ListItemHeight);
    ListMaxChars = ini["LIST_MAX_CHARS"].asInt(ListMaxChars);

    // UI - Common
    walkmanPos = ini["WALKMAN_POS"].asRect(walkmanPos);
    walkmanOffset = ini["WALKMAN_OFFSET"].asVec2d(walkmanOffset);
    ItemPaddingX = ini["ITEM_PADDING_X"].asFloat(ItemPaddingX);
    ItemPaddingY = ini["ITEM_PADDING_Y"].asFloat(ItemPaddingY);
    TextScrollSpeed = ini["TEXT_SCROLL_SPEED"].asInt(TextScrollSpeed);
    HelperTextOffset = ini["HELPER_TEXT_OFFSET"].asVec2d(HelperTextOffset);

    // UI - Station
    StationStartX = ini["STATION_START_X"].asFloat(StationStartX);
    StationStartY = ini["STATION_START_Y"].asFloat(StationStartY);
    StationLineGap = ini["STATION_LINE_GAP"].asFloat(StationLineGap);
    StationTextScale = ini["STATION_TEXT_SCALE"].asVec2d(StationTextScale);
    StationItemHeight = ini["STATION_ITEM_HEIGHT"].asFloat(StationItemHeight);
    StationMaxChars = ini["STATION_MAX_CHARS"].asInt(StationMaxChars);
    FadeOutMp3Station = ini["FADE_OUT_MP3_STATION"].asInt(FadeOutMp3Station);
}

void WalkManConfig::Write() {
    SetDefaults();

    string path = GAME_PATH("\\scripts\\walkman.ini");
    ofstream out(path);
    if (!out.is_open()) return;

    out << "; WalkMan Plugin Configuration\n\n";

    out << "[KEYBINDS]\n";
    out << "; Virtual Key Codes (Decimal)\n";
    out << "NEXT_TRACK=" << NextTrack << "\n";
    out << "PREVIOUS_TRACK=" << PrevTrack << "\n";
    out << "TOGGLE_SHUFFLE=" << ToggleShuffle << "\n";
    out << "VOLUME_UP=" << VolumeUp << "\n";
    out << "VOLUME_DOWN=" << VolumeDown << "\n";
    out << "BROWSE_LIST=" << ToggleList << "\n";
    out << "LIST_SCROLL_UP=" << ListScrollUp << "\n";
    out << "LIST_SCROLL_DOWN=" << ListScrollDown << "\n";
    out << "LIST_CHOOSE=" << ListChoose << "\n";
    out << "RELOAD_CONFIG_KEY=" << ReloadKey << "\n\n";
	out << "CONTROL_DISABLE_TOGGLE=" << ControlDisableToggle << "\n\n";

    out << "[LIST_UI]\n";
    out << "; Track browser list layout\n";
    out << "LIST_START_X=" << ListStartX << "\n";
    out << "LIST_START_Y=" << ListStartY << "\n";
    out << "LINE_GAP=" << LineGap << "\n";
    out << "TEXT_SCALE=" << TextScale.x << " " << TextScale.y << "\n";
    out << "LIST_ITEM_HEIGHT=" << ListItemHeight << "\n";
    out << "LIST_MAX_CHARS=" << ListMaxChars << "\n\n";

    out << "[COMMON_UI]\n";
    out << "; Shared visual settings\n";
    out << "WALKMAN_POS=" << walkmanPos.left << " " << walkmanPos.top << " " << walkmanPos.right << " " << walkmanPos.bottom << "\n";
    out << "WALKMAN_OFFSET=" << walkmanOffset.x << " " << walkmanOffset.y << "\n";
    out << "ITEM_PADDING_X=" << ItemPaddingX << "\n";
    out << "ITEM_PADDING_Y=" << ItemPaddingY << "\n";
    out << "TEXT_SCROLL_SPEED=" << TextScrollSpeed << "\n";
    out << "HELPER_TEXT_OFFSET=" << HelperTextOffset.x << " " << HelperTextOffset.y << "\n\n";

    out << "[STATION_UI]\n";
    out << "; Now-playing station / track display\n";
    out << "STATION_START_X=" << StationStartX << "\n";
    out << "STATION_START_Y=" << StationStartY << "\n";
    out << "STATION_LINE_GAP=" << StationLineGap << "\n";
    out << "STATION_TEXT_SCALE=" << StationTextScale.x << " " << StationTextScale.y << "\n";
    out << "STATION_ITEM_HEIGHT=" << StationItemHeight << "\n";
    out << "STATION_MAX_CHARS=" << StationMaxChars << "\n";
    out << "FADE_OUT_MP3_STATION=" << FadeOutMp3Station << "\n";

    out.close();
}
