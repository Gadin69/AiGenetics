# Phase 5: PBR Lighting and Materials System Implementation

## Overview
Implement a complete physically based rendering (PBR) system with HDR rendering, image-based lighting, procedural texture generation, and taxonomy-specific biological materials. The system will use the Cook-Torrance BRDF model with GGX normal distribution, map existing organism properties to PBR parameters, and support multiple tone mapping operators.

## Architecture Summary

The PBR system consists of four major subsystems:

**1. PBR Core** - Math and shader infrastructure
- Cook-Torrance BRDF implementation (HLSL)
- GGX normal distribution function
- Smith geometry function
- Schlick Fresnel approximation
- Multiple tone mapping operators (ACES Filmic, Reinhard, Uncharted2)

**2. HDR Rendering Pipeline** - High dynamic range rendering
- HDR render targets (FP16 format)
- Environment map generation (procedural skybox)
- Irradiance map generation (diffuse IBL)
- Prefiltered environment map (specular IBL with mipmaps)
- BRDF integration LUT (precomputed)
- Tone mapping post-process

**3. Procedural Texture System** - Runtime texture generation
- Procedural albedo texture generation from genetic colors
- Procedural normal map generation (perturbed surfaces)
- Procedural roughness/metallic maps from organism properties
- Texture caching to avoid regeneration

**4. Biological Materials** - Taxonomy-specific implementations
- SkinMaterial (Chordata): Based on m_skinRoughness, m_skinMetallic
- ExoskeletonMaterial (Arthropoda): Based on m_exoskeletonThickness, m_jointFlexibility
- ShellMaterial (Mollusca): Based on m_shellSpiralAngle, m_shellThickness, m_mantleTexture

## Implementation Tasks

### Task 1: Create PBR Material Core System
**Files**:
- `src/engine/rendering/materials/pbr/PBRMaterial.h`
- `src/engine/rendering/materials/pbr/PBRMaterial.cpp`
- `src/engine/rendering/materials/pbr/MaterialSystem.h`
- `src/engine/rendering/materials/pbr/MaterialSystem.cpp`
- `src/engine/rendering/materials/pbr/MaterialCache.h`
- `src/engine/rendering/materials/pbr/MaterialCache.cpp`

**Details**:
Create the core material infrastructure:

**PBRMaterial struct** - Represents a single PBR material:
```cpp
struct PBRMaterial {
    std::string materialID;
    DirectX::XMFLOAT3 albedo;        // Base color (RGB)
    float roughness;                  // Surface roughness (0-1)
    float metallic;                   // Metallic property (0-1)
    float ambientOcclusion;           // AO factor (0-1)
    float emissiveIntensity;          // Emissive strength
    
    // Optional textures
    Microsoft::WRL::ComPtr<ID3D12Resource> albedoTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> normalMap;
    Microsoft::WRL::ComPtr<ID3D12Resource> roughnessMetallicAO; // Packed RMA texture
    
    // DX12 resources
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
    PBRMaterialConstants* pConstantData;
};
```

**MaterialSystem** - Manages all materials:
- Create/destroy materials
- Bind materials to rendering pipeline
- Update material constant buffers
- Map organism properties to PBR parameters
- Support material sharing (hybrid approach)

**MaterialCache** - Cache for shared materials:
- Hash materials by parameters
- Reuse identical materials across creatures
- LRU eviction for memory management
- Track material usage statistics

**Genetic property mapping** (using existing organism properties):
```cpp
// Chordata -> Skin material
roughness = organism.GetSkinRoughness();      // 0.0-1.0
metallic = organism.GetSkinMetallic();        // 0.0-1.0
albedo = GetColorFromIndex(organism.GetColorIndex());

// Arthropoda -> Exoskeleton material
roughness = 1.0f - organism.GetJointFlexibility(); // Flexible = less rough
metallic = 0.3f + organism.GetExoskeletonThickness() * 0.4f; // Thick = more metallic
albedo = GetColorFromIndex(organism.GetColorIndex());

// Mollusca -> Shell material
roughness = 0.2f + (1.0f - organism.GetShellThickness()) * 0.5f;
metallic = 0.5f + organism.GetMantleTexture() * 0.3f; // Iridescent
albedo = GetColorFromIndex(organism.GetColorIndex());
```

