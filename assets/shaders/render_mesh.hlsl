float4x4 WorldViewProjection;
sampler2D DiffuseTexture : register(s0);
float4 TintColor : register(c0);

struct VSInput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 position : POSITION0;
    float2 texCoord : TEXCOORD0;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, WorldViewProjection);
    output.texCoord = input.texCoord;
    return output;
}

float4 PS_Main(VSOutput input) : COLOR0
{
    return tex2D(DiffuseTexture, input.texCoord) * TintColor;
}

#ifndef SIGMA_RUNTIME_SHADER
technique RenderMeshTechnique
{
    pass RenderMeshPass
    {
        VertexShader = compile vs_2_0 VS_Main();
        PixelShader = compile ps_2_0 PS_Main();
    }
}
#endif
