#pragma once

#include "Texture2D.h"

#include <d3d9.h>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
    struct Material
    {
        std::shared_ptr<Texture2D> diffuseTexture;
        std::shared_ptr<Texture2D> normalMap;
        std::shared_ptr<Texture2D> bumpMap;

        std::string diffuseTexturePath;
        std::string normalMapPath;
        std::string bumpMapPath;
        std::string pixelShaderPath;
        std::string highlightPixelShaderPath;
        std::string overlayPixelShaderPath;

        std::vector<D3DCOLOR> faceTints;
        D3DCOLOR defaultTint = D3DCOLOR_XRGB(255, 255, 255);
        D3DCOLOR selectedTint = D3DCOLOR_XRGB(220, 235, 255);
        float highlightColor[4] = { 0.05f, 0.38f, 1.0f, 0.52f };
        float overlayColor[4] = { 1.0f, 0.35f, 0.15f, 0.6f };
        float overlayParameters[4] = { 0.0f, 0.5f / 255.0f, 0.0f, 0.0f };
        bool overlayEnabled = false;
    };
}
