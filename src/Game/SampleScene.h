#pragma once

#include "../Engine/Scene.h"

namespace Game
{
    class SampleScene final : public Engine::Scene
    {
    public:
        bool Load(Engine::GameContext& context) override;
    };
}
