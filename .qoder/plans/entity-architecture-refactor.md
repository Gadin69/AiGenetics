# Fix Plan: Entity Architecture Refactor for Per-Entity World Transforms

## Root Cause

The current system bakes entity position directly into mesh vertex data during generation, while the skeleton is generated separately in local space without knowledge of the mesh offset. This creates a fundamental coordinate space mismatch:
- **Mesh vertices**: Offset in vertex buffer (world space baked in)
- **Skeleton bones**: Generated in local space (no offset applied)
- **No world matrix**: Rendering pipeline doesn't apply transforms at draw time

This architecture makes it impossible to correctly align skeleton debug visualization with the mesh, and prevents proper entity positioning, rotation, and scaling.

## Solution Approach

Based on official Microsoft samples, NVIDIA performance guidance, and established D3D12 engine architecture patterns, implement a **per-entity world transform system** using:

1. **Keep meshes in local space** (centered at origin)
2. **Keep skeleton in local space** (matching mesh coordinate system)
3. **Apply world matrix at render time** via per-object constant buffer
4. **Use frame resources ring buffer** for efficient per-frame updates
5. **Organize root signature by update frequency** for optimal performance

**Reference**: Microsoft DirectX-Graphics-Samples HelloConstBuffers, NVIDIA Advanced API Performance blog, DirectXTK12 Model architecture

---

## Step 1: Define Per-Object Constant Buffer Structure

### File: `src/RenderTypes.h` (or equivalent header)

**Change**: Add constant buffer structures for per-object and per-frame data

**Reference**: CosmicLearn D3D12 Constant Buffers + Microsoft HelloConstBuffers

**Code**:
```cpp
#include <DirectXMath.h>

// Per-object constant buffer data
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = DirectX::XMMatrixIdentity();
    // Padding to 256 bytes: XMFLOAT4X4 is 64 bytes, need 192 more bytes
    // Can add more per-object data here (material index, etc.)
};

// Per-frame constant buffer data
struct FrameConstants
{
    DirectX::XMFLOAT4X4 View;
    DirectX::XMFLOAT4X4 Projection;
    DirectX::XMFLOAT4X4 ViewProjection;
    // Add other per-frame data: camera position, time, light directions, etc.
};

// Utility function for 256-byte alignment (MANDATORY for D3D12 constant buffers)
inline UINT CalculateConstantBufferByteSize(UINT byteSize)
{
    // Rounds up to nearest multiple of 256
    return (byteSize + 255) & ~255;
}
```

**Verification**:
- Compile and verify structure sizes
- Confirm `sizeof(ObjectConstants)` is 64 bytes
- Confirm `CalculateConstantBufferByteSize(64)` returns 256

---

## Step 2: Create Frame Resources System

### File: `src/FrameResource.h` (new file)

**Change**: Create frame resource structure to manage per-frame upload heaps

**Reference**: Frank Luna D3D12 book pattern + Microsoft samples

**Code**:
```cpp
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "RenderTypes.h"

using Microsoft::WRL::ComPtr;

class FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT maxObjectCount);
    ~FrameResource();

    // Prevent copy
    FrameResource(const FrameResource&) = delete;
    FrameResource& operator=(const FrameResource&) = delete;

    // Upload heap for per-object constant buffers
    ComPtr<ID3D12Resource> ObjectCB;
    
    // Mapped pointer (keep mapped for entire lifetime)
    UINT8* ObjectCBDataBegin = nullptr;
    
    // Fence value for synchronization
    UINT64 FenceValue = 0;

private:
    UINT mMaxObjectCount;
    UINT mObjectCBByteSize; // 256-byte aligned size
};
```

### File: `src/FrameResource.cpp` (new file)

