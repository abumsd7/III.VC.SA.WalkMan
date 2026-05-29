#include "../includes/InputHandler.h"
#include "../includes/WalkManConfig.h"
#include "../includes/ProcessHelper.h"
#include <windows.h>

typedef SHORT(WINAPI* fnGetKeyState)(int);
static fnGetKeyState pGetKeyState = nullptr;

static bool MyKeyPressed(unsigned int keyCode) {
    if (!pGetKeyState) {
        pGetKeyState = (fnGetKeyState)ProcessHelper::GetFunc("user32.dll", "GetKeyState");
    }
    if (pGetKeyState) {
        return (pGetKeyState(keyCode) & 0x8000) != 0;
    }
    return false;
}

bool InputHandler::oldKeyState[256] = { false };

__declspec(noinline) bool InputHandler::IsKeyJustPressed(unsigned int key) {
    if (key >= 256) return false;
    bool current = MyKeyPressed(key);
    bool pressed = current && !oldKeyState[key];
    if (current) {
        oldKeyState[key] = true;
    }
    return pressed;
}

void InputHandler::UpdateOldKeyState() {
    int keysToPoll[] = {
        config.ToggleList, config.ListChoose, VK_RETURN,
        config.ListScrollUp, VK_UP, config.ListScrollDown, VK_DOWN,
        VK_LEFT, VK_RIGHT, config.PrevTrack, config.NextTrack,
        config.ToggleShuffle, config.VolumeUp, config.VolumeDown,
        config.ReloadKey,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39
    };
    for (int key : keysToPoll) {
        if (key > 0 && key < 256) {
            if (!MyKeyPressed(key)) {
                oldKeyState[key] = false;
            }
        }
    }
}


