#pragma once

#include "../Engine/Scene.h"

namespace Game
{
    class HeartScene final : public Engine::Scene
    {
    public:
        bool Load(Engine::GameContext& context) override;
    };
}
