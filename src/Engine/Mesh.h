#pragma once

#include "Vertex.h"

#include <d3d9.h>
#include <memory>
#include <string>
#include <vector>

namespace Engine
{
    struct MeshFaceSpan
    {
        UINT startIndex = 0;
        UINT primitiveCount = 0;
    };

    class Mesh
    {
    public:
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        static std::shared_ptr<Mesh> LoadFromFile(IDirect3DDevice9* device, const char* path);

        void Bind(IDirect3DDevice9* device) const;
        void DrawAll(IDirect3DDevice9* device) const;
        void DrawFace(IDirect3DDevice9* device, int faceIndex) const;

        int FaceCount() const { return faceCount_; }
        float BoundingRadius() const { return boundingRadius_; }

    private:
        Mesh() = default;
        static std::shared_ptr<Mesh> CreateFromData(
            IDirect3DDevice9* device,
            const std::vector<Vertex>& vertices,
            const std::vector<WORD>& indices,
            std::vector<MeshFaceSpan> faceSpans,
            float boundingRadius);

        static std::shared_ptr<Mesh> CreateFromData(
            IDirect3DDevice9* device,
            const std::vector<Vertex>& vertices,
            const std::vector<DWORD>& indices,
            std::vector<MeshFaceSpan> faceSpans,
            float boundingRadius);

        IDirect3DVertexBuffer9* vertexBuffer_ = nullptr;
        IDirect3DIndexBuffer9* indexBuffer_ = nullptr;
        D3DFORMAT indexFormat_ = D3DFMT_INDEX16;
        UINT vertexCount_ = 0;
        UINT primitiveCount_ = 0;
        int faceCount_ = 0;
        float boundingRadius_ = 1.7321f;
        std::vector<MeshFaceSpan> faceSpans_;
    };
}
