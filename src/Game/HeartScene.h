#pragma once

#include "../Engine/Scene.h"

namespace Game
{
    class HeartScene final : public Engine::Scene
    {
    public:
        const char* SceneName() const override { return "HeartScene"; }
        const char* SettingsFilePath() const override { return "src\\Game\\HeartScene\\HeartScene.xml"; }
        bool Load(Engine::GameContext& context) override;
    };
}
