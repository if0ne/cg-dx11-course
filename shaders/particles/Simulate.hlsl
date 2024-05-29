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

RWBuffer<uint> indirectDrawArgs : register(u0);
AppendStructuredBuffer<ParticleIndexElement> aliveParticleIndex : register(u1);
AppendStructuredBuffer<uint> deadParticleIndex : register(u2);
RWStructuredBuffer<Particle> particleList : register(u3);
ConsumeStructuredBuffer<ParticleIndexElement> aliveParticleIndexIn : register(u4);
RWBuffer<uint> indirectDispatchArgs : register(u5);

cbuffer FrameTimeCB : register(b0)
{
    float4 g_frameTime;
};

cbuffer ViewProjectionCB : register(b3)
{
    float4x4 View;
    float4x4 Projection;
}

[numthreads(256, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if (id.x == 0)
    {
        indirectDrawArgs[0] = 0;
        indirectDrawArgs[1] = 1;
        indirectDrawArgs[2] = 0;
        indirectDrawArgs[3] = 0;
        indirectDrawArgs[4] = 0;

        indirectDispatchArgs[0] = 0;
        indirectDispatchArgs[1] = 1;
        indirectDispatchArgs[2] = 1;
    }

    GroupMemoryBarrierWithGroupSync();

    ParticleIndexElement index = aliveParticleIndexIn.Consume();
    Particle particle = particleList[index.index];
	
    if (particle.age > 0.0f)
    {
        particle.age -= g_frameTime.x;

        float3 vNewPosition = particle.positon;

        vNewPosition = particle.positon + (particle.velocity * g_frameTime.x);
        
        particle.positon = vNewPosition;

        if (particle.age <= 0.0f)
        {
            particle.age = -1;
            index.distance = 100000;
            deadParticleIndex.Append(index.index);
        }
        else
        {
            float3 vec = mul(float4(vNewPosition, 1), View);
            index.distance = abs(vec.z);
            aliveParticleIndex.Append(index);
			
            InterlockedAdd(indirectDrawArgs[0], 1);
            InterlockedAdd(indirectDispatchArgs[0], 1);
        }
    }

    particleList[index.index] = particle;
}