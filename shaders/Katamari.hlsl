struct SCameraData
{
    float4x4 WorldViewProj;
};

struct SModelData
{
    float4x4 Transform;
};

cbuffer Camera : register(b0)
{
    SCameraData CameraData;
}

cbuffer Model : register(b1)
{
    SModelData ModelData;
}

struct VS_IN
{
    float3 pos : POSITION0;
    float2 tex : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD;
};

Texture2D Texture : TEXTURE : register(t0);
SamplerState Sampler : SAMPLER : register(s0);

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    output.tex = input.tex;
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 pixel_color = Texture.Sample(Sampler, input.tex);
    float4 col = float4(pixel_color, 1);
    return col;
}