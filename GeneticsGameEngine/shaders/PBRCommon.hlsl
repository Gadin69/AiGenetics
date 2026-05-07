// PBR Common Shader Functions
// Shared utilities for PBR rendering

#ifndef PBR_COMMON_HLSL
#define PBR_COMMON_HLSL

// Constants
#define PI 3.14159265359f
#define MAX_LIGHTS 16

// PBR Material structure (matches C++ PBRMaterialConstants)
struct PBRMaterial
{
    float3 albedo;
    float roughness;
    float metallic;
    float ambientOcclusion;
    float emissiveIntensity;
    float3 emissive;
    float padding;
};

// Light structure
struct Light
{
    float3 position;
    float type; // 0=directional, 1=point, 2=spot
    float3 direction;
    float radius;
    float3 color;
    float intensity;
};

// PBR BRDF Functions

// Normal Distribution Function - GGX/Trowbridge-Reitz
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry Function - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

// Geometry Function - Smith
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel Function - Schlick
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Calculate F0 based on metallic value
float3 CalculateF0(float metallic, float3 albedo)
{
    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    return F0;
}

// Cook-Torrance BRDF
float3 CalculateCookTorranceBRDF(float3 N, float3 V, float3 L, float3 H,
                                  PBRMaterial material, float3 radiance)
{
    float NDF = DistributionGGX(N, H, material.roughness);
    float G = GeometrySmith(N, V, L, material.roughness);
    
    float3 F0 = CalculateF0(material.metallic, material.albedo);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = (float3(1.0, 1.0, 1.0) - kS) * (1.0 - material.metallic);
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * material.albedo / PI + specular) * radiance * NdotL;
}

// Calculate lighting for a single light
float3 CalculateLighting(float3 position, float3 normal, PBRMaterial material,
                          Light light, float3 viewDir)
{
    float3 N = normalize(normal);
    float3 V = normalize(viewDir);
    float3 L = normalize(light.position - position);
    float3 H = normalize(V + L);
    
    float distance = length(light.position - position);
    float attenuation = 1.0 / (distance * distance + 1.0);
    float3 radiance = light.color * light.intensity * attenuation;
    
    return CalculateCookTorranceBRDF(N, V, L, H, material, radiance);
}

// Tone Mapping Functions

// ACES Filmic Tone Mapping
float3 ACESFilmicToneMapping(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Reinhard Tone Mapping
float3 ReinhardToneMapping(float3 x)
{
    return x / (x + 1.0);
}

// Uncharted 2 Tone Mapping
float3 Uncharted2ToneMapping(float3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    float W = 11.2;
    
    float3 tonemapped = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    float3 white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    
    return tonemapped / white;
}

#endif // PBR_COMMON_HLSL
