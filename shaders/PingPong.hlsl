struct Position
{
    float4 v;
    float4 aspect;
};

cbuffer ConstBuf : register(b0)
{
    Position Pos;
}

struct VS_IN
{
    float4 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    input.pos *= float4(1.0, 1.0, 1.0, 1.0);
    output.pos = input.pos + Pos.v;
	
    return output;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}