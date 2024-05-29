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

struct ParticleIndexElement
{
    float distance;
    float index;
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
    uint g_NumDeadParticles;
    uint3 DeadListCount_pad;
};

cbuffer AliveListCount : register(b3)
{
    uint g_NumAliveParticles;
    uint3 AliveListCount_pad;
};

Texture2D g_RandomBuffer : register(t0);
SamplerState g_samWrapLinear : register(s0);

RWStructuredBuffer<Particle> particleBuffer : register(u0);
ConsumeStructuredBuffer<uint> deadListBuffer : register(u1);
AppendStructuredBuffer<ParticleIndexElement> aliveParticleIndex : register(u2);
RWBuffer<uint> indirectDispatchArgs : register(u3);

[numthreads(1024, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x < g_NumDeadParticles && DTid.x < emitterProp.maxNumToEmit)
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
        
        ParticleIndexElement pe;
        pe.index = index;
        pe.distance = 0;
        aliveParticleIndex.Append(pe);

        InterlockedAdd(indirectDispatchArgs[0], 1);
    }
}