### Task 2: Implement HDR Rendering Pipeline
**Files**:
- `src/engine/rendering/HDRRenderer.h`
- `src/engine/rendering/HDRRenderer.cpp`
- `src/engine/rendering/EnvironmentMap.h`
- `src/engine/rendering/EnvironmentMap.cpp`
- `src/engine/rendering/ToneMapping.h`
- `src/engine/rendering/ToneMapping.cpp`

**Details**:
Implement the HDR rendering infrastructure:

**HDRRenderer** - Manages HDR render targets:
- Create FP16 render targets for HDR color buffer
- Separate from LDR back buffer
- Render scene to HDR target first
- Apply tone mapping to convert to LDR for display

**HDR Pipeline Flow**:
1. Clear HDR render target
2. Render creatures with PBR shaders (HDR output)
3. Render environment/skybox (HDR)
4. Apply tone mapping (HDR → LDR)
5. Present to swap chain (LDR)

**EnvironmentMap** - Procedural skybox generation:
```cpp
class EnvironmentMap {
    // Generate procedural skybox
    bool GenerateProceduralSkybox(ID3D12Device* device, 
                                   ID3D12GraphicsCommandList* commandList);
    
    // Generate irradiance map (diffuse IBL)
    bool GenerateIrradianceMap(ID3D12Device* device);
    
    // Generate prefiltered environment map (specular IBL)
    bool GeneratePrefilteredMap(ID3D12Device* device);
    
    // Resources
    Microsoft::WRL::ComPtr<ID3D12Resource> skyboxCubeMap;        // 512x512x6
    Microsoft::WRL::ComPtr<ID3D12Resource> irradianceMap;        // 64x64x6
    Microsoft::WRL::ComPtr<ID3D12Resource> prefilteredMap;       // 256x256x6 with mipmaps
};
```

**Procedural Skybox Generation**:
- Create gradient sky (blue zenith → light horizon)
- Add procedural sun disk (bright spot for directional light)
- Optional: Add procedural clouds
- Store as cube map (6 faces)

**ToneMapping** - Multiple tone mapping operators:
```cpp
enum class ToneMappingOperator {
    ACESFilmic,    // Industry standard, cinematic
    Reinhard,      // Simple, good for testing
    Uncharted2,    // Uncharted 2 game operator
    Linear         // No tone mapping (debug)
};

class ToneMapping {
    void ApplyToneMapping(ID3D12GraphicsCommandList* commandList,
                         ToneMappingOperator op);
    
    // Set current operator
    void SetOperator(ToneMappingOperator op);
};
```

**Tone Mapping Shaders** (HLSL):
```hlsl
// ACES Filmic Tone Mapping
float3 ACESFilm(float3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

// Reinhard Tone Mapping
float3 Reinhard(float3 x) {
    return x / (1.0f + x);
}

// Uncharted 2 Tone Mapping
float3 Uncharted2Tonemap(float3 x) {
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
```

### Task 3: Create Procedural Texture Generation System
**Files**:
- `src/engine/rendering/materials/textures/ProceduralTexture.h`
- `src/engine/rendering/materials/textures/ProceduralTexture.cpp`
- `src/engine/rendering/materials/textures/TextureAtlas.h`
- `src/engine/rendering/materials/textures/TextureAtlas.cpp`

**Details**:
Implement runtime texture generation from genetic parameters:

