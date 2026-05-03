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

    void Scene::Render(GameContext& context)
    {
        for (auto& object : objects_)
        {
            object->Render(context);
        }
    }
}
