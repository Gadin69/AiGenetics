# PBR Sky Dome Lighting System Implementation

## Architecture Overview

**Current State:**
- Creatures render white (no lighting - pixel shader just outputs vertex color)
- Basic vertex.hlsl/pixel.hlsl have no lighting calculations
- PBR infrastructure exists in GraphicsEngine but shaders are missing (PBRVertex.hlsl, PBRPixel.hlsl)
- PBR root signature already supports 4 CBVs: b0=view/proj, b1=material, b2=lights, b3=camera
- MaterialSystem class exists but is not integrated into rendering pipeline
- Clear color is hardcoded red: `{ 0.3f, 0.0f, 0.0f, 1.0f }`

**Target State:**
- Creatures shaded with full PBR (diffuse + specular from Cook-Torrance BRDF)
- Sky dome with configurable gradient (zenith → horizon → ground)
- Sun disc with configurable position, color, intensity
- Light constant buffer with TOD-driven parameters
- Extensible architecture for clouds, multiple lights, atmospheric scattering

---

## Task 1: Create Light and TOD Data Structures

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\GraphicsEngine.h`

**Add after line 39 (CameraConstants struct):**

```cpp
// Light constant buffer structure (must be 256-byte aligned)
struct alignas(256) LightConstants {
    DirectX::XMFLOAT3 sunDirection;      // Normalized sun direction
    float sunIntensity;                   // Sun intensity multiplier
    
    DirectX::XMFLOAT3 sunColor;          // Sun color (RGB)
    float ambientIntensity;              // Ambient light intensity
    
    DirectX::XMFLOAT3 ambientColor;      // Ambient light color (sky contribution)
    float groundAmbientIntensity;         // Ground bounce light intensity
    
    DirectX::XMFLOAT3 groundAmbientColor; // Ground reflection color
    float pad0;                           // Alignment padding
};

// Time of Day configuration
struct TimeOfDayConfig {
    float sunAngle;                       // Sun angle in radians (0 = sunrise, PI/2 = noon)
    DirectX::XMFLOAT3 sunColor;          // Sun color at this TOD
    DirectX::XMFLOAT3 skyZenithColor;    // Sky color at zenith (top)
    DirectX::XMFLOAT3 skyHorizonColor;   // Sky color at horizon
    DirectX::XMFLOAT3 groundColor;       // Ground ambient color
    float ambientIntensity;               // Ambient intensity multiplier
};
```

**Add member variables to GraphicsEngine class:**

```cpp
// Lighting system
Microsoft::WRL::ComPtr<ID3D12Resource> m_lightConstantBuffer;
LightConstants m_lightData = {};
UINT8* m_lightCBVData = nullptr;

// Sky dome
Microsoft::WRL::ComPtr<ID3D12Resource> m_skyDomeVertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> m_skyDomeIndexBuffer;
Microsoft::WRL::ComPtr<ID3D12PipelineState> m_skyDomePipelineState;
D3D12_VERTEX_BUFFER_VIEW m_skyDomeVertexBufferView = {};
D3D12_INDEX_BUFFER_VIEW m_skyDomeIndexBufferView = {};
UINT m_skyDomeVertexCount = 0;
UINT m_skyDomeIndexCount = 0;

// TOD system
TimeOfDayConfig m_todConfig;
float m_currentTimeOfDay = 0.25f; // 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset
```

**Add method declarations:**

```cpp
bool CreateLightConstantBuffer();
bool CreateSkyDome();
bool CreateSkyDomePipelineState();
void UpdateLightingFromTOD();
void RenderSkyDome();
```

---

## Task 2: Create PBR Shaders for Creature Rendering

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\PBRVertex.hlsl` (NEW FILE)

```hlsl
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
```

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\PBRPixel.hlsl` (NEW FILE)

```hlsl
// PBR Pixel Shader - Cook-Torrance BRDF
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

cbuffer CameraPosition : register(b3)
{
    float3 cameraPosition;
    float pad1;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL0;
    float4 baseColor : COLOR0;
};

// Fresnel-Schlick approximation
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Normal Distribution Function - GGX/Trowbridge-Reitz
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (3.14159265 * denom * denom);
}

