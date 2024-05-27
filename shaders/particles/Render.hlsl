/*struct Particle
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

struct PS_INPUT
{
    float4 ViewSpaceCentreAndRadius : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float3 ViewPos : TEXCOORD2;
    float4 Color : COLOR0;
    float4 Position : SV_POSITION;
};

cbuffer ActiveListCount : register(b0)
{
    uint g_NumActiveParticles;
    uint3 ActiveListCount_pad;
};

cbuffer ViewProjectionCB : register(b1)
{
    float4x4 View;
    float4x4 Projection;
}

StructuredBuffer<Particle> g_ParticleBufferA : register(t0);

StructuredBuffer<float4> g_ViewSpacePositions : register(t1);

StructuredBuffer<float2> g_SortedIndexBuffer : register(t2);

PS_INPUT VSMain(uint VertexId : SV_VertexID)
{
    PS_INPUT Output = (PS_INPUT) 0;

    uint particleIndex = VertexId / 4;

    uint cornerIndex = VertexId % 4;

    float xOffset = 0;

    const float2 offsets[4] =
    {
        float2(-1, 1),
        float2(-1, -1),
        float2(1, 1),
        float2(1, -1),
    };

    uint index = (uint) g_SortedIndexBuffer[g_NumActiveParticles - particleIndex - 1].y;
    Particle pa = g_ParticleBufferA[index];

    float4 ViewSpaceCentreAndRadius = g_ViewSpacePositions[index];

    float2 offset = offsets[cornerIndex];
    float2 uv = (offset + 1) * 0.5;

    float radius = ViewSpaceCentreAndRadius.w;
    float3 cameraFacingPos;

    {
        float s, c;
        sincos(pa.rotation, s, c);
        float2x2 rotation = { float2(c, -s), float2(s, c) };

        offset = mul(offset, rotation);

        cameraFacingPos = ViewSpaceCentreAndRadius.xyz;
        cameraFacingPos.xy += radius * offset;
    }

    Output.Position = mul(float4(cameraFacingPos, 1), Projection);

    float distAltph = clamp(pa.distToEye / 10, 0, 1);
    
    Output.TexCoord = uv;
    Output.Color = float4(pa.color, 1);
    Output.ViewSpaceCentreAndRadius = ViewSpaceCentreAndRadius;
    Output.ViewPos = cameraFacingPos;

    return Output;
}

Texture2D g_ParticleTexture : register(t0);

SamplerState g_samClampLinear : register(s0);

float4 PSMain(PS_INPUT In) : SV_TARGET
{
    float3 particleViewSpacePos = In.ViewSpaceCentreAndRadius.xyz;
    float particleRadius = In.ViewSpaceCentreAndRadius.w;
    
    float4 albedo = 1;
    
    albedo *= g_ParticleTexture.SampleLevel(g_samClampLinear, In.TexCoord, 0);

    float4 color = albedo * In.Color;

    return color;
}*/

struct Particle
{
    float3 Position;
    float3 Velocity;
};

StructuredBuffer<Particle> Particles : register(t0);

cbuffer Params : register(b1)
{
    float4x4 View;
    float4x4 Projection;
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

    Particle particle = Particles[input.VertexID];

    float4 worldPosition = float4(particle.Position, 1);
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