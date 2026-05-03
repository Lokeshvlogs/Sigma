#pragma once

#include "Mesh.h"
#include "Texture2D.h"

#include <d3d9.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Engine
{
    class AssetManager final
    {
    public:
        static AssetManager& Instance();

        std::shared_ptr<Mesh> LoadMesh(IDirect3DDevice9* device, const std::string& assetPath);
        std::shared_ptr<Texture2D> LoadTexture(IDirect3DDevice9* device, const std::string& assetPath);
        void Clear();

    private:
        AssetManager() = default;

        std::unordered_map<std::string, std::weak_ptr<Mesh>> meshes_;
        std::unordered_map<std::string, std::weak_ptr<Texture2D>> textures_;
    };
}
