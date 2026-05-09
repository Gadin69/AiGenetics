# Research: DirectX 12 Per-Entity World Transforms

## Problem Statement

Current architecture has a fundamental flaw where mesh vertices are manually offset during generation (baking position into vertex data), while the skeleton is generated separately in local space. This causes coordinate space mismatches because no world matrix is used during rendering. The proper solution requires keeping meshes in local space and applying world transforms at render time using per-entity constant buffers.

## Official Sources Found

### Source 1: Microsoft DirectX-Graphics-Samples - HelloConstBuffers
- **URL**: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloConstBuffers/D3D12HelloConstBuffers.cpp
- **Status**: Official Microsoft sample (6.7k stars)
- **Key Finding**: Demonstrates per-frame constant buffer updates using upload heap with descriptor table containing CBV
- **Solution Pattern**: 
  - Creates single CBV descriptor heap with `D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE`
  - Uses upload heap for constant buffer: `D3D12_HEAP_TYPE_UPLOAD`
  - Root signature with descriptor table: `rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX)`
  - Maps constant buffer once at initialization, updates per-frame with memcpy

### Source 2: Microsoft D3D12 Resource Binding Functional Spec
- **URL**: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
- **Status**: Official Microsoft specification
- **Key Finding**: Root signature supports three parameter types for per-object data:
  1. **Root Constants** (fastest, limited to 64 DWORDs total)
  2. **Root Descriptors** (single indirection, no bounds checking)
  3. **Descriptor Tables** (two indirections, bounds checking)
- **Recommendation**: For per-object transforms, use root constants or root CBVs for performance

### Source 3: NVIDIA Technical Blog - Advanced API Performance: Descriptors
- **URL**: https://developer.nvidia.com/blog/advanced-api-performance-descriptors/
- **Status**: NVIDIA official developer guidance (Oct 2023)
- **Key Finding**: Performance ranking confirmed:
  - **Root constants are the fastest** with no indirections, directly indexable
  - **Root CBV/SRV/UAV are second fastest** with single indirection and no bounds checking
  - **Descriptor tables are slowest** with two indirections and bounds checking
- **Recommendation**: "Use root (DirectX 12) or push (Vulkan) constants. They are the fastest way to transfer per-draw varying constants."

### Source 4: CosmicLearn - Direct3D 12 Binding Performance
- **URL**: https://www.cosmiclearn.com/dx12/binding-performance.php
- **Status**: Comprehensive D3D12 performance guide
- **Key Finding**: Root signature parameters must be organized by **frequency of update**:
  - Parameter 0: Per-frame data (camera, time) - changes once per frame
  - Parameter 1: Per-material data (textures) - changes per material pass
  - Parameter 2: Per-object data (world transform) - changes per draw call
- **Critical Quote**: "If you design your Root Signature such that the per-object data is at Parameter 0, the material data is at Parameter 1, and the per-frame camera data is at Parameter 2, you have created a performance disaster. Every time you draw a new object and update Parameter 0, the hardware will unnecessarily re-version the material and camera data sitting at the higher indices."
- **Code Example Provided**: Shows root signature with per-frame CBV at parameter 0, material descriptor table at parameter 1, and per-object root constants at parameter 2

### Source 5: Stack Overflow - Draw Multiple Meshes to Different Locations (D3D12)
- **URL**: https://stackoverflow.com/questions/39796989/draw-multiple-meshes-to-different-locations-directx-12
- **Status**: Highly relevant solved problem (upvoted answer)
- **Key Finding**: Three approaches for per-object transforms:
  1. **One constant buffer per object** - Simple but memory intensive
  2. **Large buffer with N CBVs** - Create single large buffer, create N CBVs pointing to different offsets (must respect 256-byte alignment)
  3. **StructuredBuffer with root constant index** - Store all matrices in structured buffer, pass index via `SetGraphicsRoot32BitConstant`
- **Solution Quote**: "You can also use a StructuredBuffer and copy all data into it (in that case you do not need the alignment), and use an index in the vertex shader to lookup the correct matrix. (it is possible to set a uint value in your shader and use SetGraphicsRoot32BitConstant to apply it directly)."

### Source 6: Microsoft DirectXTK12 Model Documentation
- **URL**: https://github.com/microsoft/DirectXTK12/wiki/Model
- **Status**: Official Microsoft toolkit documentation
- **Key Finding**: DirectXTK uses separate world, view, projection matrices per model instance:
  - `Model::UpdateEffectMatrices(modelEffects, world, view, projection)` - applies per-object world matrix
  - `shipEffect->SetMatrices(world, view, projection)` - sets matrices per effect instance
  - ModelBone data kept in **local space**, world transform applied during rendering
