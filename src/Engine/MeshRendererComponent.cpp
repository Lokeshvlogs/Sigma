#include "MeshRendererComponent.h"

#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "GameObject.h"
#include "TransformComponent.h"

#include <utility>

namespace Engine
{
    namespace
    {
        int Red(D3DCOLOR color)
        {
            return static_cast<int>((color >> 16) & 0xff);
        }

        int Green(D3DCOLOR color)
        {
            return static_cast<int>((color >> 8) & 0xff);
        }

        int Blue(D3DCOLOR color)
        {
            return static_cast<int>(color & 0xff);
        }

        int AddClamped(int value, int amount)
        {
            value += amount;
            return value > 255 ? 255 : value;
        }
    }

    MeshRendererComponent::MeshRendererComponent(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture2D> texture)
        : mesh_(std::move(mesh))
        , texture_(std::move(texture))
    {
    }

    void MeshRendererComponent::SetFaceTints(std::vector<D3DCOLOR> faceTints)
    {
        faceTints_ = std::move(faceTints);
    }

    void MeshRendererComponent::Render(GameObject& owner, GameContext& context)
    {
        if (!mesh_ || !texture_)
        {
            return;
        }

        auto* transform = owner.GetComponent<TransformComponent>();
        if (!transform)
        {
            return;
        }

        IDirect3DDevice9* device = context.renderer.Device();
        D3DXMATRIX world = transform->WorldMatrix();
        device->SetTransform(D3DTS_WORLD, &world);
        device->SetTexture(0, texture_->Native());
        mesh_->Bind(device);

        if (static_cast<int>(faceTints_.size()) == mesh_->FaceCount())
        {
            for (int face = 0; face < mesh_->FaceCount(); ++face)
            {
                D3DCOLOR tint = faceTints_[face];
                if (selected_)
                {
                    tint = D3DCOLOR_XRGB(
                        AddClamped(Red(tint), 22),
                        AddClamped(Green(tint), 28),
                        AddClamped(Blue(tint), 48));
                }
                device->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
                mesh_->DrawFace(device, face);
            }
            return;
        }

        device->SetRenderState(D3DRS_TEXTUREFACTOR, selected_ ? D3DCOLOR_XRGB(220, 235, 255) : D3DCOLOR_XRGB(255, 255, 255));
        mesh_->DrawAll(device);
    }
}
