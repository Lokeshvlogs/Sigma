#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>

#include <string>

namespace Engine
{
    class Direct3DRenderer;
    class Scene;

    class ImGuiManager
    {
    public:
        bool Initialize(HWND hwnd, IDirect3DDevice9* device);
        void Shutdown();

        bool HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        void BeginFrame();
        void EndFrame();
        void Render(Scene& scene, Direct3DRenderer& renderer, HWND hwnd);

        void InvalidateDeviceObjects();
        void CreateDeviceObjects();

        bool WantsKeyboardCapture() const { return wantsKeyboardCapture_; }
        bool WantsMouseCapture() const { return wantsMouseCapture_; }

    private:
        int selectedSceneObjectIndex_ = 0;
        bool initialized_ = false;
        bool frameActive_ = false;
        bool wantsKeyboardCapture_ = false;
        bool wantsMouseCapture_ = false;
        std::string saveStatus_;
    };
}
