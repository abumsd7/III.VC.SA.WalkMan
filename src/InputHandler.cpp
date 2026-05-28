#include "../includes/InputHandler.h"

//bool InputHandler::oldKeyState[256];

bool InputHandler::IsKeyJustPressed(unsigned int key) {
    bool current = plugin::KeyPressed(key);
    bool pressed = current && !oldKeyState[key];
    return pressed;
}

void InputHandler::UpdateOldKeyState() {
    for (int i = 0; i < 256; i++) oldKeyState[i] = plugin::KeyPressed(i);
}


void InputHandler::ResetKeyState() {
    for (int i = 0; i < 256; i++) oldKeyState[i] = false;
}