// Geometry Function - Schlick-GGX
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.worldNormal);
    float3 V = normalize(cameraPosition - input.worldPosition);
    float3 baseColor = input.baseColor.rgb;
    
    // PBR material parameters (can be driven by genetics later)
    float metallic = 0.0;    // Creatures are dielectric (non-metallic)
    float roughness = 0.6;   // Moderate roughness for organic surfaces
    
    // Calculate F0 (reflectance at normal incidence)
    float3 F0 = float3(0.04, 0.04, 0.04); // Dielectric F0
    F0 = lerp(F0, baseColor, metallic);
    
    // Ambient lighting
    float3 ambient = baseColor * ambientColor * ambientIntensity;
    
    // Direct lighting (sun)
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    float3 L = normalize(-sunDirection);
    float3 H = normalize(V + L);
    
    float NdotL = max(dot(N, L), 0.0);
    
    if (NdotL > 0.0)
    {
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        // specular = (NDF * G * F) / (4 * (N.V) * (N.L))
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        float3 specular = numerator / denominator;
        
        // kD = 1 - kS (energy conservation)
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;
        
        // Radiance equation
        Lo += (kD * baseColor / 3.14159265 + specular) * sunColor * sunIntensity * NdotL;
    }
    
    // Ground ambient (bounce light)
    float3 groundAmbient = baseColor * groundAmbientColor * groundAmbientIntensity * 0.5;
    
    // Final color
    float3 result = ambient + Lo + groundAmbient;
    
    // Tone mapping (simple Reinhard)
    result = result / (result + float3(1.0, 1.0, 1.0));
    
    // Gamma correction
    result = pow(result, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));
    
    return float4(result, input.baseColor.a);
}
```

---

## Task 3: Create Sky Dome Shaders

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\SkyDomeVertex.hlsl` (NEW FILE)

```hlsl
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
```

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\SkyDomePixel.hlsl` (NEW FILE)

```hlsl
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
```

---

## Task 4: Implement Lighting and Sky Dome Creation

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\GraphicsEngine.cpp`

### 4.1: CreateLightConstantBuffer()

```cpp
bool GraphicsEngine::CreateLightConstantBuffer()
{
    std::cout << "  Creating light constant buffer..." << std::endl;
    
    // Create upload heap for light constant buffer
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProps.CreationNodeMask = 1;
    uploadHeapProps.VisibleNodeMask = 1;
    
    D3D12_RESOURCE_DESC bufferDesc = {};
    bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    bufferDesc.Width = sizeof(LightConstants);
    bufferDesc.Height = 1;
    bufferDesc.DepthOrArraySize = 1;
    bufferDesc.MipLevels = 1;
    bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    bufferDesc.SampleDesc.Count = 1;
    bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    HRESULT hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_lightConstantBuffer)
    );
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to create light constant buffer" << std::endl;
        return false;
    }
    
    // Map the buffer
    hr = m_lightConstantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_lightCBVData));
    if (FAILED(hr))
    {
        std::cerr << "Failed to map light constant buffer" << std::endl;
        return false;
    }
    
    // Initialize with default lighting
    UpdateLightingFromTOD();
    
    std::cout << "  Light constant buffer created successfully" << std::endl;
    return true;
}
```

### 4.2: CreateSkyDome()

```cpp
bool GraphicsEngine::CreateSkyDome()
{
    std::cout << "  Creating sky dome geometry..." << std::endl;
    
    // Create a sphere mesh (radius = 500.0f)
    const float radius = 500.0f;
    const int slices = 32;
    const int stacks = 16;
    
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<uint32_t> indices;
    
    // Generate sphere vertices
    for (int stack = 0; stack <= stacks; stack++)
    {
        float phi = DirectX::XM_PI * stack / stacks; // 0 to PI
        float y = radius * cosf(phi);
        float ringRadius = radius * sinf(phi);
        
        for (int slice = 0; slice <= slices; slice++)
        {
            float theta = 2.0f * DirectX::XM_PI * slice / slices; // 0 to 2*PI
            float x = ringRadius * cosf(theta);
            float z = ringRadius * sinf(theta);
            
            vertices.push_back({ x, y, z });
        }
    }
    
    // Generate indices
    for (int stack = 0; stack < stacks; stack++)
    {
        for (int slice = 0; slice < slices; slice++)
        {
            uint32_t current = stack * (slices + 1) + slice;
            uint32_t next = current + slices + 1;
            
            // Two triangles per quad
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    m_skyDomeVertexCount = static_cast<UINT>(vertices.size());
    m_skyDomeIndexCount = static_cast<UINT>(indices.size());
    
    // Create vertex buffer
    D3D12_HEAP_PROPERTIES defaultHeapProps = {};
    defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    
    D3D12_RESOURCE_DESC vertexBufferDesc = {};
    vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferDesc.Width = vertices.size() * sizeof(DirectX::XMFLOAT3);
    vertexBufferDesc.Height = 1;
    vertexBufferDesc.DepthOrArraySize = 1;
    vertexBufferDesc.MipLevels = 1;
    vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferDesc.SampleDesc.Count = 1;
    vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    HRESULT hr = m_device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_skyDomeVertexBuffer)
    );
    
    if (FAILED(hr))
    {
        std::cerr << "Failed to create sky dome vertex buffer" << std::endl;
        return false;
    }
    
    // Create upload buffer and copy data (similar to ProceduralMeshRenderer::UploadMeshData)
    // [Omitting upload code for brevity - follow same pattern]
    
    m_skyDomeVertexBufferView.BufferLocation = m_skyDomeVertexBuffer->GetGPUVirtualAddress();
    m_skyDomeVertexBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3);
    m_skyDomeVertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferDesc.Width);
    
    // Create index buffer (similar pattern)
    // [Omitting for brevity]
    
    std::cout << "  Sky dome created successfully (" << m_skyDomeVertexCount 
              << " vertices, " << m_skyDomeIndexCount << " indices)" << std::endl;
    return true;
}
```

