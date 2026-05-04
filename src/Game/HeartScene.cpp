#include "HeartScene.h"

#include "../Engine/AssetManager.h"
#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/InputManager.h"
#include "../Engine/Material.h"
#include "../Engine/RotationComponents.h"
#include "../Engine/SceneObject.h"
#include "../Engine/TransformComponent.h"

#include "imgui.h"

#include <algorithm>

namespace Game
{
    namespace
    {
        int RedChannel(D3DCOLOR color)
        {
            return static_cast<int>((color >> 16) & 0xff);
        }

        float NormalizedColorChannel(int value)
        {
            const int clamped = std::max(0, std::min(255, value));
            return static_cast<float>(clamped) / 255.0f;
        }

        struct HeartAssetSet
        {
            const char* meshPath = "";
            const char* diffusePath = "";
            const char* diffuseFallbackPath = nullptr;
            const char* normalPath = "";
            const char* bumpPath = "";
            const char* objectName = "";
            D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255);
            bool alphaBlendEnabled = false;
            bool zWriteEnabled = true;
            bool overlayEnabled = false;
        };

        std::shared_ptr<Engine::Texture2D> LoadTextureWithFallback(
            Engine::AssetManager& assets,
            IDirect3DDevice9* device,
            const char* primaryPath,
            const char* fallbackPath,
            const char*& loadedPath)
        {
            loadedPath = primaryPath;
            auto texture = assets.LoadTexture(device, primaryPath);
            if (texture || !fallbackPath)
            {
                return texture;
            }

            texture = assets.LoadTexture(device, fallbackPath);
            if (texture)
            {
                loadedPath = fallbackPath;
            }
            return texture;
        }