**ProceduralTexture** - Generate textures algorithmically:
```cpp
class ProceduralTexture {
    // Generate albedo texture from genetic color
    bool GenerateAlbedoTexture(ID3D12Device* device,
                               DirectX::XMFLOAT3 baseColor,
                               int textureSize = 256);
    
    // Generate normal map with perturbed surface
    bool GenerateNormalMap(ID3D12Device* device,
                          float perturbationStrength = 0.1f,
                          int textureSize = 256);
    
    // Generate roughness/metallic/AO packed texture
    bool GenerateRoughnessMetallicAO(ID3D12Device* device,
                                     float roughness,
                                     float metallic,
                                     float ao = 1.0f,
                                     int textureSize = 256);
    
    // Procedural pattern generation
    void GenerateOrganicPattern(std::vector<uint8_t>& textureData,
                               int width, int height,
                               DirectX::XMFLOAT3 color1,
                               DirectX::XMFLOAT3 color2,
                               float noiseScale = 5.0f);
    
    // Simple noise function for organic patterns
    float SimplexNoise2D(float x, float y);
};
```

**Texture Generation Approach**:
1. **Albedo**: Base color from organism, add organic noise patterns for variation
2. **Normal Map**: Generate perturbed surface normals from noise function
3. **RMA Texture**: Pack roughness (R), metallic (G), AO (B) into single texture
4. **Texture Size**: Start with 256x256, can be adjusted based on LOD

**TextureAtlas** - Pack multiple textures into single atlas:
- Reduce texture binding overhead
- Share atlas across creatures with similar materials
- UV coordinate management for atlas regions

**Caching Strategy**:
- Cache generated textures by parameters (hash)
- Reuse textures for identical genetic profiles
- LRU eviction when memory limit reached

### Task 4: Implement Biological Material Types
**Files**:
- `src/engine/rendering/materials/biological/SkinMaterial.h`
- `src/engine/rendering/materials/biological/SkinMaterial.cpp`
- `src/engine/rendering/materials/biological/ExoskeletonMaterial.h`
- `src/engine/rendering/materials/biological/ExoskeletonMaterial.cpp`
- `src/engine/rendering/materials/biological/ShellMaterial.h`
- `src/engine/rendering/materials/biological/ShellMaterial.cpp`

**Details**:
Create taxonomy-specific material implementations:

**SkinMaterial** (Chordata):
- Maps from `Chordata::m_skinRoughness` and `Chordata::m_skinMetallic`
- Organic albedo patterns (veins, pigmentation variation)
- Moderate roughness (0.4-0.8 range typically)
- Low metallic (0.0-0.2, skin is dielectric)
- Subsurface scattering **SKIPPED** (per your decision)
- Generate subtle normal map for skin pores/wrinkles

```cpp
class SkinMaterial : public PBRMaterial {
    bool InitializeFromOrganism(const Taxonomy::Chordata* organism,
                                ID3D12Device* device,
                                ID3D12GraphicsCommandList* commandList);
    
private:
    void GenerateSkinAlbedoPattern(DirectX::XMFLOAT3 baseColor);
    void GenerateSkinNormalMap(float detailLevel);
};
```

**ExoskeletonMaterial** (Arthropoda):
- Maps from `Arthropoda::m_exoskeletonThickness` and `Arthropoda::m_jointFlexibility`
- Segmented appearance (visible joints/segments in normal map)
- Higher metallic than skin (0.2-0.6, chitin has some reflectivity)
- Roughness inversely related to joint flexibility
- Anisotropic reflections can be faked with stretched normal map noise
- Wear pattern textures (scratches, damage based on age/activity)

```cpp
class ExoskeletonMaterial : public PBRMaterial {
    bool InitializeFromOrganism(const Taxonomy::Arthropoda* organism,
                                ID3D12Device* device,
                                ID3D12GraphicsCommandList* commandList);
    
private:
    void GenerateExoskeletonPattern(DirectX::XMFLOAT3 baseColor,
                                   int segmentCount);
    void GenerateSegmentedNormalMap(float thickness);
    void GenerateWearPatterns(float age);
};
```

