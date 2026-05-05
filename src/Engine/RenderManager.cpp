#include "RenderManager.h"

#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "GameObject.h"
#include "PathUtils.h"
#include "Scene.h"
#include "SceneObject.h"
#include "TransformComponent.h"

#include <algorithm>

namespace Engine
{
    namespace
    {
        int Red(D3DCOLOR color)
        {
            return static_cast<int>((color >> 16) & 0xff);
        }

        int Green(D3DCOLOR color)
        {
            return static_cast<int>((color >> 8) & 0xff);
        }

        int Blue(D3DCOLOR color)
        {
            return static_cast<int>(color & 0xff);
        }

        int Alpha(D3DCOLOR color)
        {
            return static_cast<int>((color >> 24) & 0xff);
        }

        float ColorChannel(int value)
        {
            return static_cast<float>(value) / 255.0f;
        }

        D3DCOLOR ResolveTintColor(const SceneObject& sceneObject, int faceIndex)
        {
            if (sceneObject.selected)
            {
                return sceneObject.material.selectedTint;
            }

            if (faceIndex >= 0 && faceIndex < static_cast<int>(sceneObject.material.faceTints.size()))
            {
                return sceneObject.material.faceTints[faceIndex];
            }

            return sceneObject.material.defaultTint;
        }

        void ApplyPixelShaderConstant(
            IDirect3DDevice9* device,
            const SceneObject& sceneObject,
            PixelShaderConstantSource source,
            UINT registerIndex,
            bool shaderReady,
            D3DCOLOR tintColor)
        {
            switch (source)
            {
            case PixelShaderConstantSource::TintColor:
                if (shaderReady)
                {
                    const float color[] =
                    {
                        ColorChannel(Red(tintColor)),
                        ColorChannel(Green(tintColor)),
                        ColorChannel(Blue(tintColor)),
                        ColorChannel(Alpha(tintColor))
                    };
                    device->SetPixelShaderConstantF(registerIndex, color, 1);
                }
                else if (registerIndex == 0)
                {
                    device->SetRenderState(D3DRS_TEXTUREFACTOR, tintColor);
                }
                break;

            case PixelShaderConstantSource::ShaderParameters:
                if (shaderReady)
                {
                    device->SetPixelShaderConstantF(registerIndex, sceneObject.material.shaderParameters.data(), 1);
                }
                break;

            case PixelShaderConstantSource::HighlightColor:
                if (shaderReady)
                {
                    device->SetPixelShaderConstantF(registerIndex, sceneObject.material.highlightColor.data(), 1);
                }
                break;

            case PixelShaderConstantSource::OverlayColor:
                if (shaderReady)
                {
                    device->SetPixelShaderConstantF(registerIndex, sceneObject.material.overlayColor.data(), 1);
                }
                break;

            case PixelShaderConstantSource::OverlayParameters:
                if (shaderReady)
                {
                    device->SetPixelShaderConstantF(registerIndex, sceneObject.material.overlayParameters.data(), 1);
                }
                break;

            case PixelShaderConstantSource::None:
            default:
                break;
            }
        }

        void ApplyPassConstants(
            IDirect3DDevice9* device,
            const SceneObject& sceneObject,
            const RenderPassSettings& renderPass,
            bool shaderReady,
            D3DCOLOR tintColor)
        {
            ApplyPixelShaderConstant(device, sceneObject, renderPass.constant0Source, 0, shaderReady, tintColor);
            ApplyPixelShaderConstant(device, sceneObject, renderPass.constant1Source, 1, shaderReady, tintColor);
        }
    }

    void RenderManager::AddScene(Scene& scene)
    {
        if (std::find(scenes_.begin(), scenes_.end(), &scene) == scenes_.end())
        {
            scenes_.push_back(&scene);
        }
    }

    void RenderManager::ClearScenes()
    {
        scenes_.clear();
    }

    void RenderManager::Render(GameContext& context)
    {
        for (int phaseValue = static_cast<int>(RenderPassPhase::Base);
            phaseValue <= static_cast<int>(RenderPassPhase::Overlay);
            ++phaseValue)
        {
            const RenderPassPhase phase = static_cast<RenderPassPhase>(phaseValue);
            for (Scene* scene : scenes_)
            {
                if (!scene)
                {
                    continue;
                }

                for (auto& sceneObject : scene->SceneObjects())
                {
                    RenderSceneObject(*sceneObject, context.renderer, phase);
                }
            }
        }
    }

    void RenderManager::ClearBoundResources(IDirect3DDevice9* device)
    {
        device->SetPixelShader(nullptr);
        device->SetTexture(0, nullptr);
        device->SetTexture(1, nullptr);
        device->SetTexture(2, nullptr);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }

