#include "Mesh.h"

#include "ComPtrUtils.h"

#include <cstring>

namespace Engine
{
    Mesh::~Mesh()
    {
        SafeRelease(indexBuffer_);
        SafeRelease(vertexBuffer_);
    }

    std::shared_ptr<Mesh> Mesh::CreateTexturedCube(IDirect3DDevice9* device)
    {
        const Vertex vertices[] =
        {
            { -1.0f,  1.0f, -1.0f, 0.0f, 0.0f },
            {  1.0f,  1.0f, -1.0f, 1.0f, 0.0f },
            {  1.0f, -1.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f, -1.0f, -1.0f, 0.0f, 1.0f },

            {  1.0f,  1.0f,  1.0f, 0.0f, 0.0f },
            { -1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
            { -1.0f, -1.0f,  1.0f, 1.0f, 1.0f },
            {  1.0f, -1.0f,  1.0f, 0.0f, 1.0f },

            { -1.0f,  1.0f,  1.0f, 0.0f, 0.0f },
            { -1.0f,  1.0f, -1.0f, 1.0f, 0.0f },
            { -1.0f, -1.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f, -1.0f,  1.0f, 0.0f, 1.0f },

            {  1.0f,  1.0f, -1.0f, 0.0f, 0.0f },
            {  1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
            {  1.0f, -1.0f,  1.0f, 1.0f, 1.0f },
            {  1.0f, -1.0f, -1.0f, 0.0f, 1.0f },

            { -1.0f,  1.0f,  1.0f, 0.0f, 0.0f },
            {  1.0f,  1.0f,  1.0f, 1.0f, 0.0f },
            {  1.0f,  1.0f, -1.0f, 1.0f, 1.0f },
            { -1.0f,  1.0f, -1.0f, 0.0f, 1.0f },

            { -1.0f, -1.0f, -1.0f, 0.0f, 0.0f },
            {  1.0f, -1.0f, -1.0f, 1.0f, 0.0f },
            {  1.0f, -1.0f,  1.0f, 1.0f, 1.0f },
            { -1.0f, -1.0f,  1.0f, 0.0f, 1.0f },
        };

        const WORD indices[] =
        {
             0,  1,  2,   0,  2,  3,
             4,  5,  6,   4,  6,  7,
             8,  9, 10,   8, 10, 11,
            12, 13, 14,  12, 14, 15,
            16, 17, 18,  16, 18, 19,
            20, 21, 22,  20, 22, 23,
        };

        std::shared_ptr<Mesh> mesh(new Mesh());
        HRESULT hr = device->CreateVertexBuffer(sizeof(vertices), 0, Vertex::FVF, D3DPOOL_MANAGED, &mesh->vertexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* vertexMemory = nullptr;
        hr = mesh->vertexBuffer_->Lock(0, sizeof(vertices), &vertexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(vertexMemory, vertices, sizeof(vertices));
        mesh->vertexBuffer_->Unlock();

        hr = device->CreateIndexBuffer(sizeof(indices), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &mesh->indexBuffer_, nullptr);
        if (FAILED(hr))
        {
            return nullptr;
        }

        void* indexMemory = nullptr;
        hr = mesh->indexBuffer_->Lock(0, sizeof(indices), &indexMemory, 0);
        if (FAILED(hr))
        {
            return nullptr;
        }
        memcpy(indexMemory, indices, sizeof(indices));
        mesh->indexBuffer_->Unlock();

        mesh->vertexCount_ = 24;
        mesh->primitiveCount_ = 12;
        mesh->faceCount_ = 6;
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
        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, vertexCount_, faceIndex * 6, 2);
    }
}
