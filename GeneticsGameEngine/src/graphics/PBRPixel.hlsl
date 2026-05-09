// PBR Pixel Shader - Cook-Torrance BRDF
// Updated to match new root signature register layout

// Material constants moved to b2 to match new root signature
cbuffer MaterialConstants : register(b2)
{
    float3 materialAlbedo;
    float materialRoughness;
    
    float3 materialEmissive;
    float materialMetallic;
    
    float materialAmbientOcclusion;
    float padding1;
    float padding2;
    float padding3;
};

// Light constants moved to b3 to match new root signature
cbuffer LightConstants : register(b3)
{
    float3 sunDirection;
    float sunIntensity;
    
    float3 sunColor;
    float ambientIntensity;
    
    float3 ambientColor;
    float groundAmbientIntensity;
    
    float3 groundAmbientColor;
    float pad0;
};

// Camera position moved to b4 to match new root signature
cbuffer CameraPosition : register(b4)
{
    float3 cameraPosition;
    float pad1;
};

// Shadow map constants (light matrices) moved to b5 to match new root signature
cbuffer ShadowConstants : register(b5)
{
    float4x4 lightViewMatrix;
    float4x4 lightProjMatrix;
};

// Shadow map texture and sampler
Texture2D shadowMap : register(t0);
SamplerComparisonState shadowSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL0;
    float4 baseColor : COLOR0;
};

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function - GGX/Trowbridge-Reitz
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (3.14159265 * denom * denom);
}

// Geometry Function - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// Calculate shadow factor using PCF (Percentage Closer Filtering)
float CalculateShadow(float3 worldPos)
{
    // Transform world position to light's clip space
    float4 worldPos4 = float4(worldPos, 1.0);
    float4 lightViewPos = mul(lightViewMatrix, worldPos4);
    float4 lightClipPos = mul(lightProjMatrix, lightViewPos);
    
    // Perform perspective divide
    float3 lightNDC = lightClipPos.xyz / lightClipPos.w;
    
    // Check if pixel is in light's view
    if (lightNDC.x < -1.0 || lightNDC.x > 1.0 ||
        lightNDC.y < -1.0 || lightNDC.y > 1.0 ||
        lightNDC.z < 0.0 || lightNDC.z > 1.0)
    {
        return 1.0; // Outside light's view, no shadow
    }
    
    // Convert to texture coordinates
    float2 shadowTexCoords = lightNDC.xy * 0.5 + 0.5;
    shadowTexCoords.y = 1.0 - shadowTexCoords.y; // Flip Y for texture coords
    
    // Current depth from light's perspective
    float currentDepth = lightNDC.z;
    
    // PCF sampling (3x3 filter for soft shadows)
    float shadow = 0.0;
    float bias = 0.005; // Small bias to prevent acne
    float texelSize = 1.0 / 2048.0;
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 sampleCoords = shadowTexCoords + float2(x, y) * texelSize;
            float pcfDepth = shadowMap.SampleCmp(shadowSampler, sampleCoords, currentDepth - bias);
            shadow += pcfDepth;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.worldNormal);
    float3 V = normalize(cameraPosition - input.worldPosition);
    float3 baseColor = input.baseColor.rgb;
    
    // DEBUG: If normal is zero, output red to indicate the problem
    if (length(input.worldNormal) < 0.001)
    {
        return float4(1.0, 0.0, 0.0, 1.0); // Red = bad normals
    }
    
    // PBR material parameters (can be driven by genetics later)
    float metallic = 0.0;    // Creatures are dielectric (non-metallic)
    float roughness = 0.6;   // Moderate roughness for organic surfaces
    
    // Calculate F0 (reflectance at normal incidence)
    float3 F0 = float3(0.04, 0.04, 0.04); // Dielectric F0
    F0 = lerp(F0, baseColor, metallic);
    
    // Ambient lighting
    float3 ambient = baseColor * ambientColor * ambientIntensity;
    
    // Direct lighting (sun)
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    float3 L = normalize(-sunDirection);
    float3 H = normalize(V + L);
    
    float NdotL = max(dot(N, L), 0.0);
    
    // DEBUG: Output NdotL as grayscale to see if lighting is working
    // return float4(NdotL.xxx, 1.0);
    
    if (NdotL > 0.0)
    {
        // Calculate shadow factor
        float shadow = CalculateShadow(input.worldPosition);
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        // specular = (NDF * G * F) / (4 * (N.V) * (N.L))
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        float3 specular = numerator / denominator;
        
        // kD = 1 - kS (energy conservation)
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;
        
        // Radiance equation (apply shadow with minimum floor to prevent complete darkness)
        float shadowFactor = max(shadow, 0.3); // At least 30% light even in shadow
        Lo += (kD * baseColor / 3.14159265 + specular) * sunColor * sunIntensity * NdotL * shadowFactor;
    }
    
    // DEBUG: Output just the diffuse lighting to isolate the issue
    // return float4(Lo * baseColor, 1.0);
    
    // Ground ambient (bounce light)
    float3 groundAmbient = baseColor * groundAmbientColor * groundAmbientIntensity * 0.5;
    
    // Final color
    float3 result = ambient + Lo + groundAmbient;
    
    // Tone mapping (simple Reinhard)
    result = result / (result + float3(1.0, 1.0, 1.0));
    
    // Gamma correction
    result = pow(result, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));
    
    return float4(result, input.baseColor.a);
}
