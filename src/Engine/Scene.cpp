#include "Scene.h"

#include "Direct3DRenderer.h"
#include "SceneSettingsIO.h"
#include "TransformComponent.h"

#include <utility>

namespace Engine
{
    namespace
    {
        TransformComponent* SceneObjectTransform(SceneObject& sceneObject)
        {
            if (!sceneObject.gameObject)
            {
                return nullptr;
            }

            return sceneObject.gameObject->GetComponent<TransformComponent>();
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

            for (const ObjectTransformSettings& objectSettings : settings_.objects)
            {
                if (sceneObject->gameObject->Name() != objectSettings.name)
                {
                    continue;
                }

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
        currentSettings.objects.clear();

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

            ObjectTransformSettings objectSettings;
            objectSettings.name = sceneObject->gameObject->Name();
            objectSettings.translation = transform->position;
            objectSettings.rotation = transform->rotation;
            objectSettings.scale = transform->scale;
            currentSettings.objects.push_back(objectSettings);
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
    }
}
