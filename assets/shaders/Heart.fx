float4x4 WorldViewProjection;
sampler2D DiffuseTexture : register(s0);
sampler2D NormalTexture : register(s1);
sampler2D BumpTexture : register(s2);
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

float3 DecodeNormal(float3 encodedNormal)
{
    return normalize(encodedNormal * 2.0f - 1.0f);
}

float4 PS_Main(VSOutput input) : COLOR0
{
    float4 diffuse = tex2D(DiffuseTexture, input.texCoord);
    float3 normal = DecodeNormal(tex2D(NormalTexture, input.texCoord).xyz);
    float4 bump = tex2D(BumpTexture, input.texCoord);

    float normalLight = saturate(normal.z * 0.75f + 0.25f);
    float cavity = dot(bump.rgb, float3(0.299f, 0.587f, 0.114f));
    float sheen = saturate(pow(cavity, 3.0f) + bump.a * 0.35f);

    float3 litColor = diffuse.rgb * (0.42f + 0.58f * normalLight);
    litColor += float3(0.18f, 0.03f, 0.03f) * sheen;

    float alpha = saturate(diffuse.a * TintColor.a);
    return float4(litColor * TintColor.rgb, alpha);
}

#ifndef SIGMA_RUNTIME_SHADER
technique HeartTechnique
{
    pass HeartPass
    {
        VertexShader = compile vs_2_0 VS_Main();
        PixelShader = compile ps_2_0 PS_Main();
    }
}
#endif