- **Architecture Pattern**: Mesh data in local space, world matrix passed at draw time, bones maintain local space relationship to mesh

### Source 7: CosmicLearn - Direct3D 12 Constant Buffers
- **URL**: https://www.cosmiclearn.com/dx12/constant-buffers.php
- **Status**: Comprehensive D3D12 tutorial
- **Key Finding**: Constant buffer alignment requirements:
  - **256-byte alignment mandatory** for CBV offsets
  - Upload heap with persistent mapping for per-frame updates
  - C++ and HLSL structure packing must match (16-byte boundaries)
- **Code Example**:
```cpp
UINT CalculateConstantBufferByteSize(UINT byteSize)
{
    return (byteSize + 255) & ~255;  // Round up to nearest 256
}
```

### Source 8: CosmicLearn - Direct3D 12 Upload Heaps
- **URL**: https://www.cosmiclearn.com/dx12/upload-heaps.php
- **Status**: Detailed memory architecture guide
- **Key Finding**: Upload heap best practices:
  - Create with `D3D12_RESOURCE_STATE_GENERIC_READ`
  - **Map once at initialization**, keep mapped for entire frame
  - Write-combined memory bypasses CPU cache (fast writes, slow reads)
  - Use ring buffer pattern for multiple frames (avoid GPU reading while CPU writes)

## Verified Solutions

### Solution A: Per-Object Root Constants (Highest Performance)
- **Source**: NVIDIA Blog + CosmicLearn Binding Performance
- **Effectiveness**: HIGH - Fastest binding method, no descriptor overhead
- **Complexity**: SIMPLE - Direct parameter passing
- **Code**:
```cpp
// Root signature parameter (last parameter to avoid re-versioning)
D3D12_ROOT_PARAMETER objectParam = {};
objectParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
objectParam.Constants.ShaderRegister = 1;
objectParam.Constants.Num32BitValues = 4;  // Can pass world matrix index or partial matrix
objectParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

// Per draw call:
commandList->SetGraphicsRoot32BitConstants(
    rootParamIndex, 
    4,  // Number of 32-bit values
    &worldMatrixIndex, 
    0
);
```
- **Limitation**: Limited to 64 DWORDs total in root signature, may need to pass matrix index instead of full matrix

### Solution B: Per-Object Constant Buffer with CBV (Standard Approach)
- **Source**: Microsoft HelloConstBuffers + Stack Overflow
- **Effectiveness**: HIGH - Standard pattern, well-documented
- **Complexity**: MODERATE - Requires CBV management
- **Code**:
```cpp
// Create upload heap large enough for all objects per frame
const UINT alignedSize = CalculateConstantBufferByteSize(sizeof(ObjectConstants));
const UINT totalSize = alignedSize * maxObjects;

D3D12_HEAP_PROPERTIES heapProps = {};
heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
// ... (standard upload heap creation)

// Map once at initialization
void* pMappedData = nullptr;
constantBuffer->Map(0, &readRange, &pMappedData);

// Per frame, per object:
UINT offset = objectIndex * alignedSize;
memcpy((BYTE*)pMappedData + offset, &objectData, sizeof(objectData));

// Create CBV for this object's offset
D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress() + offset;
cbvDesc.SizeInBytes = alignedSize;
device->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());

// Set CBV before draw
commandList->SetGraphicsRootDescriptorTable(cbvRootParamIndex, cbvGpuHandle);
commandList->DrawInstanced(vertexCount, 1, 0, 0);
```

### Solution C: Frame Resources Ring Buffer (Professional Engine Pattern)
- **Source**: Frank Luna's D3D12 book + Microsoft samples
- **Effectiveness**: HIGH - Used in production engines
- **Complexity**: MODERATE - Requires synchronization
- **Pattern**:
  - Allocate 2-3 frame resources (triple buffering)
  - Each frame resource contains upload heap for all per-object constants
  - Cycle through frame resources each frame
  - Use fence to ensure GPU finished with frame resource before CPU overwrites
- **Code Structure**:
```cpp
struct FrameResource
{
    ID3D12Resource* objectCB;  // Upload heap for all object constants
    UINT64 fence;              // Fence value for synchronization
};

FrameResource* currentFrame = frameResources[currentFrameIndex];

// Update object constants in current frame's upload heap
memcpy(mappedPtr + objectIndex * alignedSize, &data, sizeof(data));

// After N frames, GPU will have finished, safe to reuse
```

