#include "HeartScene.h"

#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/InputManager.h"
#include "../Engine/SceneObject.h"
#include "../Engine/TransformComponent.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>

namespace Game
{
    namespace
    {
        int RedChannel(D3DCOLOR color)
        {
            return static_cast<int>((color >> 16) & 0xff);
        }

        float NormalizedColorChannel(int value)
        {
            const int clamped = std::max(0, std::min(255, value));
            return static_cast<float>(clamped) / 255.0f;
        }

    }

    bool HeartScene::Load(Engine::GameContext& context)
    {
        if (!LoadSceneObjectsFromSettings(context))
        {
            return false;
        }

        innerHeartObject_ = FindSceneObject("Heart Inside");
        if (!innerHeartObject_)
        {
            return false;
        }

        highlightRedChannelId_ = static_cast<int>(roundf(innerHeartObject_->material.overlayParameters[0] * 255.0f));
        UpdateInnerHeartHighlight();
        return FindSceneObject("Heart Outside") != nullptr;
    }

    void HeartScene::RenderImGuiControls()
    {
        ImGui::SeparatorText("Heart Highlight");
        if (ImGui::Checkbox("Hover-driven highlight", &hoverDrivenHighlightEnabled_))
        {
            if (hoverDrivenHighlightEnabled_)
            {
                hoveredRedChannelId_ = -1;
            }

            UpdateInnerHeartHighlight();
        }

        if (hoverDrivenHighlightEnabled_)
        {
            ImGui::BeginDisabled();
        }
        if (ImGui::SliderInt("Red Channel ID", &highlightRedChannelId_, 0, 255))
        {
            UpdateInnerHeartHighlight();
        }
        if (hoverDrivenHighlightEnabled_)
        {
            ImGui::EndDisabled();
        }

        if (innerHeartObject_ && ImGui::ColorEdit4("Highlight Color", innerHeartObject_->material.overlayColor.data()))
        {
            UpdateInnerHeartHighlight();
        }

        if (hoverDrivenHighlightEnabled_)
        {
            if (hoveredRedChannelId_ >= 0)
            {
                ImGui::Text("Hovered Red Channel ID: %d", hoveredRedChannelId_);
            }
            else
            {
                ImGui::TextUnformatted("Hovered Red Channel ID: none");
            }
        }

        ImGui::TextUnformatted("Highlights Heart Inside regions whose vertex-color red channel matches this ID.");
    }

    void HeartScene::OnUpdate(float, Engine::GameContext& context)
    {
        if (hoverDrivenHighlightEnabled_)
        {
            UpdateHighlightFromMouseHover(context);
        }
    }

    void HeartScene::UpdateInnerHeartHighlight()
    {
        highlightRedChannelId_ = std::max(0, std::min(255, highlightRedChannelId_));
        if (!innerHeartObject_)
        {
            return;
        }

        innerHeartObject_->material.overlayParameters[0] = NormalizedColorChannel(highlightRedChannelId_);
        innerHeartObject_->material.overlayParameters[1] = 0.5f / 255.0f;
    }

    void HeartScene::UpdateHighlightFromMouseHover(Engine::GameContext& context)
    {
        hoveredRedChannelId_ = -1;
        if (!innerHeartObject_ || !innerHeartObject_->mesh || !innerHeartObject_->gameObject || context.uiWantsMouse)
        {
            return;
        }

        auto* transform = innerHeartObject_->gameObject->GetComponent<Engine::TransformComponent>();
        if (!transform)
        {
            return;
        }

        D3DVIEWPORT9 viewport = context.renderer.Viewport();
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);

        D3DXVECTOR3 nearPoint = D3DXVECTOR3(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 0.0f);
        D3DXVECTOR3 farPoint = D3DXVECTOR3(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 1.0f);
        D3DXVECTOR3 rayStart;
        D3DXVECTOR3 rayEnd;

        D3DXVec3Unproject(&rayStart, &nearPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);
        D3DXVec3Unproject(&rayEnd, &farPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);

        D3DXMATRIX world = transform->WorldMatrix();
        D3DXMATRIX inverseWorld;
        if (!D3DXMatrixInverse(&inverseWorld, nullptr, &world))
        {
            return;
        }

        D3DXVECTOR3 localStart;
        D3DXVECTOR3 localEnd;
        D3DXVec3TransformCoord(&localStart, &rayStart, &inverseWorld);
        D3DXVec3TransformCoord(&localEnd, &rayEnd, &inverseWorld);

        D3DXVECTOR3 localDirection = localEnd - localStart;
        D3DXVec3Normalize(&localDirection, &localDirection);

        Engine::MeshRaycastHit hit;
        if (!innerHeartObject_->mesh->Raycast(localStart, localDirection, hit))
        {
            return;
        }

        hoveredRedChannelId_ = RedChannel(hit.vertexColor);
        if (hoveredRedChannelId_ != highlightRedChannelId_)
        {
            highlightRedChannelId_ = hoveredRedChannelId_;
            UpdateInnerHeartHighlight();
        }
    }
}
