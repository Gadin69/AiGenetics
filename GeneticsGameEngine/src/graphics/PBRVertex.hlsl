// PBR Vertex Shader with proper lighting support
cbuffer CameraConstants : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL0;
    float4 baseColor : COLOR0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Transform to world space (identity matrix for now - creatures at world origin)
    float4 worldPos = float4(input.position.xyz, 1.0f);
    output.worldPosition = worldPos.xyz;
    
    // Transform to clip space
    float4 viewPos = mul(viewMatrix, worldPos);
    output.position = mul(projectionMatrix, viewPos);
    
    // Transform normal to world space (no rotation applied yet)
    output.worldNormal = normalize(input.normal);
    
    // Pass through base color (creature color from genetics)
    output.baseColor = input.color;
    
    return output;
}
