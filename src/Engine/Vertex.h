#pragma once

#include <d3d9.h>

namespace Engine
{
    struct Vertex
    {
        float x;
        float y;
        float z;
        D3DCOLOR color;
        float u;
        float v;

        enum { FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 };
    };
}
