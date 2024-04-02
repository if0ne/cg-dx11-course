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
};

struct SModelData
{
    float4x4 Transform;
};

struct SCascadeData
{
    float4x4 ViewProj;
};

cbuffer CascadeData : register(b0)
{
    SCascadeData CascadeData;
}

cbuffer Model : register(b1)
{
    SModelData ModelData;
}

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(mul(float4(input.pos, 1.0), ModelData.Transform), CascadeData.ViewProj);
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return (0, 0, 0, 1);
}