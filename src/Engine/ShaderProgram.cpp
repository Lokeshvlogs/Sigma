#include "ShaderProgram.h"

#include "ComPtrUtils.h"

namespace Engine
{
    PixelShaderProgram::~PixelShaderProgram()
    {
        Reset();
    }

    bool PixelShaderProgram::LoadFromFile(IDirect3DDevice9* device, const char* path)
    {
        Reset();

        const D3DXMACRO defines[] =
        {
            { "SIGMA_RUNTIME_SHADER", "1" },
            { nullptr, nullptr }
        };

        ID3DXBuffer* byteCode = nullptr;
        ID3DXBuffer* errors = nullptr;
        HRESULT hr = D3DXCompileShaderFromFileA(
            path,
            defines,
            nullptr,
            "PS_Main",
            "ps_2_0",
            0,
            &byteCode,
            &errors,
            nullptr);

        SafeRelease(errors);
        if (FAILED(hr) || !byteCode)
        {
            SafeRelease(byteCode);
            return false;
        }

        hr = device->CreatePixelShader(
            static_cast<DWORD*>(byteCode->GetBufferPointer()),
            &shader_);
        SafeRelease(byteCode);

        return SUCCEEDED(hr);
    }

    void PixelShaderProgram::Apply(IDirect3DDevice9* device) const
    {
        device->SetPixelShader(shader_);
    }

    void PixelShaderProgram::Reset()
    {
        SafeRelease(shader_);
    }
}
