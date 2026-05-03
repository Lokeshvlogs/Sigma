#include "Texture2D.h"

#include "ComPtrUtils.h"

namespace Engine
{
    Texture2D::~Texture2D()
    {
        SafeRelease(texture_);
    }

    std::shared_ptr<Texture2D> Texture2D::LoadFromFile(IDirect3DDevice9* device, const char* path)
    {
        std::shared_ptr<Texture2D> texture(new Texture2D());
        HRESULT hr = D3DXCreateTextureFromFileA(device, path, &texture->texture_);
        if (FAILED(hr))
        {
            return nullptr;
        }

        return texture;
    }
}
