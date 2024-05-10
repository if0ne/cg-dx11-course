struct Particle
{
    float3 positon;
    float3 velocity;
    float3 color;
    float rotation;
    float age;
    float radius;
    float maxLife;
    float distToEye;
};

struct EmitterProperties
{
    float3 position;
    float3 velocity;
    float3 positionVar;
    int maxNumToEmit;
    float particleLifeSpan;
    float particleInitRadius;
    float velocityPosVar;
    float minVelocity;
    float maxVelocity;
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

Texture2D g_RandomBuffer : register(t0);
SamplerState g_samWrapLinear : register(s0);

RWStructuredBuffer<Particle> g_ParticleBuffer : register(u0);

ConsumeStructuredBuffer<uint> g_DeadListToAllocFrom : register(u1);

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

        particle.color = float3(1.0f, 1.0f, 1.0f);
        particle.positon = emitterProp.position.xyz + (randomValues0.xyz * emitterProp.positionVar.xyz);
        particle.velocity = normalize(emitterProp.velocity.xyz + (randomValues1.xyz * emitterProp.velocityPosVar));
        particle.velocity *= clamp(abs(randomValues0.x) * emitterProp.maxVelocity, emitterProp.minVelocity, emitterProp.maxVelocity);
        particle.rotation = randomValues0.y;
        particle.age = emitterProp.particleLifeSpan;
        particle.radius = emitterProp.particleInitRadius;
        particle.maxLife = emitterProp.particleLifeSpan;

        uint index = g_DeadListToAllocFrom.Consume();

        g_ParticleBuffer[index] = particle;
    }
}