**Code**:
```cpp
#include "FrameResource.h"
#include <stdexcept>

FrameResource::FrameResource(ID3D12Device* device, UINT maxObjectCount)
    : mMaxObjectCount(maxObjectCount)
{
    // Calculate aligned size per object
    mObjectCBByteSize = CalculateConstantBufferByteSize(sizeof(ObjectConstants));
    
    // Total buffer size for all objects
    const UINT totalSize = mObjectCBByteSize * maxObjectCount;

    // Create upload heap (CPU writes, GPU reads)
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = totalSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // MUST be GENERIC_READ for upload heap
        nullptr,
        IID_PPV_ARGS(&ObjectCB)
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create frame resource upload heap");
    }

    // Map once at initialization, never unmap until destruction
    D3D12_RANGE readRange = {}; // Empty range = we won't read from CPU
    hr = ObjectCB->Map(0, &readRange, reinterpret_cast<void**>(&ObjectCBDataBegin));
    
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to map frame resource upload heap");
    }
}

FrameResource::~FrameResource()
{
    if (ObjectCB)
    {
        ObjectCB->Unmap(0, nullptr);
    }
}
```

**Verification**:
- Create test FrameResource, verify ObjectCB is created successfully
- Verify ObjectCBDataBegin pointer is valid (not nullptr)
- Check debug layer for any validation errors

---

## Step 3: Update Root Signature for Per-Object Transforms

### File: `src/Renderer.cpp` or `src/PipelineState.cpp` (wherever root signature is created)

**Change**: Modify root signature to include per-object CBV parameter

**Reference**: NVIDIA performance guidance + CosmicLearn binding performance article
- **Critical**: Organize by update frequency (per-frame first, per-object last)

**Code**:
```cpp
#include <d3dx12.h>

void CreateEntityRootSignature(ID3D12Device* device, ID3D12RootSignature** rootSignature)
{
    // Check for highest supported root signature version
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Define root parameters BY UPDATE FREQUENCY (most important for performance!)
    
    // Parameter 0: Per-frame constants (changes ONCE per frame)
    CD3DX12_DESCRIPTOR_RANGE1 frameRange;
    frameRange.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1,                  // 1 CBV
        1,                  // register(b1) - per-frame data
        0,                  // register space 0
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC // Won't change during frame
    );

    CD3DX12_ROOT_PARAMETER1 rootParameters[2];
    rootParameters[0].InitAsDescriptorTable(
        1, &frameRange, 
        D3D12_SHADER_VISIBILITY_ALL // View/Projection needed by all shaders
    );

    // Parameter 1: Per-object constants (changes EVERY draw call)
    CD3DX12_DESCRIPTOR_RANGE1 objectRange;
    objectRange.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
        1,                  // 1 CBV
        0,                  // register(b0) - per-object data
        0,                  // register space 0
        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE // Changes per draw
    );

    rootParameters[1].InitAsDescriptorTable(
        1, &objectRange,
        D3D12_SHADER_VISIBILITY_VERTEX // Only vertex shader needs world matrix
    );

    // Create root signature descriptor
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1(
        _countof(rootParameters),
        rootParameters,
        0,                  // No static samplers
        nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );

    // Serialize and create
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &rootSignatureDesc,
        featureData.HighestVersion,
        &signature,
        &error
    );

    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA(static_cast<const char*>(error->GetBufferPointer()));
        }
        throw std::runtime_error("Failed to serialize root signature");
    }

    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(rootSignature)
    );

    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create root signature");
    }
}
```

**Verification**:
- Use PIX or RenderDoc to inspect root signature layout
- Verify parameter 0 is per-frame CBV, parameter 1 is per-object CBV
- Check debug layer for root signature validation errors

---

## Step 4: Update HLSL Vertex Shader

### File: `shaders/EntityVertexShader.hlsl` (modify existing or create new)

**Change**: Update vertex shader to apply world matrix from per-object CBV

**Reference**: Microsoft HelloConstBuffers + standard D3D12 transformation pattern

