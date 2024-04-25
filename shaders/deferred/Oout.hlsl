struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 uv : TexCoord;
};

VertexShaderOutput VSMain(uint vertexID : SV_VertexID)
{
    VertexShaderOutput result;
    result.uv = float2(vertexID & 2, (vertexID << 1) & 2);
    result.position = float4(result.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return result;
}

Texture2D AccumTexture : register(t0);
SamplerState sampl : register(s0);

float4 PSMain(VertexShaderOutput input) : SV_TARGET
{
    float4 color = AccumTexture.Sample(sampl, input.uv);
    
    color = pow(color, 1 / 2.2);

    return float4(color.xyz, 1.0);
}