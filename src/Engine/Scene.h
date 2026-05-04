#pragma once

#include "SceneSettings.h"

#include "GameObject.h"
#include "SceneObject.h"

#include <memory>
#include <vector>

namespace Engine
{
    class Direct3DRenderer;
    struct GameContext;

    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual const char* SceneName() const = 0;
        virtual const char* SettingsFilePath() const = 0;
        virtual bool Load(GameContext& context) = 0;
        virtual void RenderImGuiControls() {}
        virtual void OnUpdate(float, GameContext&) {}
        virtual void Unload() {}

        bool LoadSettingsFromFile();
        void ApplyLoadedSettings(Direct3DRenderer& renderer);
        bool SaveSettingsToFile(const Direct3DRenderer& renderer, int windowWidth, int windowHeight);

        GameObject& CreateObject(const char* name);
        SceneObject& CreateSceneObject(GameObject& gameObject);
        std::vector<std::unique_ptr<SceneObject>>& SceneObjects() { return sceneObjects_; }
        const std::vector<std::unique_ptr<SceneObject>>& SceneObjects() const { return sceneObjects_; }
        void Start();
        void Update(float deltaSeconds, GameContext& context);
        const SceneSettings& Settings() const { return settings_; }

    private:
        SceneSettings settings_;
        std::vector<std::unique_ptr<GameObject>> objects_;
        std::vector<std::unique_ptr<SceneObject>> sceneObjects_;
    };
}
