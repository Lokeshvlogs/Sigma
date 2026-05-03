#include "SampleScene.h"

#include "../Engine/AssetManager.h"
#include "../Engine/Direct3DRenderer.h"
#include "../Engine/GameContext.h"
#include "../Engine/Material.h"
#include "../Engine/RotationComponents.h"
#include "../Engine/SceneObject.h"
#include "../Engine/SelectionComponent.h"
#include "../Engine/TransformComponent.h"

#include <vector>

namespace Game
{
    bool SampleScene::Load(Engine::GameContext& context)
    {
        const std::string meshAssetPath = "assets\\static_objects\\wooden_cube.fbx";
        const std::string textureAssetPath = "assets\\textures\\wood019_color_1k.png";

        Engine::AssetManager& assets = Engine::AssetManager::Instance();
        auto mesh = assets.LoadMesh(context.renderer.Device(), meshAssetPath);
        auto texture = assets.LoadTexture(context.renderer.Device(), textureAssetPath);
        if (!mesh || !texture)
        {
            return false;
        }

        auto& cube = CreateObject("Sample Cube");
        auto& transform = cube.AddComponent<Engine::TransformComponent>();
        transform.rotation.x = 0.35f;

        auto& sceneObject = CreateSceneObject(cube);
        sceneObject.mesh = mesh;
        sceneObject.material.diffuseTexture = texture;
        sceneObject.material.diffuseTexturePath = textureAssetPath;
        sceneObject.material.pixelShaderPath = "assets\\shaders\\base_textured_pixel.hlsl";
        sceneObject.material.highlightPixelShaderPath = "assets\\shaders\\face_highlight_pixel.hlsl";
        sceneObject.material.faceTints = std::vector<D3DCOLOR>
        {
            D3DCOLOR_XRGB(232, 222, 204),
            D3DCOLOR_XRGB(130, 124, 116),
            D3DCOLOR_XRGB(168, 158, 145),
            D3DCOLOR_XRGB(218, 205, 185),
            D3DCOLOR_XRGB(255, 245, 222),
            D3DCOLOR_XRGB(112, 106, 98),
        };
        sceneObject.renderStates.zEnabled = true;
        sceneObject.renderStates.zWriteEnabled = true;
        sceneObject.renderStates.cullMode = D3DCULL_NONE;

        cube.AddComponent<Engine::SelectionComponent>(mesh->BoundingRadius(), &sceneObject);
        cube.AddComponent<Engine::AutoRotateComponent>(0.9f, 0.35f);
        cube.AddComponent<Engine::KeyboardRotationComponent>(1.8f);
        cube.AddComponent<Engine::MouseDragRotationComponent>(0.008f);
        return true;
    }
}
