#include "Mesh.h"

#include "ComPtrUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cfloat>

namespace Engine
{
    namespace
    {
        bool IntersectTriangle(
            const D3DXVECTOR3& rayStart,
            const D3DXVECTOR3& rayDirection,
            const D3DXVECTOR3& vertex0,
            const D3DXVECTOR3& vertex1,
            const D3DXVECTOR3& vertex2,
            float& distance,
            D3DXVECTOR3& position)
        {
            const float epsilon = 0.000001f;

            const D3DXVECTOR3 edge1 = vertex1 - vertex0;
            const D3DXVECTOR3 edge2 = vertex2 - vertex0;

            D3DXVECTOR3 p;
            D3DXVec3Cross(&p, &rayDirection, &edge2);
            const float determinant = D3DXVec3Dot(&edge1, &p);
            if (fabsf(determinant) < epsilon)
            {
                return false;
            }

            const float inverseDeterminant = 1.0f / determinant;
            const D3DXVECTOR3 t = rayStart - vertex0;
            const float u = D3DXVec3Dot(&t, &p) * inverseDeterminant;
            if (u < 0.0f || u > 1.0f)
            {
                return false;
            }

            D3DXVECTOR3 q;
            D3DXVec3Cross(&q, &t, &edge1);
            const float v = D3DXVec3Dot(&rayDirection, &q) * inverseDeterminant;
            if (v < 0.0f || u + v > 1.0f)
            {
                return false;
            }

            distance = D3DXVec3Dot(&edge2, &q) * inverseDeterminant;
            if (distance < 0.0f)
            {
                return false;
            }

            position = rayStart + rayDirection * distance;
            return true;
        }

        const Vertex& ClosestTriangleVertex(
            const std::vector<Vertex>& vertices,
            DWORD index0,
            DWORD index1,
            DWORD index2,
            const D3DXVECTOR3& position)
        {
            const Vertex* closest = &vertices[index0];
            float closestDistance = FLT_MAX;

            const DWORD indices[] = { index0, index1, index2 };
            for (DWORD index : indices)
            {
                const Vertex& vertex = vertices[index];
                const D3DXVECTOR3 vertexPosition(vertex.x, vertex.y, vertex.z);
                const D3DXVECTOR3 delta = vertexPosition - position;
                const float distance = D3DXVec3LengthSq(&delta);
                if (distance < closestDistance)
                {
                    closest = &vertex;
                    closestDistance = distance;
                }
            }

            return *closest;
        }

        int ColorComponent(float value)
        {
            const float clamped = std::max(0.0f, std::min(1.0f, value));
            return static_cast<int>(clamped * 255.0f + 0.5f);
        }

        void AppendMeshData(
            const aiMesh& sourceMesh,
            std::vector<Vertex>& vertices,
            std::vector<DWORD>& indices,
            std::vector<MeshFaceSpan>& faceSpans,
            float& boundingRadius)
        {
            const DWORD baseVertex = static_cast<DWORD>(vertices.size());
            for (unsigned int vertexIndex = 0; vertexIndex < sourceMesh.mNumVertices; ++vertexIndex)
            {
                const aiVector3D& position = sourceMesh.mVertices[vertexIndex];
                aiVector3D uv(0.0f, 0.0f, 0.0f);
                if (sourceMesh.HasTextureCoords(0))
                {
                    uv = sourceMesh.mTextureCoords[0][vertexIndex];
                }

                D3DCOLOR color = D3DCOLOR_ARGB(255, 255, 255, 255);
                if (sourceMesh.HasVertexColors(0))
                {
                    const aiColor4D& vertexColor = sourceMesh.mColors[0][vertexIndex];
                    color = D3DCOLOR_ARGB(
                        ColorComponent(vertexColor.a),
                        ColorComponent(vertexColor.r),
                        ColorComponent(vertexColor.g),
                        ColorComponent(vertexColor.b));
                }

                vertices.push_back(
                    {
                        position.x,
                        position.y,
                        position.z,
                        color,
                        uv.x,
                        uv.y
                    });

                float radius = sqrtf(position.x * position.x + position.y * position.y + position.z * position.z);
                boundingRadius = std::max(boundingRadius, radius);
            }

            std::vector<MeshFaceSpan> meshFaceSpans;
            for (unsigned int faceIndex = 0; faceIndex < sourceMesh.mNumFaces; ++faceIndex)
            {
                const aiFace& face = sourceMesh.mFaces[faceIndex];
                if (face.mNumIndices != 3)
                {
                    continue;
                }

                MeshFaceSpan span = {};
                span.startIndex = static_cast<UINT>(indices.size());
                span.primitiveCount = 1;
                meshFaceSpans.push_back(span);

                for (unsigned int indexIndex = 0; indexIndex < face.mNumIndices; ++indexIndex)
                {
                    indices.push_back(baseVertex + face.mIndices[indexIndex]);
                }
            }

            if (meshFaceSpans.size() == 12)
            {
                for (size_t faceIndex = 0; faceIndex < meshFaceSpans.size(); faceIndex += 2)
                {
                    MeshFaceSpan span = {};
                    span.startIndex = meshFaceSpans[faceIndex].startIndex;
                    span.primitiveCount = 2;
                    faceSpans.push_back(span);
                }
                return;
            }

            faceSpans.insert(faceSpans.end(), meshFaceSpans.begin(), meshFaceSpans.end());
        }

    }

