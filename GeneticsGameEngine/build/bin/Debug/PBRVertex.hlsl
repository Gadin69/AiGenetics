// PBR Vertex Shader

#include "PBRCommon.hlsl"

cbuffer VSConstants : register(b0)
{
    matrix worldMatrix;
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
    
    float4 worldPos = mul(float4(input.position, 1.0), worldMatrix);
    output.worldPos = worldPos.xyz;
    output.position = mul(mul(worldPos, viewMatrix), projectionMatrix);
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.texCoord = input.texCoord;
    
    return output;
}
