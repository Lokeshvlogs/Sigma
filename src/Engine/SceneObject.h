#pragma once

#include "Material.h"
#include "Mesh.h"

#include <d3d9.h>
#include <memory>
#include <string>
#include <vector>

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

    enum class RenderPassPhase
    {
        Base = 0,
        Overlay = 1
    };

    enum class RenderPassDrawMode
    {
        All,
        FaceTints,
        HoveredFace
    };

    enum class PixelShaderConstantSource
    {
        None,
        TintColor,
        HighlightColor,
        OverlayColor,
        OverlayParameters
    };

    struct RenderPassSettings
    {
        RenderPassPhase phase = RenderPassPhase::Base;
        std::string pixelShaderPath;
        RenderStateSettings renderStates;
        bool bindMaterialTextures = false;
        RenderPassDrawMode drawMode = RenderPassDrawMode::All;
        PixelShaderConstantSource constant0Source = PixelShaderConstantSource::None;
        PixelShaderConstantSource constant1Source = PixelShaderConstantSource::None;
        bool enabled = true;
    };

    struct SceneObject
    {
        GameObject* gameObject = nullptr;
        std::string meshAssetPath;
        std::shared_ptr<Mesh> mesh;
        Material material;
        std::vector<RenderPassSettings> renderPasses;
        int hoveredFaceIndex = -1;
        bool selected = false;
    };
}
