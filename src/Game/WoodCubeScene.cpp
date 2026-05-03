#include "WoodCubeScene.h"

#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/Mesh.h"
#include "../Engine/MeshRendererComponent.h"
#include "../Engine/PathUtils.h"
#include "../Engine/RotationComponents.h"
#include "../Engine/SelectionComponent.h"
#include "../Engine/Texture2D.h"
#include "../Engine/TransformComponent.h"

#include <vector>

namespace Game
{
    bool WoodCubeScene::Load(Engine::GameContext& context)
    {
        char texturePath[MAX_PATH];
        Engine::BuildExecutableRelativePath("assets\\textures\\wood.jpg", texturePath, MAX_PATH);
        if (!Engine::FileExists(texturePath))
        {
            Engine::BuildExecutableRelativePath("wood.jpg", texturePath, MAX_PATH);
        }

        auto mesh = Engine::Mesh::CreateTexturedCube(context.renderer.Device());
        auto texture = Engine::Texture2D::LoadFromFile(context.renderer.Device(), texturePath);
        if (!mesh || !texture)
        {
            return false;
        }

        auto& cube = CreateObject("Wood Cube");
        auto& transform = cube.AddComponent<Engine::TransformComponent>();
        transform.rotation.x = 0.35f;

        auto& renderer = cube.AddComponent<Engine::MeshRendererComponent>(mesh, texture);
        renderer.SetFaceTints(std::vector<D3DCOLOR>
        {
            D3DCOLOR_XRGB(232, 222, 204),
            D3DCOLOR_XRGB(130, 124, 116),
            D3DCOLOR_XRGB(168, 158, 145),
            D3DCOLOR_XRGB(218, 205, 185),
            D3DCOLOR_XRGB(255, 245, 222),
            D3DCOLOR_XRGB(112, 106, 98),
        });

        cube.AddComponent<Engine::SelectionComponent>(mesh->BoundingRadius());
        cube.AddComponent<Engine::AutoRotateComponent>(0.9f, 0.35f);
        cube.AddComponent<Engine::KeyboardRotationComponent>(1.8f);
        cube.AddComponent<Engine::MouseDragRotationComponent>(0.008f);
        return true;
    }
}
