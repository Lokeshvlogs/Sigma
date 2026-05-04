#pragma once

#include "../Engine/Scene.h"

namespace Game
{
    class SampleScene final : public Engine::Scene
    {
    public:
        const char* SceneName() const override { return "SampleScene"; }
        const char* SettingsFilePath() const override { return "src\\Game\\SampleScene\\SampleScene.xml"; }
        bool Load(Engine::GameContext& context) override;
    };
}
