#pragma once

#include "ShaderProgram.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
    class Direct3DRenderer;
    class Scene;
    struct GameContext;
    struct SceneObject;

    class RenderManager
    {
    public:
        void AddScene(Scene& scene);
        void ClearScenes();
        void Render(GameContext& context);

    private:
        PixelShaderProgram* LoadPixelShader(IDirect3DDevice9* device, const std::string& assetPath);
        void RenderSceneObjectBasePass(SceneObject& sceneObject, Direct3DRenderer& renderer);
        void RenderSceneObjectOverlayPasses(SceneObject& sceneObject, Direct3DRenderer& renderer);
        void RenderBasePass(SceneObject& sceneObject, IDirect3DDevice9* device);
        void RenderHighlightPass(SceneObject& sceneObject, IDirect3DDevice9* device);
        void RenderOverlayPass(SceneObject& sceneObject, IDirect3DDevice9* device);
        void ApplyRenderStates(const SceneObject& sceneObject, IDirect3DDevice9* device);

        std::vector<Scene*> scenes_;
        std::map<std::string, std::unique_ptr<PixelShaderProgram>> pixelShaders_;
    };
}
