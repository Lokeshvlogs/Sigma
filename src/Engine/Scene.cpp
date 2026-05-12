#include "Scene.h"

#include "AssetManager.h"
#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "RotationComponents.h"
#include "SceneSettingsIO.h"
#include "SelectionComponent.h"
#include "TransformComponent.h"

#include <utility>

namespace Engine
{
    namespace
    {
        SceneObjectSettings* FindObjectSettingsByName(std::vector<SceneObjectSettings>& objects, const std::string& name)
        {
            for (SceneObjectSettings& object : objects)
            {
                if (object.name == name)
                {
                    return &object;
                }
            }

            return nullptr;
        }

        std::shared_ptr<Texture2D> LoadTextureWithFallback(
            AssetManager& assets,
            IDirect3DDevice9* device,
            const std::string& primaryPath,
            const std::string& fallbackPath)
        {
            if (primaryPath.empty())
            {
                return nullptr;
            }

            auto texture = assets.LoadTexture(device, primaryPath);
            if (texture || fallbackPath.empty())
            {
                return texture;
            }

            return assets.LoadTexture(device, fallbackPath);
        }
    }

    bool Scene::LoadSettingsFromFile()
    {
        SceneSettings loadedSettings;
        loadedSettings.sceneName = SceneName();
        if (LoadSceneSettingsXml(SettingsFilePath(), loadedSettings))
        {
            settings_ = std::move(loadedSettings);
            return true;
        }

        settings_ = std::move(loadedSettings);
        return false;
    }

    bool Scene::LoadSceneObjectsFromSettings(GameContext& context)
    {
        if (settings_.objects.empty())
        {
            return false;
        }

        AssetManager& assets = AssetManager::Instance();
        for (const SceneObjectSettings& objectSettings : settings_.objects)
        {
            if (objectSettings.name.empty() || objectSettings.meshPath.empty())
            {
                return false;
            }

            auto mesh = assets.LoadMesh(context.renderer.Device(), objectSettings.meshPath);
            if (!mesh)
            {
                return false;
            }

            auto diffuseTexture = LoadTextureWithFallback(
                assets,
                context.renderer.Device(),
                objectSettings.material.diffuseTexturePath,
                objectSettings.material.diffuseFallbackTexturePath);

            if (!objectSettings.material.diffuseTexturePath.empty() && !diffuseTexture)
            {
                return false;
            }

            std::shared_ptr<Texture2D> normalMap = objectSettings.material.normalMapPath.empty()
                ? nullptr
                : assets.LoadTexture(context.renderer.Device(), objectSettings.material.normalMapPath);
            std::shared_ptr<Texture2D> bumpMap = objectSettings.material.bumpMapPath.empty()
                ? nullptr
                : assets.LoadTexture(context.renderer.Device(), objectSettings.material.bumpMapPath);

            auto& gameObject = CreateObject(objectSettings.name.c_str());
            gameObject.SetActive(objectSettings.visible);
            auto& transform = gameObject.AddComponent<TransformComponent>();
            transform.position = objectSettings.translation;
            transform.rotation = objectSettings.rotation;
            transform.scale = objectSettings.scale;

            auto& sceneObject = CreateSceneObject(gameObject);
            sceneObject.meshAssetPath = objectSettings.meshPath;
            sceneObject.mesh = mesh;
            sceneObject.material.diffuseTexture = diffuseTexture;
            sceneObject.material.normalMap = normalMap;
            sceneObject.material.bumpMap = bumpMap;
            sceneObject.material.diffuseTexturePath = objectSettings.material.diffuseTexturePath;
            sceneObject.material.normalMapPath = objectSettings.material.normalMapPath;
            sceneObject.material.bumpMapPath = objectSettings.material.bumpMapPath;
            sceneObject.material.faceTints = objectSettings.material.faceTints;
            sceneObject.material.defaultTint = objectSettings.material.defaultTint;
            sceneObject.material.selectedTint = objectSettings.material.selectedTint;
            sceneObject.material.shaderParameters = objectSettings.material.shaderParameters;
            sceneObject.material.highlightColor = objectSettings.material.highlightColor;
            sceneObject.material.overlayColor = objectSettings.material.overlayColor;
            sceneObject.material.overlayParameters = objectSettings.material.overlayParameters;
            sceneObject.renderPasses = objectSettings.renderPasses;

            if (objectSettings.interaction.selectionEnabled)
            {
                const float radius = objectSettings.interaction.selectionBoundingRadius > 0.0f
                    ? objectSettings.interaction.selectionBoundingRadius
                    : mesh->BoundingRadius();
                gameObject.AddComponent<SelectionComponent>(radius, &sceneObject);
            }

            if (objectSettings.interaction.keyboardRotationSpeed != 0.0f)
            {
                gameObject.AddComponent<KeyboardRotationComponent>(objectSettings.interaction.keyboardRotationSpeed);
            }

            if (objectSettings.interaction.mouseDragSensitivity != 0.0f)
            {
                gameObject.AddComponent<MouseDragRotationComponent>(objectSettings.interaction.mouseDragSensitivity);
            }
        }

        return true;
    }

