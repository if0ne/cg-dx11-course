struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
};

struct EmitterProperties
{
    float3 position;
    int maxNumToEmit;
    float particleLifeSpan;
};

cbuffer FrameTimeCB : register(b0)
{
    float4 g_frameTime;
};

cbuffer EmitterConstantBuffer : register(b1)
{
    EmitterProperties emitterProp;
};

cbuffer DeadListCount : register(b2)
{
    uint deadNumParticles;
    uint3 DeadListCount_pad;
};

Texture2D g_RandomBuffer : register(t0);
SamplerState g_samWrapLinear : register(s0);

RWStructuredBuffer<Particle> particleBuffer : register(u0);
ConsumeStructuredBuffer<uint> deadListBuffer : register(u1);

[numthreads(1024, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x < deadNumParticles && DTid.x < emitterProp.maxNumToEmit)
    {
        Particle particle = (Particle) 0;

        float2 uv = float2(DTid.x / 1024.0, g_frameTime.x * 1024);
        float3 randomValues0 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv, 0).xyz;

        float2 uv2 = float2((DTid.x + 1) / 1024.0, g_frameTime.x * 1024);
        float3 randomValues1 = g_RandomBuffer.SampleLevel(g_samWrapLinear, uv2, 0).xyz;

        particle.positon = emitterProp.position.xyz;
        particle.velocity = normalize(randomValues1.xyz);
        particle.age = emitterProp.particleLifeSpan;

        uint index = deadListBuffer.Consume();

        particleBuffer[index] = particle;
    }
}