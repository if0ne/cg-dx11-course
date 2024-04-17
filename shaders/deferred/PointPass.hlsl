struct SPointLight
{
    float4 Position;
    float4 Color;
};

struct SViewPos
{
    float4 Pos;
};

struct SModelData
{
    float4x4 Transform;
};

struct SCameraData
{
    float4x4 WorldViewProj;
};

// VS
cbuffer ModelData : register(b0)
{
    SModelData ModelData;
};

cbuffer Camera : register(b1)
{
    SCameraData CameraData;
}

// PS
cbuffer PointLight : register(b2)
{
    SPointLight PointLight;
};

cbuffer ViewPos : register(b3)
{
    SViewPos ViewPos;
}

Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D MaterialMap : register(t2);
Texture2D WorldPosMap : register(t3);

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
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    
    return output;
}

float Attenuate(float distance, float radius, float max_intensity, float falloff)
{
    float s = distance / radius;

    if (s >= 1.0)
        return 0.0;

    float s2 = sqrt(s);

    return max_intensity * sqrt(1 - s2) / (1 + falloff * s);
}

float3 CalcPointLight(float3 normal, float3 fragPos, float3 viewDir, float4 material)
{
    float3 lightDir = normalize(PointLight.Position.xyz - fragPos);

    float diffFactor = PointLight.Color.w * material.z * max(dot(normal, lightDir), 0.0);
    float3 diff = diffFactor * PointLight.Color.xyz;
     
    float3 reflectDir = normalize(reflect(-lightDir, normal));
    float specFactor = material.y * pow(max(dot(viewDir, reflectDir), 0.0), material.x);
    float3 spec = specFactor * PointLight.Color.xyz;
    
    float distance = length(PointLight.Position.xyz - fragPos);
    float attenuation = Attenuate(distance, PointLight.Position.w, PointLight.Color.w, 4.0);
    
    diff *= attenuation;
    spec *= attenuation;
    return diff + spec;
}

float4 PSMain(PS_IN input) : SV_Target
{
    int2 texCoord = input.pos.xy;
   
    float4 worldPos = WorldPosMap.Load(int3(texCoord, 0));
    float4 material = MaterialMap.Load(int3(texCoord, 0));
    float4 normal = NormalMap.Load(int3(texCoord, 0));
    float4 diffuse = DiffuseMap.Load(int3(texCoord, 0));
    
    float viewDir = length(ViewPos.Pos.xyz - worldPos.xyz);

    float3 pointLight = CalcPointLight(normal.xyz, worldPos.xyz, viewDir, material);
    
    float3 finalColor = diffuse.xyz * pointLight;
    
    return float4(finalColor, 1.0);
}