**Code**:
```hlsl
// Per-object constant buffer (register b0)
cbuffer ObjectConstants : register(b0)
{
    float4x4 gWorld;
};

// Per-frame constant buffer (register b1)
cbuffer FrameConstants : register(b1)
{
    float4x4 gView;
    float4x4 gProjection;
    float4x4 gViewProjection;
};

// Vertex shader input (local space positions!)
struct VertexInput
{
    float3 Position : POSITION;
    // Add other attributes: NORMAL, TEXCOORD, etc.
};

// Vertex shader output
struct VertexOutput
{
    float4 PositionH : SV_POSITION;
    // Add other outputs for pixel shader
};

// Main vertex shader
VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    
    // Transform from local space to world space
    float4 worldPos = mul(float4(input.Position, 1.0f), gWorld);
    
    // Transform from world space to homogeneous clip space
    output.PositionH = mul(worldPos, gViewProjection);
    
    return output;
}
```

**Critical Changes from Current System**:
- **REMOVE** any manual vertex offset in shader
- **REMOVE** any baked-in position from vertex buffer
- Input positions MUST be in local space (centered at origin)

**Verification**:
- Compile shader with `fxc` or `dxc`
- Verify no compilation errors
- Test with simple cube at origin to confirm local space rendering

---

## Step 5: Create Per-Object CBV Management

### File: `src/EntityRenderer.h` or equivalent

**Change**: Add CBV descriptor heap and management for per-object transforms

**Code**:
```cpp
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;

class EntityRenderer
{
public:
    EntityRenderer(ID3D12Device* device, UINT maxEntities, UINT numFrameResources);
    ~EntityRenderer();

    // Update entity world transform (call before recording command list)
    void UpdateEntityTransform(UINT entityIndex, UINT frameIndex, const DirectX::XMFLOAT4X4& worldMatrix);
    
    // Get CBV GPU handle for entity (use when recording draw call)
    D3D12_GPU_DESCRIPTOR_HANDLE GetEntityCBV(UINT entityIndex, UINT frameIndex) const;
    
    // Get CBV descriptor heap (bind once per frame)
    ID3D12DescriptorHeap* GetCBVHeap() const { return mCBVHeap.Get(); }

private:
    void CreateCBVDescriptorHeap(ID3D12Device* device, UINT maxEntities);

    ComPtr<ID3D12DescriptorHeap> mCBVHeap;
    UINT mMaxEntities;
    UINT mNumFrameResources;
    UINT mCBVDescriptorSize;
    UINT mObjectCBByteSize; // 256-byte aligned
    
    // Store CBV handles for quick lookup
    std::vector<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> mEntityCBVs;
};
```

### File: `src/EntityRenderer.cpp`

**Code**:
```cpp
#include "EntityRenderer.h"
#include <d3dx12.h>
#include <stdexcept>

EntityRenderer::EntityRenderer(ID3D12Device* device, UINT maxEntities, UINT numFrameResources)
    : mMaxEntities(maxEntities),
      mNumFrameResources(numFrameResources),
      mObjectCBByteSize(CalculateConstantBufferByteSize(sizeof(ObjectConstants)))
{
    CreateCBVDescriptorHeap(device, maxEntities);
}

EntityRenderer::~EntityRenderer()
{
}

void EntityRenderer::CreateCBVDescriptorHeap(ID3D12Device* device, UINT maxEntities)
{
    // Create descriptor heap for per-object CBVs
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heapDesc.NumDescriptors = maxEntities;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    heapDesc.NodeMask = 0;

    HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mCBVHeap));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create CBV descriptor heap");
    }

    mCBVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Initialize CBV handle arrays
    mEntityCBVs.resize(mNumFrameResources);
    for (UINT frameIdx = 0; frameIdx < mNumFrameResources; ++frameIdx)
    {
        mEntityCBVs[frameIdx].resize(maxEntities);
    }
}

void EntityRenderer::UpdateEntityTransform(UINT entityIndex, UINT frameIndex, const DirectX::XMFLOAT4X4& worldMatrix)
{
    if (entityIndex >= mMaxEntities)
    {
        throw std::out_of_range("Entity index out of range");
    }

    // Calculate offset in upload heap
    const UINT offset = entityIndex * mObjectCBByteSize;

    // Copy world matrix to mapped upload heap
    // Note: FrameResource should provide access to its upload heap
    // This is pseudocode - adjust based on your FrameResource architecture
    UINT8* destinationData = GetFrameResource(frameIndex)->ObjectCBDataBegin + offset;
    
    ObjectConstants objConst;
    objConst.World = worldMatrix;
    
    memcpy(destinationData, &objConst, sizeof(ObjectConstants));

    // Create/update CBV descriptor
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
    cbvDesc.BufferLocation = GetFrameResource(frameIndex)->ObjectCB->GetGPUVirtualAddress() + offset;
    cbvDesc.SizeInBytes = mObjectCBByteSize;

    CD3DX12_CPU_DESCRIPTOR_HANDLE cbvHandle(
        mCBVHeap->GetCPUDescriptorHandleForHeapStart(),
        entityIndex,
        mCBVDescriptorSize
    );

    device->CreateConstantBufferView(&cbvDesc, cbvHandle);

    // Cache GPU handle for fast retrieval
    mEntityCBVs[frameIndex][entityIndex] = CD3DX12_GPU_DESCRIPTOR_HANDLE(
        mCBVHeap->GetGPUDescriptorHandleForHeapStart(),
        entityIndex,
        mCBVDescriptorSize
    );
}

D3D12_GPU_DESCRIPTOR_HANDLE EntityRenderer::GetEntityCBV(UINT entityIndex, UINT frameIndex) const
{
    if (entityIndex >= mMaxEntities)
    {
        throw std::out_of_range("Entity index out of range");
    }
    return mEntityCBVs[frameIndex][entityIndex];
}
```

