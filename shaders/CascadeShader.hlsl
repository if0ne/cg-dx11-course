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

struct SCascadeData
{
    float4x4 ViewProj;
};

cbuffer CascadeData : register(b0)
{
    SCascadeData CascadeData;
}

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos.xyz, 1.0f), CascadeData.ViewProj);
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return (0, 0, 0, 1);
}