**ShellMaterial** (Mollusca):
- Maps from `Mollusca::m_shellSpiralAngle`, `m_shellThickness`, `m_mantleTexture`
- Iridescent appearance (color shifts based on viewing angle)
- Spiral pattern in albedo/normal maps
- Nacre (mother-of-pearl) effect via layered Fresnel
- Higher metallic (0.4-0.7) for iridescence
- Smooth surface with growth rings

```cpp
class ShellMaterial : public PBRMaterial {
    bool InitializeFromOrganism(const Taxonomy::Mollusca* organism,
                                ID3D12Device* device,
                                ID3D12GraphicsCommandList* commandList);
    
private:
    void GenerateSpiralPattern(DirectX::XMFLOAT3 baseColor,
                              float spiralAngle);
    void GenerateNacreNormalMap(float thickness);
    void GenerateGrowthRings(float age);
    
    // Iridescence parameters (passed to shader)
    float m_iridescenceIntensity;
    float m_iridescenceThickness;
};
```

### Task 5: Create PBR Shaders (HLSL)
**Files**:
- `src/shaders/PBRVertex.hlsl`
- `src/shaders/PBRPixel.hlsl`
- `src/shaders/ToneMapping.hlsl`
- `src/shaders/IBL.hlsl` (helper functions for IBL calculations)
- `src/shaders/Skybox.hlsl`

**Details**:
Implement complete PBR shader pipeline:

**PBRVertex.hlsl** - Vertex shader for PBR rendering:
```hlsl
struct VS_INPUT {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT {
    float4 position : SV_POSITION;
    float3 worldPosition : WORLDPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 tangent : TANGENT;  // For normal mapping (optional)
};

cbuffer MaterialConstants : register(b1) {
    float4 albedo;
    float roughness;
    float metallic;
    float ambientOcclusion;
    float emissiveIntensity;
};

cbuffer LightConstants : register(b2) {
    float4 lightDirection;   // xyz = direction, w = intensity
    float4 lightColor;
    float4 cameraPosition;
};

// Texture resources
Texture2D albedoTexture : register(t0);
Texture2D normalMap : register(t1);
Texture2D rmaTexture : register(t2); // Roughness, Metallic, AO packed

// IBL resources (precomputed)
TextureCube irradianceMap : register(t3);
TextureCube prefilteredMap : register(t4);
Texture2D brdfLUT : register(t5);
```

**PBRPixel.hlsl** - Fragment shader with Cook-Torrance BRDF:
```hlsl
// Cook-Torrance BRDF implementation
float3 CookTorranceBRDF(float3 N, float3 V, float3 L, float3 albedo, 
                        float roughness, float metallic) {
    float3 H = normalize(V + L);
    
    // Calculate Fresnel term (Schlick approximation)
    float3 F0 = lerp(0.04f, albedo, metallic); // Dielectric F0 = 0.04, metallic uses albedo
    float3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);
    
    // Calculate Normal Distribution Function (GGX)
    float NDF = DistributionGGX(N, H, roughness);
    
    // Calculate Geometry function (Smith)
    float G = GeometrySmith(N, V, L, roughness);
    
    // Calculate specular component
    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.001f;
    float3 specular = numerator / denominator;
    
    // Calculate diffuse component (energy conservation)
    float3 kD = (1.0f - F) * (1.0f - metallic); // Dielectric diffuse
    float3 diffuse = kD * albedo / PI;
    
    // Final color
    float NdotL = max(dot(N, L), 0.0f);
    return (diffuse + specular) * NdotL;
}

// Image-Based Lighting
float3 CalculateIBL(float3 N, float3 V, float3 albedo, 
                    float roughness, float metallic, float ao) {
    // Ambient diffuse (irradiance map)
    float3 F0 = lerp(0.04f, albedo, metallic);
    float3 irradiance = irradianceMap.Sample(samplerCube, N).rgb;
    float3 diffuse = irradiance * albedo;
    
    // Ambient specular (prefiltered map + BRDF LUT)
    float3 R = reflect(-V, N);
    float MAX_REFLECTION_LOD = 4.0f;
    float3 prefilteredColor = prefilteredMap.SampleLevel(samplerCube, R, 
                                                          roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdf = brdfLUT.Sample(samplerLinear, float2(max(dot(N, V), 0.0f), roughness)).rg;
    float3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);
    
    // Combine with ambient occlusion
    return (diffuse + specular) * ao;
}
```

