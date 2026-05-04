float4x4 WorldViewProjection;
float4 HighlightColor : register(c0);

struct VSInput
{
    float4 position : POSITION0;
};

struct VSOutput
{
    float4 position : POSITION0;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, WorldViewProjection);
    return output;
}

float4 PS_Main(VSOutput input) : COLOR0
{
    return HighlightColor;
}

#ifndef SIGMA_RUNTIME_SHADER
technique FaceHighlightTechnique
{
    pass FaceHighlightPass
    {
        VertexShader = compile vs_2_0 VS_Main();
        PixelShader = compile ps_2_0 PS_Main();
    }
}
#endif