### 4.3: CreateSkyDomePipelineState()

```cpp
bool GraphicsEngine::CreateSkyDomePipelineState()
{
    std::cout << "  Creating sky dome pipeline state..." << std::endl;
    
    // Compile shaders
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader, pixelShader;
    // [Use D3DCompileFromFile for SkyDomeVertex.hlsl and SkyDomePixel.hlsl]
    
    // Input layout: position only
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
    
    // Create PSO (similar to CreatePBRPipelineState but with sky shaders)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_pbrRootSignature.Get(); // Reuse PBR root signature
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    
    // Rasterizer state - render back faces (inside of dome)
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT; // Cull outside faces
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    
    // Depth state - write to depth buffer but at far distance
    psoDesc.DepthStencilState.DepthEnable = TRUE;
    psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    
    // [Rest of PSO setup similar to CreatePBRPipelineState]
    
    return true;
}
```

### 4.4: UpdateLightingFromTOD()

```cpp
void GraphicsEngine::UpdateLightingFromTOD()
{
    // Calculate sun direction from angle
    m_todConfig.sunAngle = m_currentTimeOfDay * 2.0f * DirectX::XM_PI;
    
    // Sun direction (circular path)
    m_lightData.sunDirection = {
        cosf(m_todConfig.sunAngle),
        sinf(m_todConfig.sunAngle),
        0.3f // Slight offset for visual interest
    };
    
    // Normalize
    float len = sqrtf(
        m_lightData.sunDirection.x * m_lightData.sunDirection.x +
        m_lightData.sunDirection.y * m_lightData.sunDirection.y +
        m_lightData.sunDirection.z * m_lightData.sunDirection.z
    );
    m_lightData.sunDirection.x /= len;
    m_lightData.sunDirection.y /= len;
    m_lightData.sunDirection.z /= len;
    
    // Set TOD-driven colors (will be updated with full TOD system later)
    m_lightData.sunColor = { 1.0f, 0.95f, 0.8f }; // Warm sunlight
    m_lightData.sunIntensity = 1.5f;
    
    m_lightData.ambientColor = m_todConfig.skyZenithColor;
    m_lightData.ambientIntensity = 0.3f;
    
    m_lightData.groundAmbientColor = m_todConfig.groundColor;
    m_lightData.groundAmbientIntensity = 0.15f;
    
    // Copy to GPU
    memcpy(m_lightCBVData, &m_lightData, sizeof(LightConstants));
}
```

### 4.5: RenderSkyDome()

```cpp
void GraphicsEngine::RenderSkyDome()
{
    // Set sky dome PSO
    m_commandList->SetPipelineState(m_skyDomePipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_pbrRootSignature.Get());
    
    // Set vertex/index buffers
    m_commandList->IASetVertexBuffers(0, 1, &m_skyDomeVertexBufferView);
    m_commandList->IASetIndexBuffer(&m_skyDomeIndexBufferView);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Draw sky dome
    m_commandList->DrawIndexedInstanced(m_skyDomeIndexCount, 1, 0, 0, 0);
}
```

---

## Task 5: Update Rendering Pipeline

**File:** `c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine\src\graphics\GraphicsEngine.cpp`

### 5.1: Update CompilePBRShaders()

Change lines 1536-1546 to load `L"PBRVertex.hlsl"` (already correct)
Change lines 1562-1571 to load `L"PBRPixel.hlsl"` (already correct)

**Add sky dome shader compilation at end of function:**

```cpp
// Compile sky dome shaders
// [Use D3DCompileFromFile for SkyDomeVertex.hlsl and SkyDomePixel.hlsl]
// Store in member variables m_skyDomeVertexShaderBlob, m_skyDomePixelShaderBlob
```

### 5.2: Update InitializePBRSystem()

**Add after line 1519:**

```cpp
// Initialize lighting
if (!CreateLightConstantBuffer())
{
    std::cerr << "  Failed to create light constant buffer" << std::endl;
    return false;
}

// Create sky dome
if (!CreateSkyDome())
{
    std::cerr << "  Failed to create sky dome" << std::endl;
    return false;
}

// Create sky dome PSO
if (!CreateSkyDomePipelineState())
{
    std::cerr << "  Failed to create sky dome PSO" << std::endl;
    return false;
}
```