**Verification**:
- Create test entities, verify CBVs are created without errors
- Use PIX to verify CBV buffer locations are correct (256-byte aligned)
- Check descriptor heap doesn't overflow

---

## Step 6: Implement Entity Component with World Transform

### File: `src/Entity.h` or equivalent

**Change**: Add entity component to store position, rotation, scale

**Code**:
```cpp
#pragma once
#include <DirectXMath.h>

struct TransformComponent
{
    // Local space position
    DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };
    
    // Rotation (Euler angles in radians)
    DirectX::XMFLOAT3 Rotation = { 0.0f, 0.0f, 0.0f };
    
    // Scale
    DirectX::XMFLOAT3 Scale = { 1.0f, 1.0f, 1.0f };

    // Compute world matrix from transform
    DirectX::XMMATRIX GetWorldMatrix() const
    {
        using namespace DirectX;
        
        // Create transformation matrices
        XMVECTOR scaleVec = XMLoadFloat3(&Scale);
        XMVECTOR rotationVec = XMLoadFloat3(&Rotation);
        XMVECTOR positionVec = XMLoadFloat3(&Position);
        
        // Order: Scale → Rotate → Translate
        XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scaleVec);
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVec);
        XMMATRIX translationMatrix = XMMatrixTranslationFromVector(positionVec);
        
        // Combine: World = Scale * Rotation * Translation
        return scaleMatrix * rotationMatrix * translationMatrix;
    }

    // Get as XMFLOAT4X4 for constant buffer
    DirectX::XMFLOAT4X4 GetWorldMatrixFloat4x4() const
    {
        XMMATRIX worldMatrix = GetWorldMatrix();
        XMFLOAT4X4 result;
        XMStoreFloat4x4(&result, worldMatrix);
        return result;
    }
};

struct Entity
{
    UINT ID;
    TransformComponent Transform;
    UINT MeshIndex;      // Which mesh to render
    UINT SkeletonIndex;  // Which skeleton to render (for debugging)
    bool IsActive;
};
```

**Verification**:
- Create entity at origin, verify world matrix is identity
- Create entity at (5, 0, 0), verify world matrix translation is correct
- Test rotation and scaling

---

## Step 7: Refactor Mesh Generation to Local Space

### File: `src/MeshGenerator.cpp` or equivalent

**Change**: **REMOVE** manual vertex offset during mesh generation

**Current Problem** (example of what to remove):
```cpp
// ❌ WRONG - Baking position into vertex data
void GenerateCreatureMesh(float offsetX, float offsetY, float offsetZ)
{
    for (auto& vertex : vertices)
    {
        vertex.Position.x += offsetX;  // DON'T DO THIS
        vertex.Position.y += offsetY;
        vertex.Position.z += offsetZ;
    }
}
```

