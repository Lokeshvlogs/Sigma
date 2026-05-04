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
        HRESULT DeviceCooperativeLevel() const;
        void SetCameraTransform(const D3DXVECTOR3& eye, const D3DXVECTOR3& target, const D3DXVECTOR3& up);

        IDirect3DDevice9* Device() const { return device_; }
        const D3DXMATRIX& ViewMatrix() const { return view_; }
        const D3DXMATRIX& ProjectionMatrix() const { return projection_; }
        const D3DXVECTOR3& Eye() const { return eye_; }
        const D3DXVECTOR3& Target() const { return target_; }
        const D3DXVECTOR3& Up() const { return up_; }
        D3DVIEWPORT9 Viewport() const;

    private:
        void ConfigureRenderStates();
        void ConfigureCameraAndProjection();

        HWND hwnd_ = nullptr;
        IDirect3D9* d3d_ = nullptr;
        IDirect3DDevice9* device_ = nullptr;
        D3DPRESENT_PARAMETERS presentParameters_ = {};
        bool deviceReady_ = false;
        D3DXVECTOR3 eye_ = D3DXVECTOR3(0.0f, 300.0f, 300.5f);
        D3DXVECTOR3 target_ = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 up_ = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        D3DXMATRIX view_ = {};
        D3DXMATRIX projection_ = {};
    };
}
