#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <vector>
#include "MarchingCubes.h"

namespace Engine {
namespace Procedural {
namespace Mesh {

// Renders procedurally generated meshes via DX12
class ProceduralMeshRenderer {
private:
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    
    // GPU resources for mesh rendering
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
    D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
    UINT m_indexCount = 0;
    
    // Upload buffers
    Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexUploadBuffer;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_indexUploadBuffer;
    
public:
    ProceduralMeshRenderer();
    ~ProceduralMeshRenderer();
    
    // Initialize with DX12 device
    bool Initialize(ID3D12Device* device);
    
    // Update mesh data (upload to GPU)
    bool UpdateMesh(const MeshData& mesh, ID3D12GraphicsCommandList* commandList);
    
    // Render the mesh
    void Render(ID3D12GraphicsCommandList* commandList) const;
    
    // Get index count
    UINT GetIndexCount() const { return m_indexCount; }
    
private:
    bool CreateGPUResources(const MeshData& mesh);
    void UploadMeshData(const MeshData& mesh, ID3D12GraphicsCommandList* commandList);
};

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
