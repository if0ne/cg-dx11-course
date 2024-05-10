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

RWStructuredBuffer<Particle> g_ParticleBuffer : register(u0);

AppendStructuredBuffer<uint> g_DeadListToAddTo : register(u1);

RWStructuredBuffer<float2> g_IndexBuffer : register(u2);

RWStructuredBuffer<float4> g_ViewSpacePositions : register(u3);

RWBuffer<uint> g_DrawArgs : register(u4);

cbuffer FrameTimeCB : register(b0)
{
    float4 g_frameTime;
};

cbuffer ViewProjectionCB : register(b4)
{
    float4x4 View;
    float4x4 Projection;
}

[numthreads(256, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if (id.x == 0)
    {
        g_DrawArgs[0] = 0;
        g_DrawArgs[1] = 1;
        g_DrawArgs[2] = 0;
        g_DrawArgs[3] = 0;
        g_DrawArgs[4] = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    Particle particle = g_ParticleBuffer[id.x];
	
    if (particle.age > 0.0f)
    {
        particle.age -= g_frameTime.x;

        float3 vNewPosition = particle.positon;

        float fScaledLife = 1.0 - saturate(particle.age / particle.maxLife);
		
        float radius = particle.radius;
		
        bool killParticle = false;

        vNewPosition = particle.positon + (particle.velocity * g_frameTime.x);
        
        particle.positon = vNewPosition;
        
        float3 vec = mul(float4(vNewPosition, 1), View);
        particle.distToEye = abs(vec.z);

        float4 viewSpacePositionAndRadius;

        viewSpacePositionAndRadius.xyz = mul(float4(vNewPosition, 1), View).xyz;
        viewSpacePositionAndRadius.w = radius;

        g_ViewSpacePositions[id.x] = viewSpacePositionAndRadius;

        if (particle.age <= 0.0f || killParticle)
        {
            particle.age = -1;
            particle.color = float3(1.0f, 0., 0.);
            particle.distToEye = 100000;
            g_DeadListToAddTo.Append(id.x);
        }
        else
        {
            uint index = g_IndexBuffer.IncrementCounter();
            g_IndexBuffer[index] = float2(particle.distToEye, (float) id.x);
			
            uint dstIdx = 0;
            InterlockedAdd(g_DrawArgs[0], 6, dstIdx);
        }
    }

    g_ParticleBuffer[id.x] = particle;
}