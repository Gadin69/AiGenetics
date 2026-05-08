// Sky Dome Pixel Shader with gradient and sun disc
cbuffer LightConstants : register(b2)
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

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // Normalized direction from center
    float3 dir = normalize(input.worldPosition);
    
    // Height factor: -1 (bottom) to 1 (top)
    float height = dir.y;
    
    // Sky gradient colors
    float3 zenithColor = float3(0.2, 0.4, 0.9);           // Deep blue at top
    float3 horizonColor = float3(0.6, 0.75, 1.0);         // Light blue at horizon
    float3 groundColor = float3(0.3, 0.25, 0.2);          // Dark brown ground
    
    // Interpolate based on height
    float3 skyColor;
    if (height > 0.0)
    {
        // Sky: horizon to zenith
        skyColor = lerp(horizonColor, zenithColor, height);
    }
    else
    {
        // Ground: horizon to ground
        skyColor = lerp(horizonColor, groundColor, -height);
    }
    
    // Sun disc
    float sunDot = dot(dir, normalize(-sunDirection));
    float sunDisc = smoothstep(0.995, 0.998, sunDot); // Larger, more visible sun disc
    
    // Sun glow (larger area around sun)
    float sunGlow = pow(max(sunDot, 0.0), 32.0) * 0.5; // Brighter, wider glow
    
    // Combine sun effects
    skyColor += sunColor * sunIntensity * sunDisc;
    skyColor += sunColor * sunIntensity * sunGlow;
    
    return float4(skyColor, 1.0);
}
