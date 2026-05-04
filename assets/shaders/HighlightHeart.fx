float4x4 WorldViewProjection;
float4 HighlightColor : register(c0);
float4 HighlightParameters : register(c1);

struct VSInput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

struct VSOutput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

VSOutput VS_Main(VSInput input)
{
    VSOutput output;
    output.position = mul(input.position, WorldViewProjection);
    output.color = input.color;
    return output;
}

float4 PS_Main(VSOutput input) : COLOR0
{
    clip(HighlightParameters.y - abs(input.color.r - HighlightParameters.x));
    return HighlightColor;
}

#ifndef SIGMA_RUNTIME_SHADER
technique HighlightHeartTechnique
{
    pass HighlightHeartPass
    {
        VertexShader = compile vs_2_0 VS_Main();
        PixelShader = compile ps_2_0 PS_Main();
    }
}
#endif