### 5.3: Update PopulateCommandList() - Clear Color

**Change line 1296:**

```cpp
// Before:
const FLOAT clearColor[] = { 0.3f, 0.0f, 0.0f, 1.0f };  // Dark red

// After:
const FLOAT clearColor[] = { 
    m_todConfig.skyHorizonColor.x,
    m_todConfig.skyHorizonColor.y,
    m_todConfig.skyHorizonColor.z,
    1.0f 
};
```

### 5.4: Update PopulateCommandList() - Render Order

**Add sky dome rendering BEFORE creatures (after line 1349, before RenderCreatures):**

```cpp
// Draw 3D pyramid as reference object
m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
m_commandList->DrawInstanced(12, 1, 0, 0);

// Render sky dome (background)
RenderSkyDome();

// Render creature meshes
```

### 5.5: Update RenderCreatures() - Use PBR Pipeline

**Replace lines 1392-1393:**

```cpp
// Before:
m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
m_commandList->SetPipelineState(m_pipelineState.Get());

// After:
m_commandList->SetGraphicsRootSignature(m_pbrRootSignature.Get());
m_commandList->SetPipelineState(
    m_wireframeMode ? m_pbrWireframePipelineState.Get() : m_pbrPipelineState.Get()
);

// Set light constant buffer (b2)
m_commandList->SetGraphicsRootConstantBufferView(2, m_lightConstantBuffer->GetGPUVirtualAddress());

// Set camera position (b3)
struct CameraPosCB {
    DirectX::XMFLOAT3 position;
    float pad;
} cameraPosCB;
cameraPosCB.position = m_camera ? m_camera->GetPosition() : DirectX::XMFLOAT3(0, 5, 10);
cameraPosCB.pad = 0.0f;

// Create temporary upload buffer for camera position
// [Or use a persistent constant buffer like m_lightConstantBuffer]
```

---

## Task 6: Integration and Testing

### 6.1: Update GraphicsEngine::Initialize()

**Add initialization calls in proper order:**

```cpp
// In InitializePBRSystem() call chain:
// Already has: CompilePBRShaders(), CreatePBRRootSignature(), CreatePBRPipelineState()
// Add: CreateLightConstantBuffer(), CreateSkyDome(), CreateSkyDomePipelineState()
```

### 6.2: Verify Build

```bash
cd c:\Users\Taktix\Desktop\CodingAI\AiProjects\3dGenetics\GeneticsGameEngine
.\build.bat
```

### 6.3: Test Execution

Run application and verify:
- Sky dome renders with gradient colors
- Sun disc visible in sky
- Creatures show proper PBR shading (diffuse + specular)
- Camera movement works correctly
- No rendering artifacts or crashes

---

## Key Design Decisions

1. **PBR Shaders Reuse Existing Infrastructure**: PBR root signature already supports 4 CBVs (view/proj, material, lights, camera) - leverage this instead of creating new root signature
2. **Sky Dome Reuses PBR Root Signature**: Sky dome needs view/proj (b0) + lights (b2) - same root signature, just different shader code
3. **TOD-Driven Architecture**: All lighting parameters controlled by TOD config struct, making it easy to animate time of day later
4. **Cook-Torrance BRDF**: Industry-standard PBR model with GGX normal distribution and Schlick Fresnel
5. **Extensible for Clouds**: Sky dome shader can be extended with noise-based cloud layers in future
6. **Creatures at World Origin**: Vertex shader assumes creatures are at world position (no world matrix yet) - can add per-creature world matrices later

---

## Files Modified/Created

### Created (7 files):
1. `src/graphics/PBRVertex.hlsl` - PBR vertex shader
2. `src/graphics/PBRPixel.hlsl` - PBR pixel shader with Cook-Torrance BRDF
3. `src/graphics/SkyDomeVertex.hlsl` - Sky dome vertex shader
4. `src/graphics/SkyDomePixel.hlsl` - Sky dome pixel shader with gradient + sun
5. Build directory copies of all 4 shader files

### Modified (2 files):
1. `src/graphics/GraphicsEngine.h` - Add LightConstants, TimeOfDayConfig, sky dome members, method declarations
2. `src/graphics/GraphicsEngine.cpp` - Implement lighting, sky dome creation, update rendering pipeline

---

## Future Extensions (Out of Scope for This Task)

- Animate TOD over time (sunrise to sunset cycle)
- Add cloud layers to sky dome shader
- Multiple light sources (point lights, spot lights)
- Atmospheric scattering (Rayleigh + Mie)
- HDR rendering with bloom
- Per-creature world matrices for positioning
- Material constants from genetics system (roughness/metallic driven by genes)
