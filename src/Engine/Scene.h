#pragma once

#include "GameObject.h"
#include "SceneObject.h"

#include <memory>
#include <vector>

namespace Engine
{
    struct GameContext;

    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual bool Load(GameContext& context) = 0;
        virtual void Unload() {}

        GameObject& CreateObject(const char* name);
        SceneObject& CreateSceneObject(GameObject& gameObject);
        std::vector<std::unique_ptr<SceneObject>>& SceneObjects() { return sceneObjects_; }
        void Start();
        void Update(float deltaSeconds, GameContext& context);

    private:
        std::vector<std::unique_ptr<GameObject>> objects_;
        std::vector<std::unique_ptr<SceneObject>> sceneObjects_;
    };
}
