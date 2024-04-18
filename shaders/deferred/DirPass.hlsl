struct SDirectional
{
    float4 Direction;
    float4 Color;
};

struct SAmbient
{
    float3 Color;
    float Intensity;
};

struct SScreenSize
{
    float4 Size;
};


struct SCascadeData
{
    float4x4 ViewProj[4];
    float4 Distances;
};

struct SViewPos
{
    float4 Pos;
};

// PS
cbuffer Directional : register(b0)
{
    SDirectional Directional;
};

cbuffer Ambient : register(b1)
{
    SAmbient Ambient;
}

cbuffer CascadeData : register(b2)
{
    SCascadeData CascadeData;
}

cbuffer ScreenSize : register(b3)
{
    SScreenSize ScreenSize;
}

cbuffer ViewPos : register(b4)
{
    SViewPos ViewPos;
}

Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D MaterialMap : register(t2);
Texture2D WorldPosMap : register(t3);

Texture2DArray shadowmapT : register(t5);
SamplerComparisonState shadowmapS : register(s0);

struct VS_IN
{
    float3 pos : POSITION0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = float4(input.pos, 1.0);

    return output;
}

float3 CalcDirLight(float3 normal, float3 viewDir, float4 material)
{
    float3 lightDir = normalize(-Directional.Direction.xyz);

    float diffFactor = Directional.Color.w * material.z * max(dot(normal, lightDir), 0.0);
    float3 diff = diffFactor * Directional.Color.xyz;
    
    float3 reflectDir = normalize(reflect(-lightDir, normal));
    float specFactor = material.y * pow(max(dot(viewDir, reflectDir), 0.0), material.x);
    float3 spec = specFactor * Directional.Color.xyz;
    
    float3 amb = Ambient.Color * Ambient.Intensity;

    return diff + spec + amb;
}

float ShadowSample(float4 shadowCoord, uint cascadeIndex)
{
    float3 projCoords = shadowCoord.xyz / shadowCoord.w;
    
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = -projCoords.y * 0.5 + 0.5;

    float3 texCoord;
    texCoord.xy = projCoords.xy;
    texCoord.z = cascadeIndex;
    float sampled = shadowmapT.SampleCmp(shadowmapS, texCoord, projCoords.z);

    return sampled;
}

static float3 CascadeColors[4] =
{
    float3(1.0f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f),
    float3(1.0f, 0.2f, 0.2f),
};

float2 ScreenToView(float4 screen)
{
    return screen.xy / ScreenSize.Size.xy;
}

float4 PSMain(PS_IN input) : SV_Target
{
    int2 texCoord = input.pos.xy;
   
    float4 worldPos = WorldPosMap.Load(int3(texCoord, 0));
    float4 material = MaterialMap.Load(int3(texCoord, 0));
    float4 normal = NormalMap.Load(int3(texCoord, 0));
    float4 diffuse = DiffuseMap.Load(int3(texCoord, 0));
    
    float viewDir = length(ViewPos.Pos.xyz - worldPos.xyz);
    uint cascadeIndex = 0;
    for (uint i = 0; i < 4 - 1; ++i)
    {
        if (viewDir > CascadeData.Distances[i])
        {
            cascadeIndex = i + 1;
        }
    }

    float4 shadowCoord = mul(float4(worldPos.xyz, 1.0), CascadeData.ViewProj[cascadeIndex]);
    float shadow = ShadowSample(shadowCoord, cascadeIndex);
    
    float3 dirLight = CalcDirLight(normal.xyz, normalize(ViewPos.Pos.xyz - worldPos.xyz), material);
   
    float3 finalColor = diffuse.xyz * dirLight;
  
    if (material.w > 0.5)
    {
        return float4(diffuse.xyz * CascadeColors[cascadeIndex], 1.0);
    }
    
    return float4(finalColor, 1.0);
}