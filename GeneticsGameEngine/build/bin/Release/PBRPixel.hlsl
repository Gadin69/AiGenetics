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
    float2 texCoord : TEXCOORD;
};

float4 main(PSInput input) : SV_TARGET
{
    float3 N = normalize(input.normal);
    float3 V = normalize(cameraPosition - input.worldPos);
    
    // Ambient lighting
    float3 ambient = ambientColor * material.albedo * material.ambientOcclusion;
    
    // Calculate shadow factor
    float shadowFactor = 1.0;
    if (shadowEnabled > 0.5)
    {
        // Transform world position to light's clip space
        float4 worldPos = float4(input.worldPos, 1.0);
        float4 lightViewPos = mul(worldPos, lightViewMatrix);
        float4 lightClipPos = mul(lightViewPos, lightProjMatrix);
        
        // Normalize to [0, 1] range
        float3 shadowCoord = lightClipPos.xyz / lightClipPos.w;
        shadowCoord.xy = shadowCoord.xy * 0.5 + 0.5;
        shadowCoord.y = 1.0 - shadowCoord.y;  // Flip Y for texture coordinates
        
        // Sample shadow map
        if (shadowCoord.x >= 0.0 && shadowCoord.x <= 1.0 &&
            shadowCoord.y >= 0.0 && shadowCoord.y <= 1.0 &&
            shadowCoord.z >= 0.0 && shadowCoord.z <= 1.0)
        {
            float closestDepth = shadowMap.SampleCmpLevelZero(
                shadowSampler,
                shadowCoord.xy,
                shadowCoord.z - 0.005  // Small bias to prevent shadow acne
            );
            shadowFactor = closestDepth;
        }
    }
    
    // Calculate lighting from all lights
    float3 finalColor = ambient;
    
    for (int i = 0; i < lightCount && i < MAX_LIGHTS; i++)
    {
        float3 lightContrib = CalculateLighting(input.worldPos, N, material, lights[i], V);
        finalColor += lightContrib * shadowFactor;  // Apply shadow to direct lighting
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
