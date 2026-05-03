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

        float ColorChannel(int value)
        {
            return static_cast<float>(value) / 255.0f;
        }

        void SetTintConstant(IDirect3DDevice9* device, D3DCOLOR tint)
        {
            const float color[] =
            {
                ColorChannel(Red(tint)),
                ColorChannel(Green(tint)),
                ColorChannel(Blue(tint)),
                1.0f
            };
            device->SetPixelShaderConstantF(0, color, 1);
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
        for (Scene* scene : scenes_)
        {
            if (!scene)
            {
                continue;
            }

            for (auto& sceneObject : scene->SceneObjects())
            {
                RenderSceneObject(*sceneObject, context.renderer);
            }
        }
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

    void RenderManager::RenderSceneObject(SceneObject& sceneObject, Direct3DRenderer& renderer)
    {
        if (!sceneObject.gameObject || !sceneObject.gameObject->IsActive() || !sceneObject.mesh)
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

        ApplyRenderStates(sceneObject, device);
        RenderBasePass(sceneObject, device);
        RenderHighlightPass(sceneObject, device);

        device->SetPixelShader(nullptr);
        device->SetTexture(0, nullptr);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }

    void RenderManager::RenderBasePass(SceneObject& sceneObject, IDirect3DDevice9* device)
    {
        PixelShaderProgram* shader = LoadPixelShader(device, sceneObject.material.pixelShaderPath);
        if (shader)
        {
            shader->Apply(device);
        }

        device->SetTexture(0, sceneObject.material.diffuseTexture ? sceneObject.material.diffuseTexture->Native() : nullptr);

        if (static_cast<int>(sceneObject.material.faceTints.size()) == sceneObject.mesh->FaceCount())
        {
            for (int face = 0; face < sceneObject.mesh->FaceCount(); ++face)
            {
                D3DCOLOR tint = sceneObject.selected ? sceneObject.material.selectedTint : sceneObject.material.faceTints[face];
                if (shader)
                {
                    SetTintConstant(device, tint);
                }
                else
                {
                    device->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
                }
                sceneObject.mesh->DrawFace(device, face);
            }
            return;
        }

        D3DCOLOR tint = sceneObject.selected ? sceneObject.material.selectedTint : sceneObject.material.defaultTint;
        if (shader)
        {
            SetTintConstant(device, tint);
        }
        else
        {
            device->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
        }
        sceneObject.mesh->DrawAll(device);
    }

    void RenderManager::RenderHighlightPass(SceneObject& sceneObject, IDirect3DDevice9* device)
    {
        if (sceneObject.hoveredFaceIndex < 0 || sceneObject.hoveredFaceIndex >= sceneObject.mesh->FaceCount())
        {
            return;
        }

        PixelShaderProgram* shader = LoadPixelShader(device, sceneObject.material.highlightPixelShaderPath);
        if (!shader)
        {
            return;
        }

        device->SetTexture(0, nullptr);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        shader->Apply(device);
        device->SetPixelShaderConstantF(0, sceneObject.material.highlightColor, 1);
        sceneObject.mesh->DrawFace(device, sceneObject.hoveredFaceIndex);
    }

    void RenderManager::ApplyRenderStates(const SceneObject& sceneObject, IDirect3DDevice9* device)
    {
        device->SetRenderState(D3DRS_ZENABLE, sceneObject.renderStates.zEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, sceneObject.renderStates.zWriteEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_CULLMODE, sceneObject.renderStates.cullMode);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, sceneObject.renderStates.alphaBlendEnabled ? TRUE : FALSE);
        device->SetRenderState(D3DRS_SRCBLEND, sceneObject.renderStates.sourceBlend);
        device->SetRenderState(D3DRS_DESTBLEND, sceneObject.renderStates.destinationBlend);
    }
}
