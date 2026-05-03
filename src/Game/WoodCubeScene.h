#pragma once

#include "../Engine/Scene.h"

namespace Game
{
    class WoodCubeScene final : public Engine::Scene
    {
    public:
        bool Load(Engine::GameContext& context) override;
    };
}
