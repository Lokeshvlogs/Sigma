#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

namespace Engine
{
    class Direct3DRenderer
    {
    public:
        ~Direct3DRenderer();

        HRESULT Initialize(HWND hwnd, int width, int height);
        void Shutdown();
        HRESULT Reset();

        bool IsReady() const { return deviceReady_; }
        bool PrepareFrame();
        bool BeginFrame(D3DCOLOR clearColor);
        void EndFrame();

        IDirect3DDevice9* Device() const { return device_; }
        const D3DXMATRIX& ViewMatrix() const { return view_; }
        const D3DXMATRIX& ProjectionMatrix() const { return projection_; }
        D3DVIEWPORT9 Viewport() const;

    private:
        void ConfigureRenderStates();
        void ConfigureCameraAndProjection();

        HWND hwnd_ = nullptr;
        IDirect3D9* d3d_ = nullptr;
        IDirect3DDevice9* device_ = nullptr;
        D3DPRESENT_PARAMETERS presentParameters_ = {};
        bool deviceReady_ = false;
        D3DXMATRIX view_ = {};
        D3DXMATRIX projection_ = {};
    };
}
