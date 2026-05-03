#include "SelectionComponent.h"

#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "InputManager.h"
#include "GameObject.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"

#include <d3dx9.h>

namespace Engine
{
    SelectionComponent::SelectionComponent(float localBoundingRadius)
        : localBoundingRadius_(localBoundingRadius)
    {
    }

    void SelectionComponent::Update(GameObject& owner, float, GameContext& context)
    {
        if (context.input.WasMousePressed(MouseButton::Left))
        {
            selected_ = HitTest(owner, context);
        }

        if (auto* renderer = owner.GetComponent<MeshRendererComponent>())
        {
            renderer->SetSelected(selected_);
        }
    }

    bool SelectionComponent::HitTest(GameObject& owner, GameContext& context) const
    {
        auto* transform = owner.GetComponent<TransformComponent>();
        if (!transform)
        {
            return false;
        }

        D3DVIEWPORT9 viewport = context.renderer.Viewport();
        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);

        D3DXVECTOR3 nearPoint(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 0.0f);
        D3DXVECTOR3 farPoint(static_cast<float>(context.input.MouseX()), static_cast<float>(context.input.MouseY()), 1.0f);
        D3DXVECTOR3 rayStart;
        D3DXVECTOR3 rayEnd;

        D3DXVec3Unproject(&rayStart, &nearPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);
        D3DXVec3Unproject(&rayEnd, &farPoint, &viewport, &context.renderer.ProjectionMatrix(), &context.renderer.ViewMatrix(), &identity);

        D3DXVECTOR3 rayDirection = rayEnd - rayStart;
        D3DXVec3Normalize(&rayDirection, &rayDirection);

        D3DXVECTOR3 toCenter = transform->position - rayStart;
        float projection = D3DXVec3Dot(&toCenter, &rayDirection);
        if (projection < 0.0f)
        {
            return false;
        }

        D3DXVECTOR3 closestPoint = rayStart + rayDirection * projection;
        D3DXVECTOR3 difference = transform->position - closestPoint;
        float radius = transform->BoundingRadius(localBoundingRadius_);
        return D3DXVec3LengthSq(&difference) <= radius * radius;
    }
}