**Correct Implementation**:
```cpp
// ✅ CORRECT - Keep mesh in local space (centered at origin)
void GenerateCreatureMesh()
{
    // Generate vertices centered at origin (0, 0, 0)
    // No manual offsetting!
    for (auto& vertex : vertices)
    {
        // vertex.Position stays as-is in local space
    }
}
```

**Verification**:
- Generate mesh, verify vertices are centered around origin
- Check bounding box is roughly symmetric around (0, 0, 0)
- Confirm no position offset in vertex generation code

---

## Step 8: Refactor Skeleton Generation to Match Mesh Local Space

### File: `src/SkeletonGenerator.cpp` or equivalent

**Change**: Ensure skeleton bones are in same local space as mesh

**Code**:
```cpp
// Skeleton bones should be positioned relative to mesh origin
struct Bone
{
    std::string Name;
    DirectX::XMFLOAT3 LocalPosition;  // Position in mesh local space
    DirectX::XMFLOAT4X4 LocalTransform; // Bone transform in local space
};

void GenerateCreatureSkeleton()
{
    // All bone positions relative to mesh origin (0, 0, 0)
    std::vector<Bone> bones;
    
    Bone rootBone;
    rootBone.Name = "Root";
    rootBone.LocalPosition = { 0.0f, 0.0f, 0.0f }; // Origin
    
    Bone headBone;
    headBone.Name = "Head";
    headBone.LocalPosition = { 0.0f, 1.5f, 0.0f }; // 1.5 units above root
    
    // ... etc
    
    // World transform will be applied during rendering/debug visualization
}
```

**Critical**: Skeleton and mesh MUST share the same local coordinate system. If mesh vertices range from -1 to +1, skeleton bones should be positioned within that same space.

**Verification**:
- Render skeleton in local space (no world transform)
- Render mesh in local space (no world transform)
- Verify skeleton aligns with mesh visually
- Apply same world transform to both, verify they move together

---

## Step 9: Update Render Loop to Apply Per-Entity Transforms

### File: `src/Renderer.cpp` or `src/RenderPass.cpp`

**Change**: Update render loop to set per-object CBV before each draw call

**Code**:
```cpp
void Renderer::RenderFrame(UINT frameIndex, const std::vector<Entity>& entities)
{
    FrameResource* currentFrame = mFrameResources[frameIndex].get();

    // Wait if GPU hasn't finished with this frame resource
    if (currentFrame->FenceValue > mLastCompletedFenceValue)
    {
        mFenceEvent->Reset();
        mFence->SetEventOnCompletion(currentFrame->FenceValue, mFenceEvent);
        WaitForSingleObject(mFenceEvent->GetHandle(), INFINITE);
        mLastCompletedFenceValue = currentFrame->FenceValue;
    }

    // Record command list
    mCommandAllocator->Reset();
    mCommandList->Reset(mCommandAllocator.Get(), mPipelineState.Get());

    // Set descriptor heaps (once per frame)
    ID3D12DescriptorHeap* heaps[] = { mEntityRenderer->GetCBVHeap() };
    mCommandList->SetDescriptorHeaps(1, heaps);

    // Set per-frame CBV (parameter 0 in root signature)
    UpdatePerFrameConstants(frameIndex, mCamera.View, mCamera.Projection);
    mCommandList->SetGraphicsRootDescriptorTable(0, mFrameCBV);

    // Set root signature and PSO
    mCommandList->SetRootSignature(mRootSignature.Get());
    mCommandList->SetPipelineState(mPipelineState.Get());

    // Render each entity
    for (const auto& entity : entities)
    {
        if (!entity.IsActive) continue;

        // Update entity's world transform in upload heap
        XMFLOAT4X4 worldMatrix = entity.Transform.GetWorldMatrixFloat4x4();
        mEntityRenderer->UpdateEntityTransform(entity.ID, frameIndex, worldMatrix);

        // Set per-object CBV (parameter 1 in root signature)
        D3D12_GPU_DESCRIPTOR_HANDLE entityCBV = 
            mEntityRenderer->GetEntityCBV(entity.ID, frameIndex);
        mCommandList->SetGraphicsRootDescriptorTable(1, entityCBV);

        // Bind vertex/index buffers for this entity's mesh
        BindMesh(entity.MeshIndex);

        // Draw
        mCommandList->DrawIndexedInstanced(
            GetMeshIndexCount(entity.MeshIndex),
            1, 0, 0, 0
        );
    }

    // Close and execute command list
    mCommandList->Close();
    ID3D12CommandList* cmdLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(1, cmdLists);

    // Signal fence with new frame value
    mCurrentFenceValue++;
    currentFrame->FenceValue = mCurrentFenceValue;
    mCommandQueue->Signal(mFence.Get(), mCurrentFenceValue);

    // Advance to next frame resource
    mFrameIndex = (frameIndex + 1) % mNumFrameResources;
}
```

