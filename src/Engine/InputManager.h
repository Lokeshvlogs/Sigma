#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Engine
{
    enum class MouseButton
    {
        Left = 0,
        Right = 1,
        Middle = 2,
        Count = 3
    };

    class InputManager
    {
    public:
        void BeginFrame();
        void HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);

        bool IsKeyDown(int virtualKey) const;
        bool WasKeyPressed(int virtualKey) const;
        bool WasKeyReleased(int virtualKey) const;

        bool IsMouseDown(MouseButton button) const;
        bool WasMousePressed(MouseButton button) const;
        bool WasMouseReleased(MouseButton button) const;

        int MouseX() const { return mouseX_; }
        int MouseY() const { return mouseY_; }
        int MouseDeltaX() const { return mouseDeltaX_; }
        int MouseDeltaY() const { return mouseDeltaY_; }
        int WheelDelta() const { return wheelDelta_; }

    private:
        bool keys_[256] = {};
        bool previousKeys_[256] = {};
        bool mouseButtons_[static_cast<int>(MouseButton::Count)] = {};
        bool previousMouseButtons_[static_cast<int>(MouseButton::Count)] = {};
        int mouseX_ = 0;
        int mouseY_ = 0;
        int mouseDeltaX_ = 0;
        int mouseDeltaY_ = 0;
        int wheelDelta_ = 0;
    };
}
