struct SCameraData
{
    float4x4 WorldViewProj;
    float3 ViewPos;
};

struct SModelData
{
    float4x4 Transform;
};

struct SDirectional
{
    float4 Direction;
    float4 Color;
};

struct SPoint
{
    float3 Pos;
    float3 Color;
};

struct SMaterial
{
    float4 BaseColor;
    float Reflection;
    float Absorption;
    float Shininess;
    float _padding;
};

struct SAmbient
{
    float3 Color;
    float Intensity;
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
cbuffer Directional : register(b2)
{
    SDirectional Directional;
};

cbuffer Ambient : register(b3)
{
    SAmbient Ambient;
}

cbuffer Material : register(b8)
{
    SMaterial Material;
};

Texture2D Texture : TEXTURE : register(t0);
SamplerState Sampler : SAMPLER : register(s0);

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    output.tex = input.tex;
    output.normal = normalize(mul(float4(input.normal, 0), ModelData.Transform).xyz);
    output.worldPos = mul(float4(input.pos, 1.0f), ModelData.Transform);
	
    return output;
}

float3 CalcDirLight(float3 normal, float3 viewDir)
{
    float3 lightDir = normalize(-Directional.Direction.xyz);

    float diff = Directional.Color.w * Material.Reflection * max(dot(normal, lightDir), 0.0);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = Material.Absorption * pow(max(dot(viewDir, reflectDir), 0.0), Material.Shininess);
    
    float3 amb = Ambient.Color * Ambient.Intensity;

    return (diff + spec) * amb * Directional.Color.xyz;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 surfaceColor = Material.BaseColor.xyz * Texture.Sample(Sampler, input.tex);
    float3 dirLight = CalcDirLight(input.normal, CameraData.ViewPos);
    float3 finalColor = surfaceColor * dirLight;
    return float4(finalColor, 1.0);
}