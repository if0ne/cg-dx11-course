struct SCameraData
{
    float4x4 WorldViewProj;
    float3 ViewPos;
};

struct SModelData
{
    float4x4 Transform;
};

struct Directional
{
    float4 Direction;
    float4 Color;
};

struct Point
{
    float3 Pos;
    float3 Color;
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
cbuffer DirCB : register(b2)
{
    Directional directional;
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
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    output.tex = input.tex;
    output.normal = normalize(mul(float4(input.normal, 0), ModelData.Transform).xyz);
	
    return output;
}

float3 CalcDirLight(float3 normal, float3 viewDir)
{
    float3 lightDir = normalize(-directional.Direction.xyz);

    float diff = max(dot(normal, lightDir), 0.0);

    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1.0);

    return directional.Color.w * (diff + spec) * directional.Color.xyz;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 surfaceColor = Texture.Sample(Sampler, input.tex);
    float3 dirLight = CalcDirLight(input.normal, CameraData.ViewPos);
    float3 finalColor = surfaceColor * dirLight;
    return float4(finalColor, 1.0);
}