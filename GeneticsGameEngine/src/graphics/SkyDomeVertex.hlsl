// Sky Dome Vertex Shader
cbuffer CameraConstants : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

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

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Sky dome position (large sphere around camera)
    float4 worldPos = float4(input.position.xyz, 1.0f);
    output.worldPosition = worldPos.xyz;
    
    // Transform to clip space
    float4 viewPos = mul(viewMatrix, worldPos);
    output.position = mul(projectionMatrix, viewPos);
    
    return output;
}
