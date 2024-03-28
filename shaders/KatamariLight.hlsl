struct SCameraData
{
    float4x4 WorldViewProj;
};

struct SViewPos
{
    float4 ViewPos;
};

struct SModelData
{
    float4x4 Transform;
};

struct SDirectional
{
    float4 Direction;
    float4 Color;
};

struct SPoint
{
    float4 Position;
    float4 Color;
};

struct SMaterial
{
    float4 BaseColor;
    float Reflection;
    float Absorption;
    float Shininess;
    float _padding;
};

struct SAmbient
{
    float3 Color;
    float Intensity;
};

// VS
cbuffer Camera : register(b0)
{
    SCameraData CameraData;
}

cbuffer Model : register(b1)
{
    SModelData ModelData;
}

// PS
cbuffer Directional : register(b2)
{
    SDirectional Directional;
};

cbuffer Ambient : register(b3)
{
    SAmbient Ambient;
}

cbuffer Point : register(b4)
{
    SPoint Point;
};

cbuffer ViewPos : register(b7)
{
    SViewPos ViewPos;
};

cbuffer Material : register(b8)
{
    SMaterial Material;
};

Texture2D Texture : TEXTURE : register(t0);
SamplerState Sampler : SAMPLER : register(s0);

struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD;
};

PS_IN VSMain(VS_IN input)
{
    PS_IN output = (PS_IN) 0;
	
    output.pos = mul(float4(input.pos, 1.0f), mul(ModelData.Transform, CameraData.WorldViewProj));
    output.tex = input.tex;
    output.normal = normalize(mul(float4(input.normal, 0), ModelData.Transform).xyz);
    output.worldPos = mul(float4(input.pos, 1.0f), ModelData.Transform);
	
    return output;
}

float3 CalcDirLight(float3 normal, float3 viewDir)
{
    float3 lightDir = normalize(-Directional.Direction.xyz);

    float diffFactor = Directional.Color.w * Material.Reflection * max(dot(normal, lightDir), 0.0);
    float3 diff = diffFactor * Directional.Color.xyz;
    
    float3 reflectDir = normalize(reflect(-lightDir, normal));
    float specFactor = Material.Absorption * pow(max(dot(viewDir, reflectDir), 0.0), Material.Shininess);
    float3 spec = specFactor * Directional.Color.xyz;
    
    float3 amb = Ambient.Color * Ambient.Intensity;

    return diff + spec + amb;
}

float Attenuate(float distance, float radius, float max_intensity, float falloff)
{
    float s = distance / radius;

    if (s >= 1.0)
        return 0.0;

    float s2 = sqrt(s);

    return max_intensity * sqrt(1 - s2) / (1 + falloff * s);
}

float3 CalcPointLight(float3 normal, float3 fragPos, float3 viewDir)
{
    float3 lightDir = normalize(Point.Position.xyz - fragPos);

    float diffFactor = Point.Color.w * Material.Reflection * max(dot(normal, lightDir), 0.0);
    float3 diff = diffFactor * Point.Color.xyz;
     
    float3 reflectDir = normalize(reflect(-lightDir, normal));
    float specFactor = Material.Absorption * pow(max(dot(viewDir, reflectDir), 0.0), Material.Shininess);
    float3 spec = specFactor * Point.Color.xyz;
    
    float3 amb = Ambient.Color * Ambient.Intensity;
    
    float distance = length(Point.Position.xyz - fragPos);
    float attenuation = Attenuate(distance, Point.Position.w, Point.Color.w, 4.0);
    
    amb *= attenuation;
    diff *= attenuation;
    spec *= attenuation;
    return diff + spec + amb;
}

float4 PSMain(PS_IN input) : SV_Target
{
    float3 surfaceColor = Material.BaseColor.xyz * Texture.Sample(Sampler, input.tex).xyz;
    float3 dirLight = CalcDirLight(input.normal, normalize(ViewPos.ViewPos.xyz - input.worldPos));
    float3 pointLight = CalcPointLight(input.normal, input.worldPos, normalize(ViewPos.ViewPos.xyz - input.worldPos));
    float3 finalColor = surfaceColor * (dirLight + pointLight);
    return float4(finalColor, 1.0);
}