**Verification**:
- Render single entity at origin, verify it appears at (0, 0, 0)
- Render entity at (5, 0, 0), verify it appears at world position (5, 0, 0)
- Render multiple entities at different positions, verify all appear correctly
- Use PIX to verify root parameter changes per draw call

---

## Step 10: Update Skeleton Debug Visualization

### File: `src/SkeletonRenderer.cpp` or equivalent

**Change**: Apply entity world transform to skeleton bone positions during debug rendering

**Code**:
```cpp
void SkeletonRenderer::RenderDebugSkeleton(
    const Skeleton& skeleton,
    const DirectX::XMFLOAT4X4& entityWorldMatrix,
    ID3D12GraphicsCommandList* commandList,
    UINT frameIndex)
{
    // Transform each bone position by entity's world matrix
    for (const auto& bone : skeleton.Bones)
    {
        // Bone local position
        XMVECTOR boneLocalPos = XMLoadFloat3(&bone.LocalPosition);
        XMVECTOR boneWorldPos;
        
        // Apply entity world transform
        XMMATRIX worldMatrix = XMLoadFloat4x4(&entityWorldMatrix);
        boneWorldPos = XMVector3TransformCoord(boneLocalPos, worldMatrix);
        
        XMFLOAT3 boneWorldPosFloat;
        XMStoreFloat3(&boneWorldPosFloat, boneWorldPos);
        
        // Render bone debug sphere/line at world position
        RenderDebugPoint(boneWorldPosFloat, commandList, frameIndex);
    }

    // Render bone connections (lines between parent and child bones)
    for (const auto& bone : skeleton.Bones)
    {
        if (bone.ParentIndex >= 0)
        {
            const auto& parentBone = skeleton.Bones[bone.ParentIndex];
            
            // Transform both parent and child to world space
            XMVECTOR parentLocal = XMLoadFloat3(&parentBone.LocalPosition);
            XMVECTOR childLocal = XMLoadFloat3(&bone.LocalPosition);
            
            XMMATRIX worldMatrix = XMLoadFloat4x4(&entityWorldMatrix);
            XMVECTOR parentWorld = XMVector3TransformCoord(parentLocal, worldMatrix);
            XMVECTOR childWorld = XMVector3TransformCoord(childLocal, worldMatrix);
            
            XMFLOAT3 parentWorldFloat, childWorldFloat;
            XMStoreFloat3(&parentWorldFloat, parentWorld);
            XMStoreFloat3(&childWorldFloat, childWorld);
            
            // Render debug line
            RenderDebugLine(parentWorldFloat, childWorldFloat, commandList, frameIndex);
        }
    }
}
```

**Verification**:
- Render skeleton debug visualization
- Verify skeleton bones align perfectly with mesh
- Move entity, verify skeleton and mesh move together
- Rotate entity, verify skeleton and mesh rotate together

---

## Step 11: Create Entity Manager

### File: `src/EntityManager.h` (new file)

**Change**: Create manager to handle entity lifecycle and transforms