    PixelShaderProgram* RenderManager::LoadPixelShader(IDirect3DDevice9* device, const std::string& assetPath)
    {
        if (assetPath.empty())
        {
            return nullptr;
        }

        auto existing = pixelShaders_.find(assetPath);
        if (existing != pixelShaders_.end())
        {
            return existing->second->IsReady() ? existing->second.get() : nullptr;
        }

        char shaderPath[MAX_PATH];
        BuildExecutableRelativePath(assetPath.c_str(), shaderPath, MAX_PATH);

        auto shader = std::make_unique<PixelShaderProgram>();
        PixelShaderProgram* shaderPointer = shader.get();
        shader->LoadFromFile(device, shaderPath);
        pixelShaders_[assetPath] = std::move(shader);

        return shaderPointer->IsReady() ? shaderPointer : nullptr;
    }

    void RenderManager::RenderSceneObject(SceneObject& sceneObject, Direct3DRenderer& renderer, RenderPassPhase phase)
    {
        if (!sceneObject.gameObject || !sceneObject.gameObject->IsActive() || !sceneObject.mesh || sceneObject.renderPasses.empty())
        {
            return;
        }

        auto* transform = sceneObject.gameObject->GetComponent<TransformComponent>();
        if (!transform)
        {
            return;
        }

        IDirect3DDevice9* device = renderer.Device();
        D3DXMATRIX world = transform->WorldMatrix();
        device->SetTransform(D3DTS_WORLD, &world);
        sceneObject.mesh->Bind(device);

        for (const RenderPassSettings& renderPass : sceneObject.renderPasses)
        {
            if (renderPass.phase == phase)
            {
                RenderPass(sceneObject, renderPass, device);
            }
        }

        ClearBoundResources(device);
    }

    void RenderManager::RenderPass(SceneObject& sceneObject, const RenderPassSettings& renderPass, IDirect3DDevice9* device)
    {
        if (!renderPass.enabled)
        {
            return;
        }

        if (renderPass.drawMode == RenderPassDrawMode::HoveredFace &&
            (sceneObject.hoveredFaceIndex < 0 || sceneObject.hoveredFaceIndex >= sceneObject.mesh->FaceCount()))
        {
            return;
        }

        ApplyRenderStates(renderPass.renderStates, device);
        if (renderPass.bindMaterialTextures)
        {
            device->SetTexture(0, sceneObject.material.diffuseTexture ? sceneObject.material.diffuseTexture->Native() : nullptr);
            device->SetTexture(1, sceneObject.material.normalMap ? sceneObject.material.normalMap->Native() : nullptr);
            device->SetTexture(2, sceneObject.material.bumpMap ? sceneObject.material.bumpMap->Native() : nullptr);
        }
        else
        {
            device->SetTexture(0, nullptr);
            device->SetTexture(1, nullptr);
            device->SetTexture(2, nullptr);
        }

        PixelShaderProgram* shader = LoadPixelShader(device, renderPass.shaderProgramPath);
        const bool shaderReady = shader != nullptr;
        if (shaderReady)
        {
            shader->Apply(device);
        }
        else
        {
            device->SetPixelShader(nullptr);
        }

        if (renderPass.drawMode == RenderPassDrawMode::FaceTints &&
            static_cast<int>(sceneObject.material.faceTints.size()) == sceneObject.mesh->FaceCount())
        {
            for (int face = 0; face < sceneObject.mesh->FaceCount(); ++face)
            {
                const D3DCOLOR tint = ResolveTintColor(sceneObject, face);
                ApplyPassConstants(device, sceneObject, renderPass, shaderReady, tint);
                sceneObject.mesh->DrawFace(device, face);
            }
            return;
        }

        const D3DCOLOR tint = ResolveTintColor(sceneObject, -1);
        ApplyPassConstants(device, sceneObject, renderPass, shaderReady, tint);

        if (renderPass.drawMode == RenderPassDrawMode::HoveredFace)
        {
            sceneObject.mesh->DrawFace(device, sceneObject.hoveredFaceIndex);
            return;
        }

        sceneObject.mesh->DrawAll(device);
    }

    void RenderManager::ApplyRenderStates(const RenderStateSettings& renderStates, IDirect3DDevice9* device)
    {
        device->SetRenderState(D3DRS_ZENABLE, renderStates.zEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, renderStates.zWriteEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_CULLMODE, renderStates.cullMode);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, renderStates.alphaBlendEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_SRCBLEND, renderStates.sourceBlend);
        device->SetRenderState(D3DRS_DESTBLEND, renderStates.destinationBlend);
    }
}
