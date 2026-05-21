#pragma once
#include <plugin.h>
#include <CPad.h>

class PadHelper {
public:
static bool GetMouseWheelUpJustDown() { return !!(CPad::GetPad(0)->NewMouseControllerState.wheelUp && !CPad::GetPad(0)->OldMouseControllerState.wheelUp); }
static bool GetMouseWheelDownJustDown() { return !!(CPad::GetPad(0)->NewMouseControllerState.wheelDown && !CPad::GetPad(0)->OldMouseControllerState.wheelDown); }
static bool GetMouseWheelUpJustUp() { return !!(!CPad::GetPad(0)->NewMouseControllerState.wheelUp && CPad::GetPad(0)->OldMouseControllerState.wheelUp); }
static bool GetMouseWheelDownJustUp() { return !!(!CPad::GetPad(0)->NewMouseControllerState.wheelDown && CPad::GetPad(0)->OldMouseControllerState.wheelDown); }
static bool GetMouseWheelUp() { return CPad::GetPad(0)->NewMouseControllerState.wheelUp; }
static bool GetMouseWheelDown() { return CPad::GetPad(0)->NewMouseControllerState.wheelDown; }
static bool GetMouseWheelUpUp() { return !CPad::GetPad(0)->OldMouseControllerState.wheelUp; }
static bool GetMouseWheelDownUp() { return !CPad::GetPad(0)->OldMouseControllerState.wheelDown; }
static bool GetChar(int32_t c) { return CPad::GetPad(0)->NewKeyState.standardKeys[c]; }
static bool GetCharJustDown(int32_t c) { return !!(CPad::GetPad(0)->NewKeyState.standardKeys[c] && !CPad::GetPad(0)->OldKeyState.standardKeys[c]); }
};