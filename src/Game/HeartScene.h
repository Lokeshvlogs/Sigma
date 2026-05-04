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
        void RenderImGuiControls() override;
        void OnUpdate(float deltaSeconds, Engine::GameContext& context) override;

    private:
        void UpdateInnerHeartHighlight();
        void UpdateHighlightFromMouseHover(Engine::GameContext& context);

        Engine::SceneObject* innerHeartObject_ = nullptr;
        int highlightRedChannelId_ = 0;
        int hoveredRedChannelId_ = -1;
        bool hoverDrivenHighlightEnabled_ = false;
    };
}
