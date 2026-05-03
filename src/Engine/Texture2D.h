#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <memory>

namespace Engine
{
    class Texture2D
    {
    public:
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        static std::shared_ptr<Texture2D> LoadFromFile(IDirect3DDevice9* device, const char* path);

        IDirect3DTexture9* Native() const { return texture_; }

    private:
        Texture2D() = default;

        IDirect3DTexture9* texture_ = nullptr;
    };
}
