float4 HighlightColor : register(c0);

float4 main(float2 texCoord : TEXCOORD0) : COLOR0
{
    return HighlightColor;
}
