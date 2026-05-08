// PBR Pixel Shader

#include "PBRCommon.hlsl"

cbuffer PSConstants : register(b1)
{
    PBRMaterial material;
};

cbuffer LightConstants : register(b2)
{
    Light lights[MAX_LIGHTS];
    int lightCount;
    float3 ambientColor;
    float exposure;
    float4x4 lightViewMatrix;
    float4x4 lightProjMatrix;
    float shadowEnabled;
};

cbuffer CameraConstants : register(b3)
{
    float3 cameraPosition;
    float padding;
};

Texture2D shadowMap : register(t0);
SamplerComparisonState shadowSampler : register(s0);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 V = normalize(cameraPosition - input.worldPos);
    
    // Use vertex color as the base albedo
    float3 albedo = input.color.rgb;
    
    // Ambient lighting
    float3 ambientColor = float3(0.3, 0.5, 0.9);
    float ambientIntensity = 0.3;
    float3 ambient = ambientColor * albedo * ambientIntensity;
    
    // Directional sun lighting
    float3 sunDirection = normalize(float3(0.287348, -4.18679e-08, -0.957826));
    float3 sunColor = float3(1.0, 0.95, 0.8);
    float sunIntensity = 1.5;
    
    // Calculate diffuse (Lambertian)
    float NdotL = max(dot(N, -sunDirection), 0.0);
    float3 diffuse = albedo * sunColor * sunIntensity * NdotL;
    
    // Simple specular (Blinn-Phong)
    float3 H = normalize(-sunDirection + V);
    float NdotH = max(dot(N, H), 0.0);
    float specularIntensity = pow(NdotH, 32.0) * 0.5; // 32 = shininess, 0.5 = specular strength
    float3 specular = sunColor * specularIntensity;
    
    // Combine lighting
    float3 finalColor = ambient + diffuse + specular;
    
    // Apply gamma correction
    finalColor = pow(finalColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(finalColor, 1.0);
}
