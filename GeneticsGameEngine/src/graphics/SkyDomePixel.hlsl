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
    float3 zenithColor = ambientColor;           // From TOD config
    float3 horizonColor = ambientColor * 1.5;    // Lighter at horizon
    float3 groundColor = groundAmbientColor;     // Ground color
    
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
    float sunDot = dot(dir, -sunDirection);
    float sunDisc = smoothstep(0.998, 0.999, sunDot); // Sharp sun disc
    
    // Sun glow (larger area around sun)
    float sunGlow = pow(max(sunDot, 0.0), 64.0) * 0.3;
    
    // Combine sun effects
    skyColor += sunColor * sunIntensity * sunDisc;
    skyColor += sunColor * sunIntensity * sunGlow;
    
    return float4(skyColor, 1.0);
}
