// Vertex Shader - MINIMAL TEST

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
    
    // Pass position directly to clip space (assumes positions are already in [-1, 1] range)
    output.position = float4(input.position.xyz, 1.0f);
    output.color = input.color;
    
    return output;
}