**Code**:
```cpp
#pragma once
#include <vector>
#include <unordered_map>
#include "Entity.h"

class EntityManager
{
public:
    static EntityManager& Instance()
    {
        static EntityManager instance;
        return instance;
    }

    // Create new entity
    UINT CreateEntity()
    {
        Entity entity;
        entity.ID = mNextEntityID++;
        entity.IsActive = true;
        entity.Transform = TransformComponent{};
        mEntities.push_back(entity);
        mEntityIndexMap[entity.ID] = mEntities.size() - 1;
        return entity.ID;
    }

    // Get entity by ID
    Entity* GetEntity(UINT entityID)
    {
        auto it = mEntityIndexMap.find(entityID);
        if (it == mEntityIndexMap.end()) return nullptr;
        return &mEntities[it->second];
    }

    // Get all active entities
    const std::vector<Entity>& GetActiveEntities() const
    {
        return mEntities;
    }

    // Set entity position
    void SetPosition(UINT entityID, float x, float y, float z)
    {
        Entity* entity = GetEntity(entityID);
        if (entity)
        {
            entity->Transform.Position = { x, y, z };
        }
    }

    // Set entity rotation
    void SetRotation(UINT entityID, float pitch, float yaw, float roll)
    {
        Entity* entity = GetEntity(entityID);
        if (entity)
        {
            entity->Transform.Rotation = { pitch, yaw, roll };
        }
    }

    // Set entity scale
    void SetScale(UINT entityID, float x, float y, float z)
    {
        Entity* entity = GetEntity(entityID);
        if (entity)
        {
            entity->Transform.Scale = { x, y, z };
        }
    }

private:
    EntityManager() : mNextEntityID(0) {}
    ~EntityManager() = default;

    std::vector<Entity> mEntities;
    std::unordered_map<UINT, UINT> mEntityIndexMap; // ID -> index
    UINT mNextEntityID;
};
```

**Verification**:
- Create multiple entities, verify unique IDs
- Set transforms, verify world matrices update correctly
- Retrieve entities by ID, verify correct entity returned

---

## Verification Steps

### Phase 1: Unit Testing (Before Integration)

1. **Test constant buffer alignment**:
   ```cpp
   assert(CalculateConstantBufferByteSize(64) == 256);
   assert(CalculateConstantBufferByteSize(256) == 256);
   assert(CalculateConstantBufferByteSize(257) == 512);
   ```

2. **Test world matrix generation**:
   ```cpp
   TransformComponent transform;
   transform.Position = { 5.0f, 0.0f, 0.0f };
   XMFLOAT4X4 world = transform.GetWorldMatrixFloat4x4();
   // Verify translation components are 5, 0, 0
   ```

3. **Test mesh local space**:
   - Generate creature mesh
   - Verify vertex positions are centered around origin
   - Check min/max bounds are roughly symmetric

4. **Test skeleton local space**:
   - Generate skeleton
   - Verify bone positions align with mesh in local space
   - No offsets applied

### Phase 2: Integration Testing

1. **Render single entity at origin**:
   - Entity position: (0, 0, 0)
   - Verify mesh renders at world origin
   - Verify skeleton debug visualization aligns with mesh

2. **Render single entity at offset**:
   - Entity position: (10, 0, 5)
   - Verify mesh renders at world position (10, 0, 5)
   - Verify skeleton debug visualization moves with mesh

3. **Render multiple entities**:
   - Create 5 entities at different positions
   - Verify each entity renders at correct world position
   - Verify skeleton and mesh stay aligned for each entity

4. **Test rotation**:
   - Rotate entity 45 degrees around Y axis
   - Verify mesh rotates correctly
   - Verify skeleton rotates with mesh

5. **Test scaling**:
   - Scale entity to 2x size
   - Verify mesh scales correctly
   - Verify skeleton scales with mesh

### Phase 3: Performance Validation

1. **Use PIX to profile**:
   - Capture frame with 100 entities
   - Verify root signature changes per draw call
   - Check CBV buffer locations are correct
   - Verify no descriptor heap switches during frame

2. **Check frame resource synchronization**:
   - Run for 1000+ frames
   - Verify no GPU/CPU race conditions
   - Check fence values advance correctly
   - Verify no crashes or validation errors

