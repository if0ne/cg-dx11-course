struct SCameraData
{
    float4x4 WorldViewProj;
};

struct SModelData
{
    float4x4 Transform;
};

struct SMaterial
{
    float4 BaseColor;
    float Reflection;
    float Absorption;
    float Shininess;
    float _padding;
};

// VS
cbuffer Camera : register(b0)
{
    SCameraData CameraData;
}

cbuffer Model : register(b1)
{
    SModelData ModelData;
}

// PS
cbuffer Material : register(b2)
{
    SMaterial Material;
};

Texture2D Texture : TEXTURE : register(t0);
Texture2D NormalMap : TEXTURE : register(t1);
SamplerState Sampler : SAMPLER : register(s0);

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float3 tangent : TANGENT;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
    float3 tangent : TANGENT;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    output.tex = input.tex;
    output.normal = normalize(mul(float4(input.normal, 0), ModelData.Transform).xyz);
    output.worldPos = mul(float4(input.pos, 1.0f), ModelData.Transform);
    output.tangent = normalize(mul(float4(input.tangent, 0), ModelData.Transform).xyz);
	
    return output;
}

float3 NormalSampleToWorldSpace(
    float3 normalMapSample,
    float3 unitNormalW,
    float3 tangentW)
{
    float3 normalT = 2.0f * normalMapSample - 1.0f;

    float3 N = unitNormalW;
    float3 T = normalize(tangentW.xyz - dot(tangentW.xyz, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

struct PixelShaderOutput
{
    float4 Diffuse : SV_Target0;
    float4 Normal : SV_Target1;
    float4 MatProp : SV_Target2;
    float4 WorldPos : SV_Target3;
};

[earlydepthstencil]
PixelShaderOutput PSMain(PS_IN input)
{
    PixelShaderOutput output;
    
    float3 normalMapSample = NormalMap.Sample(Sampler, input.tex).rgb;
    float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, input.normal, input.tangent);
    
    float3 surfaceColor = Material.BaseColor.xyz * Texture.Sample(Sampler, input.tex).xyz;

    output.Diffuse = float4(surfaceColor, 1.0f);
    output.Normal = float4(bumpedNormalW, 0.0);
    output.MatProp = float4(Material.Shininess, Material.Absorption, Material.Reflection, Material._padding);
    output.WorldPos = float4(input.worldPos, 1.0);
    
    return output;
}