#pragma once

#include <d3d9.h>
#include <d3dx9.h>

namespace Engine
{
    class PixelShaderProgram
    {
    public:
        ~PixelShaderProgram();

        PixelShaderProgram(const PixelShaderProgram&) = delete;
        PixelShaderProgram& operator=(const PixelShaderProgram&) = delete;

        PixelShaderProgram() = default;

        bool LoadFromFile(IDirect3DDevice9* device, const char* path);
        void Apply(IDirect3DDevice9* device) const;
        void Reset();

        bool IsReady() const { return shader_ != nullptr; }

    private:
        IDirect3DPixelShader9* shader_ = nullptr;
    };
}
