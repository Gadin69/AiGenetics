// Shadow Map Vertex Shader - Depth only rendering
cbuffer LightConstants : register(b0)
{
    float4x4 lightViewMatrix;
    float4x4 lightProjMatrix;
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
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Transform position to light's clip space
    float4 worldPos = float4(input.position.xyz, 1.0f);
    float4 lightViewPos = mul(lightViewMatrix, worldPos);
    output.position = mul(lightProjMatrix, lightViewPos);
    
    return output;
}
