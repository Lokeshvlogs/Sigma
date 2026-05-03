#include "AssetManager.h"

#include "PathUtils.h"

namespace Engine
{
    AssetManager& AssetManager::Instance()
    {
        static AssetManager assetManager;
        return assetManager;
    }

    std::shared_ptr<Mesh> AssetManager::LoadMesh(IDirect3DDevice9* device, const std::string& assetPath)
    {
        auto existing = meshes_.find(assetPath);
        if (existing != meshes_.end())
        {
            if (auto mesh = existing->second.lock())
            {
                return mesh;
            }
        }

        char path[MAX_PATH];
        BuildExecutableRelativePath(assetPath.c_str(), path, MAX_PATH);

        auto mesh = Mesh::LoadFromFile(device, path);
        meshes_[assetPath] = mesh;
        return mesh;
    }

    std::shared_ptr<Texture2D> AssetManager::LoadTexture(IDirect3DDevice9* device, const std::string& assetPath)
    {
        auto existing = textures_.find(assetPath);
        if (existing != textures_.end())
        {
            if (auto texture = existing->second.lock())
            {
                return texture;
            }
        }

        char path[MAX_PATH];
        BuildExecutableRelativePath(assetPath.c_str(), path, MAX_PATH);

        auto texture = Texture2D::LoadFromFile(device, path);
        textures_[assetPath] = texture;
        return texture;
    }

    void AssetManager::Clear()
    {
        meshes_.clear();
        textures_.clear();
    }
}