**Required HLSL Functions**:
```hlsl
// GGX Normal Distribution Function
float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Schlick Fresnel approximation
float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

// Smith Geometry function
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    
    float num = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}
```

**ToneMapping.hlsl** - Post-process tone mapping:
```hlsl
cbuffer ToneMappingConstants : register(b0) {
    int toneMappingOperator; // 0=ACES, 1=Reinhard, 2=Uncharted2, 3=Linear
    float exposure;
};

Texture2D HDRInput : register(t0);

float4 main(PS_INPUT input) : SV_TARGET {
    float3 hdrColor = HDRInput.Sample(samplerLinear, input.uv).rgb;
    
    // Apply exposure
    hdrColor *= exposure;
    
    // Apply selected tone mapping operator
    float3 ldrColor;
    switch (toneMappingOperator) {
        case 0: ldrColor = ACESFilm(hdrColor); break;
        case 1: ldrColor = Reinhard(hdrColor); break;
        case 2: ldrColor = Uncharted2Tonemap(hdrColor) * 1.2f; break;
        default: ldrColor = hdrColor; break;
    }
    
    // Gamma correction
    ldrColor = pow(ldrColor, 1.0f / 2.2f);
    
    return float4(ldrColor, 1.0f);
}
```

### Task 6: Integrate PBR System into Rendering Pipeline
**Files**:
- `src/graphics/GraphicsEngine.h` (modify)
- `src/graphics/GraphicsEngine.cpp` (modify)
- `src/engine/procedural/mesh/ProceduralMeshRenderer.h` (modify)
- `src/engine/procedural/mesh/ProceduralMeshRenderer.cpp` (modify)
- `src/engine/CMakeLists.txt` (modify)
- `src/engine/rendering/materials/CMakeLists.txt` (create)

**Details**:
Integrate PBR system into existing rendering pipeline:

**GraphicsEngine Updates**:
1. Add HDR render target creation in `InitializeDX12()`:
```cpp
bool CreateHDRRenderTarget();
bool CreateEnvironmentMaps();
bool CreateToneMappingPipeline();
```

2. Modify render loop to use HDR pipeline:
```cpp
void Render(std::unique_ptr<GeneticsIntegration>& geneticsIntegration, 
            Engine::Rendering::BaseCameraController* camera) {
    // 1. Clear HDR render target
    // 2. Render creatures with PBR shader
    // 3. Render skybox/environment
    // 4. Apply tone mapping (HDR → LDR)
    // 5. Present to swap chain
}
```

3. Add PBR root signature and pipeline state:
```cpp
Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pbrRootSignature;
Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pbrPipelineState;
Microsoft::WRL::ComPtr<ID3D12PipelineState> m_toneMappingPipelineState;
```

**ProceduralMeshRenderer Updates**:
1. Add material support to mesh renderer:
```cpp
void SetMaterial(const PBRMaterial* material);
const PBRMaterial* GetMaterial() const { return m_currentMaterial; }
```

2. Update vertex structure to include UV coordinates:
```cpp
struct PBRVertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 uv;
};
```

**Material System Initialization**:
```cpp
// In Application::Initialize()
// After creating creatures
for (auto& organism : organisms) {
    auto material = MaterialSystem::CreateMaterialFromOrganism(organism);
    creatureMesh.SetMaterial(material);
}
```

**CMakeLists.txt Updates**:
- Add materials subdirectory to `src/engine/rendering/CMakeLists.txt`
- Create `src/engine/rendering/materials/CMakeLists.txt` with PBR, textures, biological modules
- Link Materials library to GraphicsEngine and main executable

