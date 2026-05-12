float4x4 WorldViewProjection;
sampler2D DiffuseTexture : register(s0);
sampler2D NormalTexture : register(s1);
sampler2D BumpTexture : register(s2);
float4 TintColor : register(c0);
float4 MaterialParameters : register(c1);
float4 KeyLightDirectionAndIntensity : register(c9);
float4 FillLightDirectionAndIntensity : register(c10);
float4 AmbientLighting : register(c11);

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
    float3 normalSample = tex2D(NormalTexture, input.texCoord).xyz;
    float3 normal = dot(normalSample, normalSample) > 0.0001f
        ? DecodeNormal(normalSample)
        : float3(0.0f, 0.0f, 1.0f);
    float4 bump = tex2D(BumpTexture, input.texCoord);

    float3 keyLightDirection = normalize(KeyLightDirectionAndIntensity.xyz);
    float3 fillLightDirection = normalize(FillLightDirectionAndIntensity.xyz);
    float keyLight = saturate(dot(normal, keyLightDirection));
    float fillLight = saturate(dot(normal, fillLightDirection));
    float normalLight = saturate(normal.z * 0.55f + 0.45f);
    float cavity = dot(bump.rgb, float3(0.299f, 0.587f, 0.114f));
    float sheen = saturate(pow(cavity, 3.0f) + bump.a * 0.35f);
    float brightnessBoost = 1.0f + max(MaterialParameters.z, 0.0f);
    float sheenBoost = 1.0f + max(MaterialParameters.w, 0.0f);
    float warmFactor = saturate(AmbientLighting.y * 0.5f + 0.5f);
    float ambientIntensity = max(AmbientLighting.x, 0.0f);
    float keyIntensity = max(KeyLightDirectionAndIntensity.w, 0.0f);
    float fillIntensity = max(FillLightDirectionAndIntensity.w, 0.0f);
    float3 keyColor = lerp(float3(0.84f, 0.92f, 1.0f), float3(1.0f, 0.86f, 0.78f), warmFactor);
    float3 fillColor = lerp(float3(0.48f, 0.60f, 0.82f), float3(0.84f, 0.66f, 0.56f), warmFactor);
    float3 ambientLift = diffuse.rgb * lerp(float3(0.18f, 0.20f, 0.24f), float3(0.28f, 0.16f, 0.15f), warmFactor) * ambientIntensity;

    float3 litColor = diffuse.rgb * (0.20f + 0.32f * normalLight);
    litColor += ambientLift;
    litColor += diffuse.rgb * keyColor * keyLight * keyIntensity * 0.95f;
    litColor += diffuse.rgb * fillColor * fillLight * fillIntensity * 0.65f;
    litColor *= brightnessBoost;
    litColor += lerp(float3(0.20f, 0.07f, 0.07f), float3(0.28f, 0.11f, 0.09f), warmFactor) * sheen * sheenBoost;

    float useDiffuseAlpha = saturate(MaterialParameters.y);
    float baseAlpha = lerp(MaterialParameters.x, diffuse.a, useDiffuseAlpha);
    float alpha = saturate(baseAlpha * TintColor.a);
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
