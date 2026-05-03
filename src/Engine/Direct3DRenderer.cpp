#include "Direct3DRenderer.h"

#include "ComPtrUtils.h"

namespace Engine
{
    namespace
    {
        LONG AtLeastOne(LONG value)
        {
            return value > 1L ? value : 1L;
        }
    }

    Direct3DRenderer::~Direct3DRenderer()
    {
        Shutdown();
    }

    HRESULT Direct3DRenderer::Initialize(HWND hwnd, int width, int height)
    {
        hwnd_ = hwnd;
        d3d_ = Direct3DCreate9(D3D_SDK_VERSION);
        if (!d3d_)
        {
            return E_FAIL;
        }

        ZeroMemory(&presentParameters_, sizeof(presentParameters_));
        presentParameters_.Windowed = TRUE;
        presentParameters_.SwapEffect = D3DSWAPEFFECT_DISCARD;
        presentParameters_.BackBufferFormat = D3DFMT_UNKNOWN;
        presentParameters_.BackBufferWidth = width;
        presentParameters_.BackBufferHeight = height;
        presentParameters_.EnableAutoDepthStencil = TRUE;
        presentParameters_.AutoDepthStencilFormat = D3DFMT_D16;
        presentParameters_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        presentParameters_.hDeviceWindow = hwnd_;

        DWORD behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING;
        HRESULT hr = d3d_->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            hwnd_,
            behavior,
            &presentParameters_,
            &device_);

        if (FAILED(hr))
        {
            behavior = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
            hr = d3d_->CreateDevice(
                D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                hwnd_,
                behavior,
                &presentParameters_,
                &device_);
        }

        if (FAILED(hr))
        {
            return hr;
        }

        ConfigureRenderStates();
        ConfigureCameraAndProjection();
        deviceReady_ = true;
        return S_OK;
    }

    void Direct3DRenderer::Shutdown()
    {
        SafeRelease(device_);
        SafeRelease(d3d_);
        deviceReady_ = false;
        hwnd_ = nullptr;
    }

    HRESULT Direct3DRenderer::Reset()
    {
        if (!device_)
        {
            return E_FAIL;
        }

        deviceReady_ = false;

        RECT client = {};
        GetClientRect(hwnd_, &client);
        presentParameters_.BackBufferWidth = static_cast<UINT>(AtLeastOne(client.right - client.left));
        presentParameters_.BackBufferHeight = static_cast<UINT>(AtLeastOne(client.bottom - client.top));

        HRESULT hr = device_->Reset(&presentParameters_);
        if (SUCCEEDED(hr))
        {
            ConfigureRenderStates();
            ConfigureCameraAndProjection();
            deviceReady_ = true;
        }

        return hr;
    }

    bool Direct3DRenderer::PrepareFrame()
    {
        if (!device_ || !deviceReady_)
        {
            return false;
        }

        HRESULT cooperative = device_->TestCooperativeLevel();
        if (cooperative == D3DERR_DEVICELOST)
        {
            deviceReady_ = false;
            Sleep(20);
            return false;
        }

        if (cooperative == D3DERR_DEVICENOTRESET)
        {
            Reset();
            return false;
        }

        return true;
    }

    bool Direct3DRenderer::BeginFrame(D3DCOLOR clearColor)
    {
        device_->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0);
        return SUCCEEDED(device_->BeginScene());
    }

    void Direct3DRenderer::EndFrame()
    {
        device_->EndScene();
        HRESULT present = device_->Present(nullptr, nullptr, nullptr, nullptr);
        if (present == D3DERR_DEVICELOST)
        {
            deviceReady_ = false;
        }
    }

    D3DVIEWPORT9 Direct3DRenderer::Viewport() const
    {
        D3DVIEWPORT9 viewport = {};
        if (device_)
        {
            device_->GetViewport(&viewport);
        }
        return viewport;
    }

    void Direct3DRenderer::ConfigureRenderStates()
    {
        device_->SetRenderState(D3DRS_ZENABLE, TRUE);
        device_->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
        device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        device_->SetRenderState(D3DRS_LIGHTING, FALSE);
        device_->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(255, 255, 255));

        D3DMATERIAL9 material = {};
        material.Diffuse.r = material.Diffuse.g = material.Diffuse.b = material.Diffuse.a = 1.0f;
        material.Ambient = material.Diffuse;
        device_->SetMaterial(&material);

        device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        device_->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        device_->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        device_->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        device_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        device_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }

    void Direct3DRenderer::ConfigureCameraAndProjection()
    {
        RECT client = {};
        GetClientRect(hwnd_, &client);
        float width = static_cast<float>(AtLeastOne(client.right - client.left));
        float height = static_cast<float>(AtLeastOne(client.bottom - client.top));

        D3DXVECTOR3 eye(0.0f, 2.2f, -5.5f);
        D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

        D3DXMatrixLookAtLH(&view_, &eye, &target, &up);
        device_->SetTransform(D3DTS_VIEW, &view_);

        D3DXMatrixPerspectiveFovLH(&projection_, D3DX_PI / 4.0f, width / height, 0.1f, 100.0f);
        device_->SetTransform(D3DTS_PROJECTION, &projection_);
    }
}
