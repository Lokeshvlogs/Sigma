#include "SelectionComponent.h"

#include "Direct3DRenderer.h"
#include "GameContext.h"
#include "InputManager.h"
#include "GameObject.h"
#include "MeshRendererComponent.h"
#include "TransformComponent.h"

#include <d3dx9.h>
#include <cmath>

namespace Engine
{
    SelectionComponent::SelectionComponent(float localBoundingRadius)
        : localBoundingRadius_(localBoundingRadius)
    {
    }

    void SelectionComponent::Update(GameObject& owner, float, GameContext& context)
    {
        hoveredFaceIndex_ = HitTestFace(owner, context);

        if (context.input.WasMousePressed(MouseButton::Left))
        {
            selected_ = hoveredFaceIndex_ != -1;
        }

        if (auto* renderer = owner.GetComponent<MeshRendererComponent>())
        {
            renderer->SetSelected(selected_);
            renderer->SetHoveredFaceIndex(hoveredFaceIndex_);
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

    int SelectionComponent::HitTestFace(GameObject& owner, GameContext& context) const
    {
        auto* transform = owner.GetComponent<TransformComponent>();
        if (!transform)
        {
            return -1;
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

        D3DXMATRIX world = transform->WorldMatrix();
        D3DXMATRIX inverseWorld;
        if (!D3DXMatrixInverse(&inverseWorld, nullptr, &world))
        {
            return -1;
        }

        D3DXVECTOR3 localStart;
        D3DXVECTOR3 localEnd;
        D3DXVec3TransformCoord(&localStart, &rayStart, &inverseWorld);
        D3DXVec3TransformCoord(&localEnd, &rayEnd, &inverseWorld);

        D3DXVECTOR3 direction = localEnd - localStart;
        D3DXVec3Normalize(&direction, &direction);

        float tMin = -FLT_MAX;
        float tMax = FLT_MAX;
        int faceIndex = -1;

        const float origins[] = { localStart.x, localStart.y, localStart.z };
        const float directions[] = { direction.x, direction.y, direction.z };

        for (int axis = 0; axis < 3; ++axis)
        {
            if (fabsf(directions[axis]) < 0.0001f)
            {
                if (origins[axis] < -1.0f || origins[axis] > 1.0f)
                {
                    return -1;
                }
                continue;
            }

            float nearT = (-1.0f - origins[axis]) / directions[axis];
            float farT = (1.0f - origins[axis]) / directions[axis];
            int nearFace = -1;

            if (nearT > farT)
            {
                float swapT = nearT;
                nearT = farT;
                farT = swapT;
                nearFace = axis == 0 ? 3 : (axis == 1 ? 4 : 1);
            }
            else
            {
                nearFace = axis == 0 ? 2 : (axis == 1 ? 5 : 0);
            }

            if (nearT > tMin)
            {
                tMin = nearT;
                faceIndex = nearFace;
            }

            if (farT < tMax)
            {
                tMax = farT;
            }

            if (tMin > tMax)
            {
                return -1;
            }
        }

        if (tMax < 0.0f)
        {
            return -1;
        }

        return faceIndex;
    }
}
