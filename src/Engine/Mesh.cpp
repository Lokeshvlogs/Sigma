#include "Mesh.h"

#include "ComPtrUtils.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Engine
{
    namespace
    {
        void AppendMeshData(
            const aiMesh& sourceMesh,
            std::vector<Vertex>& vertices,
            std::vector<WORD>& indices,
            std::vector<MeshFaceSpan>& faceSpans,
            float& boundingRadius)
        {
            const UINT baseVertex = static_cast<UINT>(vertices.size());
            for (unsigned int vertexIndex = 0; vertexIndex < sourceMesh.mNumVertices; ++vertexIndex)
            {
                const aiVector3D& position = sourceMesh.mVertices[vertexIndex];
                aiVector3D uv(0.0f, 0.0f, 0.0f);
                if (sourceMesh.HasTextureCoords(0))
                {
                    uv = sourceMesh.mTextureCoords[0][vertexIndex];
                }

                vertices.push_back(
                    {
                        position.x,
                        position.y,
                        position.z,
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
                    indices.push_back(static_cast<WORD>(baseVertex + face.mIndices[indexIndex]));
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
        std::vector<WORD> indices;
        std::vector<MeshFaceSpan> faceSpans;
        float boundingRadius = 0.0f;

        for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            const aiMesh* sourceMesh = scene->mMeshes[meshIndex];
            if (!sourceMesh)
            {
                continue;
            }

            if (vertices.size() + sourceMesh->mNumVertices > 65535)
            {
                return nullptr;
            }

            AppendMeshData(*sourceMesh, vertices, indices, faceSpans, boundingRadius);
        }

        return CreateFromData(device, vertices, indices, faceSpans, boundingRadius);
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

        mesh->vertexCount_ = static_cast<UINT>(vertices.size());
        mesh->primitiveCount_ = static_cast<UINT>(indices.size() / 3);
        mesh->faceCount_ = static_cast<int>(faceSpans.size());
        mesh->boundingRadius_ = boundingRadius > 0.0f ? boundingRadius : 1.0f;
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
}
