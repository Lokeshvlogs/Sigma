#pragma once

#include "Component.h"

#include <d3dx9.h>

namespace Engine
{
    class TransformComponent final : public Component
    {
    public:
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

        D3DXMATRIX WorldMatrix() const;
        float BoundingRadius(float baseRadius) const;
    };
}