### Solution D: StructuredBuffer with Index (Flexible but Complex)
- **Source**: Stack Overflow answer
- **Effectiveness**: MEDIUM - No alignment issues, but requires shader changes
- **Complexity**: COMPLEX - Indirection in vertex shader
- **Code**:
```cpp
// Store all world matrices in structured buffer (no 256-byte alignment needed)
StructuredBuffer<float4x4> objectTransforms : register(t0);

// Pass index via root constant
commandList->SetGraphicsRoot32BitConstant(rootParamIndex, objectIndex, 0);

// Vertex shader:
uint objectIdx = rootConstant;
float4x4 worldMatrix = objectTransforms[objectIdx];
```

## HLSL Vertex Shader Patterns

### Pattern 1: Per-Object CBV (Most Common)
```hlsl
// Constant buffer for per-object data
cbuffer ObjectConstants : register(b0)
{
    float4x4 gWorld;
};

// Constant buffer for per-frame data
cbuffer PassConstants : register(b1)
{
    float4x4 gView;
    float4x4 gProjection;
};

struct VertexInput
{
    float3 Position : POSITION;
};

struct VertexOutput
{
    float4 PositionH : SV_POSITION;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    // Transform to world space
    float4 worldPos = mul(float4(input.Position, 1.0f), gWorld);
    
    // Transform to homogeneous clip space
    output.PositionH = mul(worldPos, mul(gView, gProjection));
    
    return output;
}
```

### Pattern 2: Root Constants for Matrix Index
```hlsl
// Per-frame constants
cbuffer PassConstants : register(b0)
{
    float4x4 gView;
    float4x4 gProjection;
};

// All object transforms in structured buffer
StructuredBuffer<float4x4> gObjectTransforms : register(t0);

// Root constant for object index
cbuffer RootConstants : register(b1)
{
    uint gObjectIndex;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    // Get world matrix from structured buffer
    float4x4 worldMatrix = gObjectTransforms[gObjectIndex];
    
    float4 worldPos = mul(float4(input.Position, 1.0f), worldMatrix);
    output.PositionH = mul(worldPos, mul(gView, gProjection));
    
    return output;
}
```

### Pattern 3: Combined World-View-Projection (Simplified)
```hlsl
cbuffer ObjectConstants : register(b0)
{
    float4x4 gWorldViewProj;
};

VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    output.PositionH = mul(float4(input.Position, 1.0f), gWorldViewProj);
    return output;
}
```
- **Note**: Must compute WVP on CPU each frame, less flexible for lighting in world space

## Performance Recommendations

Based on official sources, the recommended approach for a game engine with multiple creatures:

1. **Use frame resources ring buffer** (2-3 frames) to avoid CPU-GPU synchronization stalls
2. **Allocate single large upload heap per frame resource** containing all per-object constants
3. **Use CBVs with 256-byte aligned offsets** for each object's world matrix
4. **Organize root signature by update frequency**:
   - Parameter 0: Per-frame CBV (view, projection, time)
   - Parameter 1: Per-object CBV (world matrix)
   - Parameter 2: Material descriptor table (if needed)
5. **Keep meshes in local space** - never bake position into vertex data
6. **Keep skeleton/bones in local space** - apply world transform during debug visualization
7. **Map upload heap once at initialization**, never unmap until shutdown
8. **Use D3D12_SHADER_VISIBILITY_VERTEX** for world matrix CBV (pixel shader doesn't need it)

## Coordinate Space Architecture

### Local Space (Object Space)
- Mesh vertices stored relative to mesh origin (0,0,0)
- Skeleton bones positioned relative to mesh origin
- **Both mesh and skeleton share same local coordinate system**
- Created in modeling software or procedurally centered at origin

### World Space
- Applied during rendering via world matrix
- Transforms local space to world position
- **Same world matrix applied to both mesh vertices and bone positions**
- Computed from entity's position, rotation, scale

### View Space
- Camera transformation
- Applied after world space in vertex shader
- Shared across all entities (per-frame constant)

### Projection Space
- Lens transformation
- Applied after view space
- Shared across all entities (per-frame constant)

### Critical Rule
**Mesh and skeleton must both be in local space initially.** World matrix transforms both together, maintaining their spatial relationship. This eliminates the coordinate space mismatch in the current system.

## References

1. Microsoft DirectX-Graphics-Samples: https://github.com/microsoft/DirectX-Graphics-Samples
2. Microsoft D3D12 Resource Binding Spec: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
3. NVIDIA Advanced API Performance: https://developer.nvidia.com/blog/advanced-api-performance-descriptors/
4. Microsoft DirectXTK12 Model: https://github.com/microsoft/DirectXTK12/wiki/Model
5. CosmicLearn D3D12 Tutorials: https://www.cosmiclearn.com/dx12/
6. Stack Overflow D3D12 Multiple Meshes: https://stackoverflow.com/questions/39796989/draw-multiple-meshes-to-different-locations-directx-12
7. 3D Game Engine Programming D3D12: https://www.3dgep.com/learning-directx-12-2/
