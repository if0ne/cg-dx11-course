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

Texture2D NormalMap : register(t1);
Texture2D MaterialMap : register(t2);

cbuffer FrameTimeCB : register(b0)
{
    float4 g_frameTime;
};

cbuffer ViewProjectionCB : register(b3)
{
    float4x4 View;
    float4x4 Projection;
    float4x4 InverseProjectionView;
    float4x4 ViewInv;
    float4x4 ProjInv;
}

float4 ClipToWorld(float4 clip)
{
    float4 view = mul(clip, InverseProjectionView);

    view = view / view.w;

    return view;
}

float4 ScreenToWorld(float4 screen)
{
    float2 texCoord = screen.xy / float2(1240.0, 720.0);

    float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

    return ClipToWorld(clip);
}

float LinearizeDepth(const float z)
{
    const float near = 0.1f;
    const float far = 300.0;
    return near * far / (far + z * (near - far));
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
     
        float4 viewPos = mul(float4(vNewPosition, 1.0f), View);
        float4 ndc = mul(mul(float4(vNewPosition, 1.0f), View), Projection);
        ndc.xyz /= ndc.w;
    
        if (ndc.x > -1.0 && ndc.x < 1.0 && ndc.y > -1.0 && ndc.y < 1.0)
        {
            float2 texCoord;
            texCoord.x = (ndc.x * 0.5 + 0.5) * 1240.0;
            texCoord.y = (-ndc.y * 0.5 + 0.5) * 720.0;
            float4 material = MaterialMap.Load(int3(texCoord, 0));
            float depth = LinearizeDepth(1.0 / material.y);
                
            if ((viewPos.z > depth - 0.1) && (viewPos.z < depth + 0.1))
            {
                float4 normal = NormalMap.Load(int3(texCoord, 0));
                normal = normalize(mul(-normal, ViewInv));
            
                float3 newVelocity = reflect(particle.velocity, normal.xyz);
                particle.velocity = newVelocity;
                vNewPosition = particle.positon + (particle.velocity * g_frameTime.x);
            }
            
            particle.positon = vNewPosition;
        }
        else
        {
            particle.positon = vNewPosition;
        }
        

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