    Mesh::~Mesh()
    {
        SafeRelease(indexBuffer_);
        SafeRelease(vertexBuffer_);
    }

    std::shared_ptr<Mesh> Mesh::LoadFromFile(IDirect3DDevice9* device, const char* path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_SortByPType |
            aiProcess_ValidateDataStructure);

        if (!scene || !scene->HasMeshes())
        {
            return nullptr;
        }

        std::vector<Vertex> vertices;
        std::vector<DWORD> indices;
        std::vector<MeshFaceSpan> faceSpans;
        float boundingRadius = 0.0f;
        D3DCAPS9 deviceCaps = {};
        const bool hasDeviceCaps = SUCCEEDED(device->GetDeviceCaps(&deviceCaps));
        const bool supportsIndex32 = !hasDeviceCaps || deviceCaps.MaxVertexIndex > 65535;

        for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            const aiMesh* sourceMesh = scene->mMeshes[meshIndex];
            if (!sourceMesh)
            {
                continue;
            }

            if (!supportsIndex32 && vertices.size() + sourceMesh->mNumVertices > 65535)
            {
                return nullptr;
            }

            AppendMeshData(*sourceMesh, vertices, indices, faceSpans, boundingRadius);
        }

        if (vertices.size() > 65535)
        {
            return CreateFromData(device, vertices, indices, std::move(faceSpans), boundingRadius);
        }

        std::vector<WORD> indices16;
        indices16.reserve(indices.size());
        for (DWORD index : indices)
        {
            indices16.push_back(static_cast<WORD>(index));
        }

        return CreateFromData(device, vertices, indices16, std::move(faceSpans), boundingRadius);
    }

    std::shared_ptr<Mesh> Mesh::CreateFromData(
        IDirect3DDevice9* device,
        const std::vector<Vertex>& vertices,
        const std::vector<WORD>& indices,
        std::vector<MeshFaceSpan> faceSpans,
        float boundingRadius)
    {
        if (vertices.empty() || indices.empty())
        {
            return nullptr;
        }

        std::shared_ptr<Mesh> mesh(new Mesh());
        const UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        HRESULT hr = device->CreateVertexBuffer(vertexBufferSize, 0, Vertex::FVF, D3DPOOL_MANAGED, &mesh->vertexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* vertexMemory = nullptr;
        hr = mesh->vertexBuffer_->Lock(0, vertexBufferSize, &vertexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(vertexMemory, vertices.data(), vertexBufferSize);
        mesh->vertexBuffer_->Unlock();

        const UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(WORD));
        hr = device->CreateIndexBuffer(indexBufferSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mesh->indexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* indexMemory = nullptr;
        hr = mesh->indexBuffer_->Lock(0, indexBufferSize, &indexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(indexMemory, indices.data(), indexBufferSize);
        mesh->indexBuffer_->Unlock();

        mesh->indexFormat_ = D3DFMT_INDEX16;
        mesh->vertexCount_ = static_cast<UINT>(vertices.size());
        mesh->primitiveCount_ = static_cast<UINT>(indices.size() / 3);
        mesh->faceCount_ = static_cast<int>(faceSpans.size());
        mesh->boundingRadius_ = boundingRadius > 0.0f ? boundingRadius : 1.0f;
        mesh->vertices_ = vertices;
        mesh->indices_.assign(indices.begin(), indices.end());
        mesh->faceSpans_ = std::move(faceSpans);
        return mesh;
    }

    std::shared_ptr<Mesh> Mesh::CreateFromData(
        IDirect3DDevice9* device,
        const std::vector<Vertex>& vertices,
        const std::vector<DWORD>& indices,
        std::vector<MeshFaceSpan> faceSpans,
        float boundingRadius)
    {
        if (vertices.empty() || indices.empty())
        {
            return nullptr;
        }

        std::shared_ptr<Mesh> mesh(new Mesh());
        const UINT vertexBufferSize = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        HRESULT hr = device->CreateVertexBuffer(vertexBufferSize, 0, Vertex::FVF, D3DPOOL_MANAGED, &mesh->vertexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* vertexMemory = nullptr;
        hr = mesh->vertexBuffer_->Lock(0, vertexBufferSize, &vertexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(vertexMemory, vertices.data(), vertexBufferSize);
        mesh->vertexBuffer_->Unlock();

        const UINT indexBufferSize = static_cast<UINT>(indices.size() * sizeof(DWORD));
        hr = device->CreateIndexBuffer(indexBufferSize, 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &mesh->indexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* indexMemory = nullptr;
        hr = mesh->indexBuffer_->Lock(0, indexBufferSize, &indexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(indexMemory, indices.data(), indexBufferSize);
        mesh->indexBuffer_->Unlock();

        mesh->indexFormat_ = D3DFMT_INDEX32;
        mesh->vertexCount_ = static_cast<UINT>(vertices.size());
        mesh->primitiveCount_ = static_cast<UINT>(indices.size() / 3);
        mesh->faceCount_ = static_cast<int>(faceSpans.size());
        mesh->boundingRadius_ = boundingRadius > 0.0f ? boundingRadius : 1.0f;
        mesh->vertices_ = vertices;
        mesh->indices_ = indices;
        mesh->faceSpans_ = std::move(faceSpans);
        return mesh;
    }

    void Mesh::Bind(IDirect3DDevice9* device) const
    {
        device->SetFVF(Vertex::FVF);
        device->SetStreamSource(0, vertexBuffer_, 0, sizeof(Vertex));
        device->SetIndices(indexBuffer_);
    }

    void Mesh::DrawAll(IDirect3DDevice9* device) const
    {
        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount_, 0, primitiveCount_);
    }

    void Mesh::DrawFace(IDirect3DDevice9* device, int faceIndex) const
    {
        if (faceIndex < 0 || faceIndex >= static_cast<int>(faceSpans_.size()))
        {
            return;
        }

        const MeshFaceSpan& span = faceSpans_[faceIndex];
        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount_, span.startIndex, span.primitiveCount);
    }

    bool Mesh::Raycast(const D3DXVECTOR3& localStart, const D3DXVECTOR3& localDirection, MeshRaycastHit& hit) const
    {
        hit = MeshRaycastHit{};
        if (vertices_.empty() || indices_.empty() || faceSpans_.empty())
        {
            return false;
        }

        bool foundHit = false;
        for (int faceIndex = 0; faceIndex < static_cast<int>(faceSpans_.size()); ++faceIndex)
        {
            const MeshFaceSpan& span = faceSpans_[faceIndex];
            for (UINT primitive = 0; primitive < span.primitiveCount; ++primitive)
            {
                const UINT startIndex = span.startIndex + primitive * 3;
                if (startIndex + 2 >= indices_.size())
                {
                    continue;
                }

                const DWORD index0 = indices_[startIndex];
                const DWORD index1 = indices_[startIndex + 1];
                const DWORD index2 = indices_[startIndex + 2];
                if (index0 >= vertices_.size() || index1 >= vertices_.size() || index2 >= vertices_.size())
                {
                    continue;
                }

                const D3DXVECTOR3 vertex0(vertices_[index0].x, vertices_[index0].y, vertices_[index0].z);
                const D3DXVECTOR3 vertex1(vertices_[index1].x, vertices_[index1].y, vertices_[index1].z);
                const D3DXVECTOR3 vertex2(vertices_[index2].x, vertices_[index2].y, vertices_[index2].z);

                float distance = 0.0f;
                D3DXVECTOR3 position;
                if (!IntersectTriangle(localStart, localDirection, vertex0, vertex1, vertex2, distance, position))
                {
                    continue;
                }

                if (distance >= hit.distance)
                {
                    continue;
                }

                const Vertex& closestVertex = ClosestTriangleVertex(vertices_, index0, index1, index2, position);
                hit.faceIndex = faceIndex;
                hit.distance = distance;
                hit.position = position;
                hit.vertexColor = closestVertex.color;
                foundHit = true;
            }
        }

        return foundHit;
    }
}