3. **Measure CPU overhead**:
   - Profile time spent in `UpdateEntityTransform`
   - Profile time spent in `SetGraphicsRootDescriptorTable`
   - Compare with baseline (should be minimal overhead)

### Phase 4: Edge Cases

1. **Test entity at extreme positions**:
   - Position: (10000, 0, -10000)
   - Verify no precision issues

2. **Test zero scale**:
   - Scale: (0, 0, 0)
   - Verify no divide-by-zero or NaN

3. **Test rapid entity creation/destruction**:
   - Create 1000 entities, destroy 500, create 500 more
   - Verify no memory leaks
   - Verify no ID collisions

4. **Test frame resource exhaustion**:
   - Render more entities than `maxEntities` limit
   - Verify proper error handling or dynamic resizing

---

## File Changes Summary

| File | Change Type | Description |
|------|-------------|-------------|
| `src/RenderTypes.h` | **Modify** | Add `ObjectConstants`, `FrameConstants`, alignment utility |
| `src/FrameResource.h` | **Create** | Frame resource class declaration |
| `src/FrameResource.cpp` | **Create** | Frame resource upload heap implementation |
| `src/Renderer.cpp` | **Modify** | Update root signature creation, render loop |
| `shaders/EntityVertexShader.hlsl` | **Modify** | Add world matrix transformation |
| `src/EntityRenderer.h` | **Create/Modify** | Per-object CBV management |
| `src/EntityRenderer.cpp` | **Create/Modify** | CBV descriptor creation and updates |
| `src/Entity.h` | **Create/Modify** | Entity component with TransformComponent |
| `src/MeshGenerator.cpp` | **Modify** | **REMOVE** manual vertex offsetting |
| `src/SkeletonGenerator.cpp` | **Modify** | Ensure local space bone positions |
| `src/SkeletonRenderer.cpp` | **Modify** | Apply world transform to debug visualization |
| `src/EntityManager.h` | **Create** | Entity lifecycle management |
| `src/EntityManager.cpp` | **Create** | Entity manager implementation |

---

## Migration Checklist

- [ ] **Step 1**: Define constant buffer structures
- [ ] **Step 2**: Create frame resources system
- [ ] **Step 3**: Update root signature
- [ ] **Step 4**: Update HLSL vertex shader
- [ ] **Step 5**: Create per-object CBV management
- [ ] **Step 6**: Implement entity component with world transform
- [ ] **Step 7**: Refactor mesh generation to local space
- [ ] **Step 8**: Refactor skeleton generation to local space
- [ ] **Step 9**: Update render loop
- [ ] **Step 10**: Update skeleton debug visualization
- [ ] **Step 11**: Create entity manager
- [ ] **Testing**: Run all verification steps
- [ ] **Performance**: Profile with PIX
- [ ] **Cleanup**: Remove old coordinate space code

---

## References

1. **Microsoft DirectX-Graphics-Samples HelloConstBuffers**: https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12HelloWorld/src/HelloConstBuffers/D3D12HelloConstBuffers.cpp
2. **Microsoft D3D12 Resource Binding Spec**: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
3. **NVIDIA Advanced API Performance**: https://developer.nvidia.com/blog/advanced-api-performance-descriptors/
4. **CosmicLearn D3D12 Binding Performance**: https://www.cosmiclearn.com/dx12/binding-performance.php
5. **CosmicLearn D3D12 Constant Buffers**: https://www.cosmiclearn.com/dx12/constant-buffers.php
6. **CosmicLearn D3D12 Upload Heaps**: https://www.cosmiclearn.com/dx12/upload-heaps.php
7. **Microsoft DirectXTK12 Model**: https://github.com/microsoft/DirectXTK12/wiki/Model
8. **Stack Overflow D3D12 Multiple Meshes**: https://stackoverflow.com/questions/39796989/draw-multiple-meshes-to-different-locations-directx-12
9. **3D Game Engine Programming D3D12**: https://www.3dgep.com/learning-directx-12-2/
