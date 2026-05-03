#pragma once

#include "Material.h"
#include "Mesh.h"

#include <d3d9.h>
#include <memory>

namespace Engine
{
    class GameObject;

    struct RenderStateSettings
    {
        bool zEnabled = true;
        bool zWriteEnabled = true;
        D3DCULL cullMode = D3DCULL_NONE;
        bool alphaBlendEnabled = false;
        D3DBLEND sourceBlend = D3DBLEND_SRCALPHA;
        D3DBLEND destinationBlend = D3DBLEND_INVSRCALPHA;
    };

    struct SceneObject
    {
        GameObject* gameObject = nullptr;
        std::shared_ptr<Mesh> mesh;
        Material material;
        RenderStateSettings renderStates;
        int hoveredFaceIndex = -1;
        bool selected = false;
    };
}
