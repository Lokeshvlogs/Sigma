#pragma once

#include "Vertex.h"

#include <d3d9.h>
#include <d3dx9.h>
#include <cfloat>
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

    struct MeshRaycastHit
    {
        int faceIndex = -1;
        float distance = FLT_MAX;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DCOLOR vertexColor = D3DCOLOR_ARGB(255, 255, 255, 255);
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
        bool Raycast(const D3DXVECTOR3& localStart, const D3DXVECTOR3& localDirection, MeshRaycastHit& hit) const;

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
        std::vector<Vertex> vertices_;
        std::vector<DWORD> indices_;
        std::vector<MeshFaceSpan> faceSpans_;
    };
}
