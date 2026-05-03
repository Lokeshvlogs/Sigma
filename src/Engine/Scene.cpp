#include "Scene.h"

#include <utility>

namespace Engine
{
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
