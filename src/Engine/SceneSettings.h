#pragma once

#include "SceneObject.h"

#include <array>
#include <d3dx9.h>

#include <string>
#include <vector>

namespace Engine
{
    struct MaterialSettings
    {
        std::string diffuseTexturePath;
        std::string diffuseFallbackTexturePath;
        std::string normalMapPath;
        std::string bumpMapPath;
        std::vector<D3DCOLOR> faceTints;
        D3DCOLOR defaultTint = D3DCOLOR_XRGB(255, 255, 255);
        D3DCOLOR selectedTint = D3DCOLOR_XRGB(220, 235, 255);
        std::array<float, 4> highlightColor = { 0.05f, 0.38f, 1.0f, 0.52f };
        std::array<float, 4> overlayColor = { 1.0f, 0.35f, 0.15f, 0.6f };
        std::array<float, 4> overlayParameters = { 0.0f, 0.5f / 255.0f, 0.0f, 0.0f };
    };

    struct InteractionSettings
    {
        bool selectionEnabled = false;
        float selectionBoundingRadius = 0.0f;
        float keyboardRotationSpeed = 0.0f;
        float mouseDragSensitivity = 0.0f;
    };

    struct SceneObjectSettings
    {
        std::string name;
        std::string meshPath;
        D3DXVECTOR3 translation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
        MaterialSettings material;
        InteractionSettings interaction;
        std::vector<RenderPassSettings> renderPasses;
    };

    struct SceneSettings
    {
        std::string sceneName;
        int windowWidth = 1920;
        int windowHeight = 1080;
        D3DXVECTOR3 cameraEye = D3DXVECTOR3(0.0f, 300.0f, 300.5f);
        D3DXVECTOR3 cameraTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 cameraUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        std::vector<SceneObjectSettings> objects;
    };
}