        Engine::SceneObject* AddHeartObject(Engine::Scene& scene, Engine::GameContext& context, const HeartAssetSet& assetSet)
        {
            Engine::AssetManager& assets = Engine::AssetManager::Instance();

            auto mesh = assets.LoadMesh(context.renderer.Device(), assetSet.meshPath);
            if (!mesh)
            {
                return nullptr;
            }

            const char* diffuseTexturePath = assetSet.diffusePath;
            auto diffuseTexture = LoadTextureWithFallback(
                assets,
                context.renderer.Device(),
                assetSet.diffusePath,
                assetSet.diffuseFallbackPath,
                diffuseTexturePath);
            if (!diffuseTexture)
            {
                return nullptr;
            }

            auto normalMap = assets.LoadTexture(context.renderer.Device(), assetSet.normalPath);
            auto bumpMap = assets.LoadTexture(context.renderer.Device(), assetSet.bumpPath);

            auto& heartPart = scene.CreateObject(assetSet.objectName);
            auto& transform = heartPart.AddComponent<Engine::TransformComponent>();
            transform.rotation.x = 0.35f;
            transform.rotation.y = -0.45f;
            transform.scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

            auto& sceneObject = scene.CreateSceneObject(heartPart);
            sceneObject.mesh = mesh;
            sceneObject.material.diffuseTexture = diffuseTexture;
            sceneObject.material.normalMap = normalMap;
            sceneObject.material.bumpMap = bumpMap;
            sceneObject.material.diffuseTexturePath = diffuseTexturePath;
            sceneObject.material.normalMapPath = assetSet.normalPath;
            sceneObject.material.bumpMapPath = assetSet.bumpPath;
            sceneObject.material.pixelShaderPath = "assets\\shaders\\Heart.fx";
            sceneObject.material.defaultTint = assetSet.tint;
            sceneObject.material.selectedTint = assetSet.tint;
            sceneObject.material.overlayEnabled = assetSet.overlayEnabled;
            sceneObject.material.overlayPixelShaderPath = assetSet.overlayEnabled ? "assets\\shaders\\HighlightHeart.fx" : "";
            sceneObject.renderStates.zEnabled = true;
            sceneObject.renderStates.zWriteEnabled = assetSet.zWriteEnabled;
            sceneObject.renderStates.cullMode = D3DCULL_NONE;
            sceneObject.renderStates.alphaBlendEnabled = assetSet.alphaBlendEnabled;

            //heartPart.AddComponent<Engine::AutoRotateComponent>(0.55f, 0.18f);
            heartPart.AddComponent<Engine::KeyboardRotationComponent>(1.2f);
            heartPart.AddComponent<Engine::MouseDragRotationComponent>(0.006f);
            return &sceneObject;
        }
    }

    bool HeartScene::Load(Engine::GameContext& context)
    {
        innerHeartObject_ = nullptr;

        const HeartAssetSet innerHeart
        {
            "assets\\static_objects\\Heart Inside.FBX",
            "assets\\textures\\Heart\\heartIN_diffuse.DDS",
            nullptr,
            "assets\\textures\\Heart\\heartIN_normal.DDS",
            "assets\\bump_maps\\Heart\\Inner.dds",
            "Heart Inside",
            D3DCOLOR_ARGB(255, 255, 255, 255),
            false,
            true,
            true
        };

        const HeartAssetSet outerHeart
        {
            "assets\\static_objects\\Heart Outside.FBX",
            "assets\\textures\\Heart\\Version2\\heart_diffuse_No_AO.DDS",
            "assets\\textures\\Heart\\WetLookOuter.dds",
            "assets\\textures\\Heart\\Version2\\heart_normal.DDS",
            "assets\\bump_maps\\Heart\\Outer.dds",
            "Heart Outside",
            D3DCOLOR_ARGB(172, 255, 255, 255),
            true,
            false,
            false
        };

        innerHeartObject_ = AddHeartObject(*this, context, innerHeart);
        if (!innerHeartObject_)
        {
            return false;
        }

        UpdateInnerHeartHighlight();
        return AddHeartObject(*this, context, outerHeart) != nullptr;
    }

    void HeartScene::RenderImGuiControls()
    {
        ImGui::SeparatorText("Heart Highlight");
        if (ImGui::Checkbox("Hover-driven highlight", &hoverDrivenHighlightEnabled_))
        {
            if (hoverDrivenHighlightEnabled_)
            {
                hoveredRedChannelId_ = -1;
            }

            UpdateInnerHeartHighlight();
        }

        if (hoverDrivenHighlightEnabled_)
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::SliderInt("Red Channel ID", &highlightRedChannelId_, 0, 255))
        {
            UpdateInnerHeartHighlight();
        }
        if (hoverDrivenHighlightEnabled_)
        {
            ImGui::EndDisabled();
        }

        if (innerHeartObject_ && ImGui::ColorEdit4("Highlight Color", innerHeartObject_->material.overlayColor))
        {
            UpdateInnerHeartHighlight();
        }

        if (hoverDrivenHighlightEnabled_)
        {
            if (hoveredRedChannelId_ >= 0)
            {
                ImGui::Text("Hovered Red Channel ID: %d", hoveredRedChannelId_);
            }
            else
            {
                ImGui::TextUnformatted("Hovered Red Channel ID: none");
            }
        }

        ImGui::TextUnformatted("Highlights Heart Inside regions whose vertex-color red channel matches this ID.");
    }

    void HeartScene::OnUpdate(float, Engine::GameContext& context)
    {
        if (hoverDrivenHighlightEnabled_)
        {
            UpdateHighlightFromMouseHover(context);
        }
    }

    void HeartScene::UpdateInnerHeartHighlight()
    {
        highlightRedChannelId_ = std::max(0, std::min(255, highlightRedChannelId_));
        if (!innerHeartObject_)
        {
            return;
        }

        innerHeartObject_->material.overlayEnabled = true;
        innerHeartObject_->material.overlayPixelShaderPath = "assets\\shaders\\HighlightHeart.fx";
        innerHeartObject_->material.overlayParameters[0] = NormalizedColorChannel(highlightRedChannelId_);
        innerHeartObject_->material.overlayParameters[1] = 0.5f / 255.0f;
    }

    void HeartScene::UpdateHighlightFromMouseHover(Engine::GameContext& context)
    {
        hoveredRedChannelId_ = -1;
        if (!innerHeartObject_ || !innerHeartObject_->mesh || !innerHeartObject_->gameObject || context.uiWantsMouse)
        {
            return;
        }

        auto* transform = innerHeartObject_->gameObject->GetComponent<Engine::TransformComponent>();
        if (!transform)
        {
            return;
        }

        D3DVIEWPORT9 viewport = context.renderer.Viewport();
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);

        D3DXVECTOR3 nearPoint = D3DXVECTOR3(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 0.0f);
        D3DXVECTOR3 farPoint = D3DXVECTOR3(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 1.0f);
        D3DXVECTOR3 rayStart;
        D3DXVECTOR3 rayEnd;

        D3DXVec3Unproject(&rayStart, &nearPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);
        D3DXVec3Unproject(&rayEnd, &farPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);

        D3DXMATRIX world = transform->WorldMatrix();
        D3DXMATRIX inverseWorld;
        if (!D3DXMatrixInverse(&inverseWorld, nullptr, &world))
        {
            return;
        }

        D3DXVECTOR3 localStart;
        D3DXVECTOR3 localEnd;
        D3DXVec3TransformCoord(&localStart, &rayStart, &inverseWorld);
        D3DXVec3TransformCoord(&localEnd, &rayEnd, &inverseWorld);

        D3DXVECTOR3 localDirection = localEnd - localStart;
        D3DXVec3Normalize(&localDirection, &localDirection);

        Engine::MeshRaycastHit hit;
        if (!innerHeartObject_->mesh->Raycast(localStart, localDirection, hit))
        {
            return;
        }

        hoveredRedChannelId_ = RedChannel(hit.vertexColor);
        if (hoveredRedChannelId_ != highlightRedChannelId_)
        {
            highlightRedChannelId_ = hoveredRedChannelId_;
            UpdateInnerHeartHighlight();
        }
    }
}
