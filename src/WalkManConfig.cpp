#include "../includes/WalkManConfig.h"
#include "../includes/WalkManState.h"
#include "../includes/ConfigHelper.h"
#include <extensions/Paths.h>

WalkManConfig config;

WalkManConfig::WalkManConfig() { SetDefaults(); }

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

  std::string path = GAME_PATH("\\scripts\\walkman.ini");
  ConfigHelper cfg(path);
  if (!cfg.Exists()) {
    Write();
    return;
  }
  
  // Keybinds
  NextTrack = cfg.ReadInt("KEYBINDS", "NEXT_TRACK", NextTrack);
  PrevTrack = cfg.ReadInt("KEYBINDS", "PREVIOUS_TRACK", PrevTrack);
  ToggleShuffle = cfg.ReadInt("KEYBINDS", "TOGGLE_SHUFFLE", ToggleShuffle);
  VolumeUp = cfg.ReadInt("KEYBINDS", "VOLUME_UP", VolumeUp);
  VolumeDown = cfg.ReadInt("KEYBINDS", "VOLUME_DOWN", VolumeDown);
  ToggleList = cfg.ReadInt("KEYBINDS", "BROWSE_LIST", ToggleList);
  ListScrollUp = cfg.ReadInt("KEYBINDS", "LIST_SCROLL_UP", ListScrollUp);
  ListScrollDown = cfg.ReadInt("KEYBINDS", "LIST_SCROLL_DOWN", ListScrollDown);
  ListChoose = cfg.ReadInt("KEYBINDS", "LIST_CHOOSE", ListChoose);
  ReloadKey = cfg.ReadInt("KEYBINDS", "RELOAD_CONFIG_KEY", ReloadKey);
  ControlDisableToggle = cfg.ReadInt("KEYBINDS", "CONTROL_DISABLE_TOGGLE", ControlDisableToggle);

  // UI - List
  ListStartX = cfg.ReadFloat("LIST_UI", "LIST_START_X", ListStartX);
  ListStartY = cfg.ReadFloat("LIST_UI", "LIST_START_Y", ListStartY);
  LineGap = cfg.ReadFloat("LIST_UI", "LINE_GAP", LineGap);
  TextScale = cfg.ReadVec("LIST_UI", "TEXT_SCALE", TextScale);
  ListItemHeight = cfg.ReadFloat("LIST_UI", "LIST_ITEM_HEIGHT", ListItemHeight);
  ListMaxChars = cfg.ReadInt("LIST_UI", "LIST_MAX_CHARS", ListMaxChars);

  // UI - Common
  walkmanPos = cfg.ReadRect("COMMON_UI", "WALKMAN_POS", walkmanPos);
  walkmanOffset = cfg.ReadVec("COMMON_UI", "WALKMAN_OFFSET", walkmanOffset);
  ItemPaddingX = cfg.ReadFloat("COMMON_UI", "ITEM_PADDING_X", ItemPaddingX);
  ItemPaddingY = cfg.ReadFloat("COMMON_UI", "ITEM_PADDING_Y", ItemPaddingY);
  TextScrollSpeed = cfg.ReadInt("COMMON_UI", "TEXT_SCROLL_SPEED", TextScrollSpeed);
  HelperTextOffset = cfg.ReadVec("COMMON_UI", "HELPER_TEXT_OFFSET", HelperTextOffset);

  // UI - Station
  StationStartX = cfg.ReadFloat("STATION_UI", "STATION_START_X", StationStartX);
  StationStartY = cfg.ReadFloat("STATION_UI", "STATION_START_Y", StationStartY);
  StationLineGap = cfg.ReadFloat("STATION_UI", "STATION_LINE_GAP", StationLineGap);
  StationTextScale = cfg.ReadVec("STATION_UI", "STATION_TEXT_SCALE", StationTextScale);
  StationItemHeight = cfg.ReadFloat("STATION_UI", "STATION_ITEM_HEIGHT", StationItemHeight);
  StationMaxChars = cfg.ReadInt("STATION_UI", "STATION_MAX_CHARS", StationMaxChars);
  FadeOutMp3Station = cfg.ReadInt("STATION_UI", "FADE_OUT_MP3_STATION", FadeOutMp3Station);
}

void WalkManConfig::Write() {
    std::string path = GAME_PATH("\\scripts\\walkman.ini");
    ConfigHelper cfg(path);
    
    // Keybinds
    cfg.WriteInt("KEYBINDS", "NEXT_TRACK", NextTrack);
    cfg.WriteInt("KEYBINDS", "PREVIOUS_TRACK", PrevTrack);
    cfg.WriteInt("KEYBINDS", "TOGGLE_SHUFFLE", ToggleShuffle);
    cfg.WriteInt("KEYBINDS", "VOLUME_UP", VolumeUp);
    cfg.WriteInt("KEYBINDS", "VOLUME_DOWN", VolumeDown);
    cfg.WriteInt("KEYBINDS", "BROWSE_LIST", ToggleList);
    cfg.WriteInt("KEYBINDS", "LIST_SCROLL_UP", ListScrollUp);
    cfg.WriteInt("KEYBINDS", "LIST_SCROLL_DOWN", ListScrollDown);
    cfg.WriteInt("KEYBINDS", "LIST_CHOOSE", ListChoose);
    cfg.WriteInt("KEYBINDS", "RELOAD_CONFIG_KEY", ReloadKey);
    cfg.WriteInt("KEYBINDS", "CONTROL_DISABLE_TOGGLE", ControlDisableToggle);

    // UI - List
    cfg.WriteFloat("LIST_UI", "LIST_START_X", ListStartX);
    cfg.WriteFloat("LIST_UI", "LIST_START_Y", ListStartY);
    cfg.WriteFloat("LIST_UI", "LINE_GAP", LineGap);
    cfg.WriteVec("LIST_UI", "TEXT_SCALE", TextScale);
    cfg.WriteFloat("LIST_UI", "LIST_ITEM_HEIGHT", ListItemHeight);
    cfg.WriteInt("LIST_UI", "LIST_MAX_CHARS", ListMaxChars);

    // UI - Common
    cfg.WriteRect("COMMON_UI", "WALKMAN_POS", walkmanPos);
    cfg.WriteVec("COMMON_UI", "WALKMAN_OFFSET", walkmanOffset);
    cfg.WriteFloat("COMMON_UI", "ITEM_PADDING_X", ItemPaddingX);
    cfg.WriteFloat("COMMON_UI", "ITEM_PADDING_Y", ItemPaddingY);
    cfg.WriteInt("COMMON_UI", "TEXT_SCROLL_SPEED", TextScrollSpeed);
    cfg.WriteVec("COMMON_UI", "HELPER_TEXT_OFFSET", HelperTextOffset);

    // UI - Station
    cfg.WriteFloat("STATION_UI", "STATION_START_X", StationStartX);
    cfg.WriteFloat("STATION_UI", "STATION_START_Y", StationStartY);
    cfg.WriteFloat("STATION_UI", "STATION_LINE_GAP", StationLineGap);
    cfg.WriteVec("STATION_UI", "STATION_TEXT_SCALE", StationTextScale);
    cfg.WriteFloat("STATION_UI", "STATION_ITEM_HEIGHT", StationItemHeight);
    cfg.WriteInt("STATION_UI", "STATION_MAX_CHARS", StationMaxChars);
    cfg.WriteInt("STATION_UI", "FADE_OUT_MP3_STATION", FadeOutMp3Station);
    
    cfg.Format("; WalkMan Configuration\n; Customize your settings here\n");
}


