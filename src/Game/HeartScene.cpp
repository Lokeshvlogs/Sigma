#include "HeartScene.h"

#include "../Engine/AssetManager.h"
#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/Material.h"
#include "../Engine/PathUtils.h"
#include "../Engine/RotationComponents.h"
#include "../Engine/SceneObject.h"
#include "../Engine/TransformComponent.h"

#include <cstdarg>
#include <cstdio>

namespace Game
{
    namespace
    {
        void AppendHeartLoadLog(const char* format, ...)
        {
            char logPath[MAX_PATH];
            Engine::BuildExecutableRelativePath("heart_scene_load.log", logPath, MAX_PATH);

            FILE* logFile = nullptr;
            if (fopen_s(&logFile, logPath, "a") != 0 || !logFile)
            {
                return;
            }

            va_list args;
            va_start(args, format);
            vfprintf(logFile, format, args);
            va_end(args);
            fprintf(logFile, "\n");
            fclose(logFile);
        }

        void ResetHeartLoadLog()
        {
            char logPath[MAX_PATH];
            Engine::BuildExecutableRelativePath("heart_scene_load.log", logPath, MAX_PATH);

            FILE* logFile = nullptr;
            if (fopen_s(&logFile, logPath, "w") != 0 || !logFile)
            {
                return;
            }

            fprintf(logFile, "HeartScene load trace\n");
            fclose(logFile);
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
        };

        std::shared_ptr<Engine::Texture2D> LoadTextureWithFallback(
            Engine::AssetManager& assets,
            IDirect3DDevice9* device,
            const char* primaryPath,
            const char* fallbackPath,
            const char* label,
            const char*& loadedPath)
        {
            loadedPath = primaryPath;
            auto texture = assets.LoadTexture(device, primaryPath);
            if (texture || !fallbackPath)
            {
                return texture;
            }

            AppendHeartLoadLog("  %s failed: %s", label, primaryPath);
            texture = assets.LoadTexture(device, fallbackPath);
            if (texture)
            {
                loadedPath = fallbackPath;
                AppendHeartLoadLog("  %s fallback: %s", label, fallbackPath);
            }
            return texture;
        }

        bool AddHeartObject(Engine::Scene& scene, Engine::GameContext& context, const HeartAssetSet& assetSet)
        {
            Engine::AssetManager& assets = Engine::AssetManager::Instance();
            AppendHeartLoadLog("Loading %s", assetSet.objectName);

            auto mesh = assets.LoadMesh(context.renderer.Device(), assetSet.meshPath);
            if (!mesh)
            {
                AppendHeartLoadLog("  mesh failed: %s", assetSet.meshPath);
                return false;
            }

            const char* diffuseTexturePath = assetSet.diffusePath;
            auto diffuseTexture = LoadTextureWithFallback(
                assets,
                context.renderer.Device(),
                assetSet.diffusePath,
                assetSet.diffuseFallbackPath,
                "diffuse",
                diffuseTexturePath);
            if (!diffuseTexture)
            {
                AppendHeartLoadLog("  diffuse failed: %s", assetSet.diffusePath);
                return false;
            }

            auto normalMap = assets.LoadTexture(context.renderer.Device(), assetSet.normalPath);
            if (!normalMap)
            {
                AppendHeartLoadLog("  optional normal missing: %s", assetSet.normalPath);
            }

            auto bumpMap = assets.LoadTexture(context.renderer.Device(), assetSet.bumpPath);
            if (!bumpMap)
            {
                AppendHeartLoadLog("  optional bump missing: %s", assetSet.bumpPath);
            }

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
            AppendHeartLoadLog("  success");
            return true;
        }
    }

    bool HeartScene::Load(Engine::GameContext& context)
    {
        ResetHeartLoadLog();

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
        const bool outerLoaded = innerLoaded && AddHeartObject(*this, context, outerHeart);
        AppendHeartLoadLog("HeartScene result: inner=%s outer=%s", innerLoaded ? "ok" : "failed", outerLoaded ? "ok" : "failed");
        return innerLoaded && outerLoaded;
    }
}
