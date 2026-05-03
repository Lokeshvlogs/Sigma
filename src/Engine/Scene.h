#pragma once

#include "GameObject.h"

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
        void Start();
        void Update(float deltaSeconds, GameContext& context);
        void Render(GameContext& context);

    private:
        std::vector<std::unique_ptr<GameObject>> objects_;
    };
}
