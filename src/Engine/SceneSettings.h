#pragma once

#include <d3dx9.h>

#include <string>
#include <vector>

namespace Engine
{
    struct ObjectTransformSettings
    {
        std::string name;
        D3DXVECTOR3 translation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    };

    struct SceneSettings
    {
        std::string sceneName;
        int windowWidth = 1920;
        int windowHeight = 1080;
        D3DXVECTOR3 cameraEye = D3DXVECTOR3(0.0f, 300.0f, 300.5f);
        D3DXVECTOR3 cameraTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 cameraUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        std::vector<ObjectTransformSettings> objects;
    };
}
