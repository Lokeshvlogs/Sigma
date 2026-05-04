#include "HeartScene.h"

#include "../Engine/AssetManager.h"
#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/Material.h"
#include "../Engine/RotationComponents.h"
#include "../Engine/SceneObject.h"
#include "../Engine/TransformComponent.h"

namespace Game
{
    namespace
    {
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

        bool AddHeartObject(Engine::Scene& scene, Engine::GameContext& context, const HeartAssetSet& assetSet)
        {
            Engine::AssetManager& assets = Engine::AssetManager::Instance();

            auto mesh = assets.LoadMesh(context.renderer.Device(), assetSet.meshPath);
            if (!mesh)
            {
                return false;
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
                return false;
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
            sceneObject.renderStates.zEnabled = true;
            sceneObject.renderStates.zWriteEnabled = assetSet.zWriteEnabled;
            sceneObject.renderStates.cullMode = D3DCULL_NONE;
            sceneObject.renderStates.alphaBlendEnabled = assetSet.alphaBlendEnabled;

            //heartPart.AddComponent<Engine::AutoRotateComponent>(0.55f, 0.18f);
            heartPart.AddComponent<Engine::KeyboardRotationComponent>(1.2f);
            heartPart.AddComponent<Engine::MouseDragRotationComponent>(0.006f);
            return true;
        }
    }

    bool HeartScene::Load(Engine::GameContext& context)
    {
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
            false
        };

        const bool innerLoaded = AddHeartObject(*this, context, innerHeart);
        return innerLoaded && AddHeartObject(*this, context, outerHeart);
    }
}