    SceneObject* Scene::FindSceneObject(const char* name)
    {
        if (!name)
        {
            return nullptr;
        }

        for (const auto& sceneObject : sceneObjects_)
        {
            if (!sceneObject || !sceneObject->gameObject)
            {
                continue;
            }

            if (sceneObject->gameObject->Name() == name)
            {
                return sceneObject.get();
            }
        }

        return nullptr;
    }

    void Scene::ApplyLoadedSettings(Direct3DRenderer& renderer)
    {
        renderer.SetCameraTransform(settings_.cameraEye, settings_.cameraTarget, settings_.cameraUp);

        for (auto& sceneObject : sceneObjects_)
        {
            if (!sceneObject || !sceneObject->gameObject)
            {
                continue;
            }

            TransformComponent* transform = sceneObject->gameObject->GetComponent<TransformComponent>();
            if (!transform)
            {
                continue;
            }

            for (const SceneObjectSettings& objectSettings : settings_.objects)
            {
                if (sceneObject->gameObject->Name() != objectSettings.name)
                {
                    continue;
                }

                sceneObject->gameObject->SetActive(objectSettings.visible);
                transform->position = objectSettings.translation;
                transform->rotation = objectSettings.rotation;
                transform->scale = objectSettings.scale;
                break;
            }
        }
    }

    bool Scene::SaveSettingsToFile(const Direct3DRenderer& renderer, int windowWidth, int windowHeight)
    {
        SceneSettings currentSettings = settings_;
        currentSettings.sceneName = SceneName();
        currentSettings.windowWidth = windowWidth;
        currentSettings.windowHeight = windowHeight;
        currentSettings.cameraEye = renderer.Eye();
        currentSettings.cameraTarget = renderer.Target();
        currentSettings.cameraUp = renderer.Up();

        for (const auto& sceneObject : sceneObjects_)
        {
            if (!sceneObject || !sceneObject->gameObject)
            {
                continue;
            }

            const TransformComponent* transform = sceneObject->gameObject->GetComponent<TransformComponent>();
            if (!transform)
            {
                continue;
            }

            SceneObjectSettings* objectSettings = FindObjectSettingsByName(currentSettings.objects, sceneObject->gameObject->Name());
            if (!objectSettings)
            {
                currentSettings.objects.push_back(SceneObjectSettings{});
                objectSettings = &currentSettings.objects.back();
                objectSettings->name = sceneObject->gameObject->Name();
            }

            objectSettings->name = sceneObject->gameObject->Name();
            if (!sceneObject->meshAssetPath.empty())
            {
                objectSettings->meshPath = sceneObject->meshAssetPath;
            }

            objectSettings->visible = sceneObject->gameObject->IsActive();
            objectSettings->translation = transform->position;
            objectSettings->rotation = transform->rotation;
            objectSettings->scale = transform->scale;
            objectSettings->material.diffuseTexturePath = sceneObject->material.diffuseTexturePath;
            objectSettings->material.normalMapPath = sceneObject->material.normalMapPath;
            objectSettings->material.bumpMapPath = sceneObject->material.bumpMapPath;
            objectSettings->material.faceTints = sceneObject->material.faceTints;
            objectSettings->material.defaultTint = sceneObject->material.defaultTint;
            objectSettings->material.selectedTint = sceneObject->material.selectedTint;
            objectSettings->material.shaderParameters = sceneObject->material.shaderParameters;
            objectSettings->material.highlightColor = sceneObject->material.highlightColor;
            objectSettings->material.overlayColor = sceneObject->material.overlayColor;
            objectSettings->material.overlayParameters = sceneObject->material.overlayParameters;
            objectSettings->renderPasses = sceneObject->renderPasses;
        }

        if (!SaveSceneSettingsXml(SettingsFilePath(), currentSettings))
        {
            return false;
        }

        settings_ = std::move(currentSettings);
        return true;
    }

    GameObject& Scene::CreateObject(const char* name)
    {
        auto object = std::make_unique<GameObject>(name);
        GameObject& reference = *object;
        objects_.push_back(std::move(object));
        return reference;
    }

    SceneObject& Scene::CreateSceneObject(GameObject& gameObject)
    {
        auto sceneObject = std::make_unique<SceneObject>();
        sceneObject->gameObject = &gameObject;
        SceneObject& reference = *sceneObject;
        sceneObjects_.push_back(std::move(sceneObject));
        return reference;
    }

    void Scene::Start()
    {
        for (auto& object : objects_)
        {
            object->Start();
        }
    }

    void Scene::Update(float deltaSeconds, GameContext& context)
    {
        for (auto& object : objects_)
        {
            object->Update(deltaSeconds, context);
        }

        OnUpdate(deltaSeconds, context);
    }
}
