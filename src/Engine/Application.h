#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Direct3DRenderer.h"
#include "InputManager.h"
#include "Scene.h"

#include <memory>

namespace Engine
{
    class Application
    {
    public:
        Application(HINSTANCE instance, int showCommand);
        int Run(std::unique_ptr<Scene> scene);

    private:
        static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        bool CreateMainWindow();
        void ShowFatalError(const char* message, HRESULT hr = S_OK);
        void UpdateWindowTitle(float fps);

        HINSTANCE instance_ = nullptr;
        int showCommand_ = SW_SHOW;
        HWND hwnd_ = nullptr;
        Direct3DRenderer renderer_;
        InputManager input_;
        LARGE_INTEGER qpcFrequency_ = {};
        LARGE_INTEGER lastCounter_ = {};
        float fpsAccumulator_ = 0.0f;
        int fpsFrames_ = 0;
    };
}
