#pragma once

#include "Vertex.h"

#include <d3d9.h>
#include <memory>
#include <vector>

namespace Engine
{
    class Mesh
    {
    public:
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        static std::shared_ptr<Mesh> CreateTexturedCube(IDirect3DDevice9* device);

        void Bind(IDirect3DDevice9* device) const;
        void DrawAll(IDirect3DDevice9* device) const;
        void DrawFace(IDirect3DDevice9* device, int faceIndex) const;

        int FaceCount() const { return faceCount_; }
        float BoundingRadius() const { return boundingRadius_; }

    private:
        Mesh() = default;

        IDirect3DVertexBuffer9* vertexBuffer_ = nullptr;
        IDirect3DIndexBuffer9* indexBuffer_ = nullptr;
        UINT vertexCount_ = 0;
        UINT primitiveCount_ = 0;
        int faceCount_ = 0;
        float boundingRadius_ = 1.7321f;
    };
}
