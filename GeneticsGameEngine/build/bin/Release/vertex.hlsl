// Vertex Shader

struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    // Transform position to clip space
    output.position = float4(input.position, 1.0f);
    
    // Pass through color
    output.color = input.color;
    
    return output;
}