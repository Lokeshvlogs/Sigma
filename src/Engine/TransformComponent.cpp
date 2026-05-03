#include "TransformComponent.h"

namespace Engine
{
    D3DXMATRIX TransformComponent::WorldMatrix() const
    {
        D3DXMATRIX scaleMatrix;
        D3DXMATRIX rotationX;
        D3DXMATRIX rotationY;
        D3DXMATRIX rotationZ;
        D3DXMATRIX translation;

        D3DXMatrixScaling(&scaleMatrix, scale.x, scale.y, scale.z);
        D3DXMatrixRotationX(&rotationX, rotation.x);
        D3DXMatrixRotationY(&rotationY, rotation.y);
        D3DXMatrixRotationZ(&rotationZ, rotation.z);
        D3DXMatrixTranslation(&translation, position.x, position.y, position.z);

        return scaleMatrix * rotationX * rotationY * rotationZ * translation;
    }

    float TransformComponent::BoundingRadius(float baseRadius) const
    {
        float maxScale = scale.x;
        if (scale.y > maxScale)
        {
            maxScale = scale.y;
        }
        if (scale.z > maxScale)
        {
            maxScale = scale.z;
        }
        return baseRadius * maxScale;
    }
}
