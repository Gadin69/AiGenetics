// PBR Vertex Shader with per-entity world transforms
// ENTITY ARCHITECTURE: Meshes are in LOCAL SPACE, world matrix applied here

// Per-object constant buffer (register b1) - world matrix for this entity
cbuffer ObjectConstants : register(b1)
{
    float4x4 worldMatrix;
};

// Per-frame constant buffer (register b0) - camera view/projection
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
    
    // Transform from LOCAL SPACE to WORLD SPACE using per-object matrix
    // Use column-vector multiplication: mul(matrix, vector)
    float4 localPos = float4(input.position.xyz, 1.0f);
    float4 worldPos = mul(worldMatrix, localPos);
    output.worldPosition = worldPos.xyz;
    
    // Transform from WORLD SPACE to CLIP SPACE
    float4 viewPos = mul(viewMatrix, worldPos);
    output.position = mul(projectionMatrix, viewPos);
    
    // Transform normal to world space (apply world matrix rotation)
    // Use column-vector multiplication for normals too
    float4 localNormal = float4(input.normal, 0.0f);
    float4 worldNormal = mul(worldMatrix, localNormal);
    output.worldNormal = normalize(worldNormal.xyz);
    
    // Pass through base color (creature color from genetics)
    output.baseColor = input.color;
    
    return output;
}