### Task 7: Update GeneticsIntegration for Material Assignment
**Files**:
- `src/genetics/GeneticsIntegration.h` (modify)
- `src/genetics/GeneticsIntegration.cpp` (modify)

**Details**:
Add material creation and assignment during creature generation:

```cpp
// In GenerateCreatureMeshes()
for (size_t i = 0; i < m_organisms.size(); ++i) {
    // Generate mesh (existing code)
    CreatureMeshData meshData = GenerateMeshForOrganism(...);
    
    // Create PBR material based on organism taxonomy
    auto material = Engine::Materials::MaterialSystem::CreateMaterialFromOrganism(
        m_organisms[i].get(), device, commandList);
    
    // Assign material to mesh renderer
    meshData.meshRenderer->SetMaterial(material);
    meshData.material = material;
    
    m_creatureMeshes.push_back(std::move(meshData));
}
```

Add material storage to CreatureMeshData:
```cpp
struct CreatureMeshData {
    std::string creatureID;
    Engine::Procedural::Mesh::MeshData mesh;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    float scale;
    int currentLOD;
    
    std::unique_ptr<Engine::Procedural::Mesh::ProceduralMeshRenderer> meshRenderer;
    std::shared_ptr<Engine::Materials::PBRMaterial> material; // NEW
};
```

## File Structure

```
src/engine/rendering/materials/
├── CMakeLists.txt
├── pbr/
│   ├── PBRMaterial.h/cpp
│   ├── MaterialSystem.h/cpp
│   └── MaterialCache.h/cpp
├── textures/
│   ├── ProceduralTexture.h/cpp
│   └── TextureAtlas.h/cpp
└── biological/
    ├── SkinMaterial.h/cpp
    ├── ExoskeletonMaterial.h/cpp
    └── ShellMaterial.h/cpp

src/shaders/
├── PBRVertex.hlsl
├── PBRPixel.hlsl
├── ToneMapping.hlsl
├── IBL.hlsl
└── Skybox.hlsl

src/engine/rendering/
├── HDRRenderer.h/cpp
├── EnvironmentMap.h/cpp
└── ToneMapping.h/cpp
```

## Dependencies

**Existing Systems**:
- `Engine::Genetics::Taxonomy::Chordata/Arthropoda/Mollusca` - For material property mapping
- `Engine::Procedural::Mesh::ProceduralMeshRenderer` - For mesh rendering with materials
- `GraphicsEngine` - For DX12 pipeline integration

**DX12 Features Required**:
- Compute shaders (for environment map filtering)
- Cube map textures (for IBL)
- FP16 render targets (for HDR)
- Multiple descriptor heaps (for texture resources)

## Performance Considerations

1. **Material Caching**: Share identical materials across creatures to reduce GPU memory
2. **Texture Atlasing**: Pack multiple materials into single textures to reduce bindings
3. **LOD Textures**: Use smaller textures for distant creatures
4. **IBL Precomputation**: Generate irradiance/prefiltered maps once at startup
5. **Instanced Rendering**: Group creatures by material for batched rendering
6. **Async Texture Generation**: Generate textures on background thread to avoid frame stalls

## Testing Strategy

1. **Unit Tests**: Verify PBR math (BRDF functions, tone mapping operators)
2. **Visual Tests**: Render test spheres with known material parameters
3. **Integration Tests**: Verify creatures render with correct materials based on genetics
4. **Performance Tests**: Measure frame time impact of HDR + PBR vs. flat rendering
5. **IBL Validation**: Compare environment map reflections against reference images

## Notes

- Subsurface scattering intentionally **SKIPPED** per user decision
- Existing organism properties (m_skinRoughness, etc.) will be mapped to PBR parameters
- Procedural textures generated at runtime from genetic parameters
- Multiple tone mapping operators selectable at runtime (ACES Filmic, Reinhard, Uncharted2)
- Hybrid material storage: shared materials for identical genetics, unique for variations
- Full Cook-Torrance BRDF with GGX NDF, Smith geometry, Schlick Fresnel
