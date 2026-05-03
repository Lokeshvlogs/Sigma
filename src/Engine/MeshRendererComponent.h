#pragma once

#include "Component.h"
#include "Mesh.h"
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
        bool IsSelected() const { return selected_; }

        void Render(GameObject& owner, GameContext& context) override;

    private:
        std::shared_ptr<Mesh> mesh_;
        std::shared_ptr<Texture2D> texture_;
        std::vector<D3DCOLOR> faceTints_;
        bool selected_ = false;
    };
}
