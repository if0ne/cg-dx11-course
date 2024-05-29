struct Particle
{
    float3 positon;
    float _pad1;
    float3 velocity;
    float age;
};

AppendStructuredBuffer<uint> deadListBuffer : register(u0);
RWStructuredBuffer<Particle> particleList : register(u1);
RWBuffer<uint> indirectDispatchArgs : register(u2);

[numthreads(256, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    if (id.x == 0)
    {
        indirectDispatchArgs[0] = 0;
        indirectDispatchArgs[1] = 1;
        indirectDispatchArgs[2] = 1;
    }

    GroupMemoryBarrierWithGroupSync();

    deadListBuffer.Append(id.x);

    particleList[id.x] = (Particle) 0;

}