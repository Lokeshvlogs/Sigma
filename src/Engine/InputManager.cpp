#include "InputManager.h"

#include <windowsx.h>
#include <cstring>

namespace Engine
{
    namespace
    {
        int ButtonIndex(MouseButton button)
        {
            return static_cast<int>(button);
        }
    }

    void InputManager::BeginFrame()
    {
        memcpy(previousKeys_, keys_, sizeof(keys_));
        memcpy(previousMouseButtons_, mouseButtons_, sizeof(mouseButtons_));
        mouseDeltaX_ = 0;
        mouseDeltaY_ = 0;
        wheelDelta_ = 0;
    }

    void InputManager::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (wParam < 256)
            {
                keys_[wParam] = true;
            }
            break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam < 256)
            {
                keys_[wParam] = false;
            }
            break;

        case WM_LBUTTONDOWN:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Left)] = true;
            break;

        case WM_LBUTTONUP:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Left)] = false;
            break;

        case WM_RBUTTONDOWN:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Right)] = true;
            break;

        case WM_RBUTTONUP:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Right)] = false;
            break;

        case WM_MBUTTONDOWN:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Middle)] = true;
            break;

        case WM_MBUTTONUP:
            mouseX_ = GET_X_LPARAM(lParam);
            mouseY_ = GET_Y_LPARAM(lParam);
            mouseButtons_[ButtonIndex(MouseButton::Middle)] = false;
            break;

        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            mouseDeltaX_ += x - mouseX_;
            mouseDeltaY_ += y - mouseY_;
            mouseX_ = x;
            mouseY_ = y;
            break;
        }

        case WM_MOUSEWHEEL:
            wheelDelta_ += GET_WHEEL_DELTA_WPARAM(wParam);
            break;
        }
    }

    bool InputManager::IsKeyDown(int virtualKey) const
    {
        return virtualKey >= 0 && virtualKey < 256 && keys_[virtualKey];
    }

    bool InputManager::WasKeyPressed(int virtualKey) const
    {
        return virtualKey >= 0 && virtualKey < 256 && keys_[virtualKey] && !previousKeys_[virtualKey];
    }

    bool InputManager::WasKeyReleased(int virtualKey) const
    {
        return virtualKey >= 0 && virtualKey < 256 && !keys_[virtualKey] && previousKeys_[virtualKey];
    }

    bool InputManager::IsMouseDown(MouseButton button) const
    {
        return mouseButtons_[ButtonIndex(button)];
    }

    bool InputManager::WasMousePressed(MouseButton button) const
    {
        int index = ButtonIndex(button);
        return mouseButtons_[index] && !previousMouseButtons_[index];
    }

    bool InputManager::WasMouseReleased(MouseButton button) const
    {
        int index = ButtonIndex(button);
        return !mouseButtons_[index] && previousMouseButtons_[index];
    }
}
