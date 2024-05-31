struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
    float4 color;
};

struct ParticleIndexElement
{
    float distance;
    float index;
};

RWStructuredBuffer<Particle> particleList : register(u0);
AppendStructuredBuffer<uint> deadParticleIndex : register(u1);
RWStructuredBuffer<ParticleIndexElement> indexBuffer : register(u2);
RWBuffer<uint> indirectDrawArgs : register(u3);

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
    }

    GroupMemoryBarrierWithGroupSync();

    Particle particle = particleList[id.x];
	
    if (particle.age > 0.0f)
    {
        particle.age -= g_frameTime.x;

        float3 vNewPosition = particle.positon;

        vNewPosition = particle.positon + (particle.velocity * g_frameTime.x);
        
        particle.positon = vNewPosition;

        if (particle.age <= 0.0f)
        {
            particle.age = -1;
            particle.color = float4(1, 0, 0, 1);
            deadParticleIndex.Append(id.x);
        }
        else
        {
            float3 vec = mul(float4(vNewPosition, 1), View);
            uint index = indexBuffer.IncrementCounter();
            indexBuffer[index].distance = abs(vec.z);
            indexBuffer[index].index = (float) id.x;
			
            InterlockedAdd(indirectDrawArgs[0], 1);
        }
    }

    particleList[id.x] = particle;
}