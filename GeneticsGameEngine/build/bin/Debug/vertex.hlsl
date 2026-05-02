// Vertex Shader - Complete with camera transformation
cbuffer CameraConstants : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

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
    
    // Transform position: World -> View -> Projection -> Clip space
    // DirectXMath creates matrices for column-vector math: mul(matrix, vector)
    float4 worldPos = float4(input.position.xyz, 1.0f);
    float4 viewPos = mul(viewMatrix, worldPos);
    output.position = mul(projectionMatrix, viewPos);
    
    // Pass through color
    output.color = input.color;
    
    return output;
}
