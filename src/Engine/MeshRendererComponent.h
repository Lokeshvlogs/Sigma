#pragma once

#include "Component.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"

#include <memory>
#include <vector>

namespace Engine
{
    class MeshRendererComponent final : public Component
    {
    public:
        MeshRendererComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture2D> texture);

        void SetFaceTints(std::vector<D3DCOLOR> faceTints);
        void SetSelected(bool selected) { selected_ = selected; }
        void SetHoveredFaceIndex(int faceIndex) { hoveredFaceIndex_ = faceIndex; }
        bool IsSelected() const { return selected_; }

        void Render(GameObject& owner, GameContext& context) override;

    private:
        bool EnsureShaders(IDirect3DDevice9* device);
        void RenderWoodPass(IDirect3DDevice9* device);
        void RenderHighlightPass(IDirect3DDevice9* device);

        std::shared_ptr<Mesh> mesh_;
        std::shared_ptr<Texture2D> texture_;
        std::vector<D3DCOLOR> faceTints_;
        PixelShaderProgram woodShader_;
        PixelShaderProgram highlightShader_;
        bool shaderInitializationAttempted_ = false;
        int hoveredFaceIndex_ = -1;
        bool selected_ = false;
    };
}
