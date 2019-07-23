#ifndef IMGUI_IMPL_H
#define IMGUI_IMPL_H

#include "imgui.h"

namespace DevUI
{
    enum GamePadButton
    {
        GP_BUTTON_CROSS,
        GP_BUTTON_CIRCLE,
        GP_BUTTON_TRIANGLE,
        GP_BUTTON_SQUARE,
        GP_BUTTON_UP,
        GP_BUTTON_DOWN,
        GP_BUTTON_LEFT,
        GP_BUTTON_RIGHT,
        GP_BUTTON_R1,
        GP_BUTTON_R2,
        GP_BUTTON_R3,
        GP_BUTTON_L1,
        GP_BUTTON_L2,
        GP_BUTTON_L3,
        GP_BUTTON_OPTIONS,
        GP_BUTTON_TOUCH_PAD,
        GP_BUTTON_INTERCEPTED
    };

    struct GamePadState
    {
        unsigned char*  m_buttons = nullptr;
        float*          m_axis = nullptr;
        int             m_axes_count = 0;
        int             m_buttons_count = 0;
    };

    void init();
    void shutdown();

    void newFrame();
    void render();

    void toggleEnabled();
    bool isEnabled();

    void demo();

    // If focused is true, ImGui captures inputs, otherwise inputs are ignored by
    // imgui and passed to the game as normal
    void setFocus(bool val);
    bool focused();

    void StyleColorsFocused();
    void StyleColorsUnfocused();

    void setGamePadState(const GamePadState& state);
}

#endif