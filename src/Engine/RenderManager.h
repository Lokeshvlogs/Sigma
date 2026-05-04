#pragma once

#include "SceneObject.h"
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

    class RenderManager
    {
    public:
        void AddScene(Scene& scene);
        void ClearScenes();
        void Render(GameContext& context);

    private:
        void ClearBoundResources(IDirect3DDevice9* device);
        PixelShaderProgram* LoadPixelShader(IDirect3DDevice9* device, const std::string& assetPath);
        void RenderSceneObject(SceneObject& sceneObject, Direct3DRenderer& renderer, RenderPassPhase phase);
        void RenderPass(SceneObject& sceneObject, const RenderPassSettings& renderPass, IDirect3DDevice9* device);
        void ApplyRenderStates(const RenderStateSettings& renderStates, IDirect3DDevice9* device);

        std::vector<Scene*> scenes_;
        std::map<std::string, std::unique_ptr<PixelShaderProgram>> pixelShaders_;
    };
}
