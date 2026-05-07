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
};

cbuffer CameraConstants : register(b3)
{
    float3 cameraPosition;
    float padding;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 V = normalize(cameraPosition - input.worldPos);
    
    // Ambient lighting
    float3 ambient = ambientColor * material.albedo * material.ambientOcclusion;
    
    // Calculate lighting from all lights
    float3 finalColor = ambient;
    
    for (int i = 0; i < lightCount && i < MAX_LIGHTS; i++)
    {
        finalColor += CalculateLighting(input.worldPos, N, material, lights[i], V);
    }
    
    // Add emissive
    finalColor += material.emissive * material.emissiveIntensity;
    
    // Apply exposure
    finalColor *= exposure;
    
    // Apply ACES Filmic tone mapping
    finalColor = ACESFilmicToneMapping(finalColor);
    
    // Gamma correction
    finalColor = pow(finalColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(finalColor, 1.0);
}
