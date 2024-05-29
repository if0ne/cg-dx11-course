struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
};

struct ParticleIndexElement
{
    float distance;
    float index;
};

StructuredBuffer<Particle> Particles : register(t3);
StructuredBuffer<ParticleIndexElement> aliveParticlesIndex : register(t4);

cbuffer Params : register(b1)
{
    float4x4 View;
    float4x4 Projection;
};

cbuffer aliveListCountConstantBuffer : register(b3)
{
    uint nbAliveParticles;
    uint3 aliveListPadding;
};

struct VertexInput
{
    uint VertexID : SV_VertexID;
};

struct PixelInput
{
    float4 Position : SV_POSITION;
    float2 UV: TEXCOORD0;
};

struct PixelOutput
{
    float4 Color : SV_TARGET0;
};

PixelInput VSMain(VertexInput input)
{
    PixelInput output = (PixelInput) 0;

    uint index = aliveParticlesIndex[nbAliveParticles - input.VertexID - 1].index;
    
    Particle particle = Particles[index];

    float4 worldPosition = float4(particle.positon, 1);
    float4 viewPosition = mul(worldPosition, View);
    output.Position = viewPosition;
    output.UV = 0;

    return output;
}

Texture2D particleTexture : register(t0);

SamplerState samClampLinear : register(s0);

PixelOutput PSMain(PixelInput input)
{
    PixelOutput output = (PixelOutput) 0;

    float3 particle = particleTexture.Sample(samClampLinear, input.UV).xyz;
    output.Color = float4(particle, 1);
	
    return output;
}

PixelInput _offsetNprojected(PixelInput data, float2 offset, float2 uv)
{
    data.Position.xy += offset;
    data.Position = mul(data.Position, Projection);
    data.UV = uv;

    return data;
}

[maxvertexcount(4)]
void GSMain(point PixelInput input[1], inout TriangleStream<PixelInput> stream)
{
    PixelInput pointOut = input[0];
	
    const float size = 0.1f;

    stream.Append(_offsetNprojected(pointOut, float2(-1, -1) * size, float2(0, 0)));
    stream.Append(_offsetNprojected(pointOut, float2(-1, 1) * size, float2(0, 1)));
    stream.Append(_offsetNprojected(pointOut, float2(1, -1) * size, float2(1, 0)));
    stream.Append(_offsetNprojected(pointOut, float2(1, 1) * size, float2(1, 1)));

    stream.RestartStrip();
}