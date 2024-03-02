struct ConstantData
{
    float4x4 WorldViewProj;
};

cbuffer ConstBuf : register(b0)
{
    ConstantData ConstData;
}

struct VS_IN
{
    float4 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos.xyz, 1.0f), ConstData.WorldViewProj);
    output.col = float4(1.0, 1.0, 1.0, 1.0);
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return input.col;
}