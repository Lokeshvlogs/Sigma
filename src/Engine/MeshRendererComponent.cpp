#include "MeshRendererComponent.h"

#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "GameObject.h"
#include "PathUtils.h"
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

        float ColorChannel(int value)
        {
            return static_cast<float>(value) / 255.0f;
        }

        void SetTintConstant(IDirect3DDevice9* device, D3DCOLOR tint)
        {
            const float color[] =
            {
                ColorChannel(Red(tint)),
                ColorChannel(Green(tint)),
                ColorChannel(Blue(tint)),
                1.0f
            };
            device->SetPixelShaderConstantF(0, color, 1);
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

        EnsureShaders(device);
        RenderWoodPass(device);
        RenderHighlightPass(device);

        device->SetPixelShader(nullptr);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    }

    bool MeshRendererComponent::EnsureShaders(IDirect3DDevice9* device)
    {
        if (woodShader_.IsReady() && highlightShader_.IsReady())
        {
            return true;
        }

        if (shaderInitializationAttempted_)
        {
            return false;
        }

        shaderInitializationAttempted_ = true;

        char woodShaderPath[MAX_PATH];
        char highlightShaderPath[MAX_PATH];
        BuildExecutableRelativePath("assets\\shaders\\wood_pixel.hlsl", woodShaderPath, MAX_PATH);
        BuildExecutableRelativePath("assets\\shaders\\face_highlight_pixel.hlsl", highlightShaderPath, MAX_PATH);

        return woodShader_.LoadFromFile(device, woodShaderPath) &&
            highlightShader_.LoadFromFile(device, highlightShaderPath);
    }

    void MeshRendererComponent::RenderWoodPass(IDirect3DDevice9* device)
    {
        if (woodShader_.IsReady())
        {
            woodShader_.Apply(device);
        }

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
                if (woodShader_.IsReady())
                {
                    SetTintConstant(device, tint);
                }
                else
                {
                    device->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
                }
                mesh_->DrawFace(device, face);
            }
            return;
        }

        D3DCOLOR tint = selected_ ? D3DCOLOR_XRGB(220, 235, 255) : D3DCOLOR_XRGB(255, 255, 255);
        if (woodShader_.IsReady())
        {
            SetTintConstant(device, tint);
        }
        else
        {
            device->SetRenderState(D3DRS_TEXTUREFACTOR, tint);
        }
        mesh_->DrawAll(device);
    }

    void MeshRendererComponent::RenderHighlightPass(IDirect3DDevice9* device)
    {
        if (!highlightShader_.IsReady() || hoveredFaceIndex_ < 0 || hoveredFaceIndex_ >= mesh_->FaceCount())
        {
            return;
        }

        const float highlightColor[] = { 0.05f, 0.38f, 1.0f, 0.52f };
        device->SetTexture(0, nullptr);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        highlightShader_.Apply(device);
        device->SetPixelShaderConstantF(0, highlightColor, 1);
        mesh_->DrawFace(device, hoveredFaceIndex_);
    }
}
