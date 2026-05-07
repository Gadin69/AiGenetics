// PBR Vertex Shader

#include "PBRCommon.hlsl"

cbuffer VSConstants : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

PSInput main(VSInput input)
{
    PSInput output;
    
    // Use position directly as world position (identity world matrix)
    float4 worldPos = float4(input.position, 1.0);
    output.worldPos = worldPos.xyz;
    
    // Correct column-vector multiplication order for DirectXMath
    output.position = mul(projectionMatrix, mul(viewMatrix, worldPos));
    
    // Transform normal (identity world matrix, so no transformation needed)
    output.normal = input.normal;
    output.texCoord = input.texCoord;
    
    return output;
}
