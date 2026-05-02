#include "ProceduralMeshRenderer.h"
#include <stdexcept>

namespace Engine {
namespace Procedural {
namespace Mesh {

ProceduralMeshRenderer::ProceduralMeshRenderer() {
}

ProceduralMeshRenderer::~ProceduralMeshRenderer() {
    // ComPtr automatically releases resources
}

bool ProceduralMeshRenderer::Initialize(ID3D12Device* device) {
    if (!device) {
        return false;
    }
    m_device = device;
    return true;
}

bool ProceduralMeshRenderer::UpdateMesh(const MeshData& mesh, ID3D12GraphicsCommandList* commandList) {
    if (!m_device || mesh.vertices.empty() || mesh.indices.empty()) {
        return false;
    }
    
    // Create GPU resources
    if (!CreateGPUResources(mesh)) {
        return false;
    }
    
    // Upload mesh data to GPU
    UploadMeshData(mesh, commandList);
    
    m_indexCount = static_cast<UINT>(mesh.indices.size());
    return true;
}

void ProceduralMeshRenderer::Render(ID3D12GraphicsCommandList* commandList) const {
    if (!commandList || m_indexCount == 0) {
        return;
    }
    
    // Set vertex and index buffers
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // Draw indexed
    commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

bool ProceduralMeshRenderer::CreateGPUResources(const MeshData& mesh) {
    // Create vertex buffer (default heap)
    D3D12_RESOURCE_DESC vertexBufferDesc = {};
    vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferDesc.Width = mesh.vertices.size() * sizeof(DirectX::XMFLOAT3) + 
                             mesh.normals.size() * sizeof(DirectX::XMFLOAT3);
    vertexBufferDesc.Height = 1;
    vertexBufferDesc.DepthOrArraySize = 1;
    vertexBufferDesc.MipLevels = 1;
    vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexBufferDesc.SampleDesc.Count = 1;
    vertexBufferDesc.SampleDesc.Quality = 0;
    vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    D3D12_HEAP_PROPERTIES defaultHeapProps = {};
    defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    
    HRESULT hr = m_device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create vertex upload buffer (upload heap)
    D3D12_HEAP_PROPERTIES uploadHeapProps = {};
    uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    
    hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexUploadBuffer)
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create index buffer (default heap)
    D3D12_RESOURCE_DESC indexBufferDesc = {};
    indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexBufferDesc.Width = mesh.indices.size() * sizeof(uint32_t);
    indexBufferDesc.Height = 1;
    indexBufferDesc.DepthOrArraySize = 1;
    indexBufferDesc.MipLevels = 1;
    indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
    indexBufferDesc.SampleDesc.Count = 1;
    indexBufferDesc.SampleDesc.Quality = 0;
    indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    
    hr = m_device->CreateCommittedResource(
        &defaultHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &indexBufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Create index upload buffer (upload heap)
    hr = m_device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &indexBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexUploadBuffer)
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Set buffer views
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3) * 2; // position + normal
    m_vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferDesc.Width);
    
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_indexBufferView.SizeInBytes = static_cast<UINT>(indexBufferDesc.Width);
    
    return true;
}

void ProceduralMeshRenderer::UploadMeshData(const MeshData& mesh, ID3D12GraphicsCommandList* commandList) {
    // Combine vertices and normals into interleaved format
    std::vector<float> vertexData;
    vertexData.reserve(mesh.vertices.size() * 6); // 3 for position + 3 for normal
    
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        vertexData.push_back(mesh.vertices[i].x);
        vertexData.push_back(mesh.vertices[i].y);
        vertexData.push_back(mesh.vertices[i].z);
        
        if (i < mesh.normals.size()) {
            vertexData.push_back(mesh.normals[i].x);
            vertexData.push_back(mesh.normals[i].y);
            vertexData.push_back(mesh.normals[i].z);
        } else {
            vertexData.push_back(0.0f);
            vertexData.push_back(1.0f);
            vertexData.push_back(0.0f); // Default normal
        }
    }
    
    // Map upload buffers and copy data
    void* pVertexData;
    m_vertexUploadBuffer->Map(0, nullptr, &pVertexData);
    memcpy(pVertexData, vertexData.data(), vertexData.size() * sizeof(float));
    m_vertexUploadBuffer->Unmap(0, nullptr);
    
    void* pIndexData;
    m_indexUploadBuffer->Map(0, nullptr, &pIndexData);
    memcpy(pIndexData, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
    m_indexUploadBuffer->Unmap(0, nullptr);
    
    // Copy from upload heap to default heap
    commandList->CopyBufferRegion(m_vertexBuffer.Get(), 0, m_vertexUploadBuffer.Get(), 0, vertexData.size() * sizeof(float));
    commandList->CopyBufferRegion(m_indexBuffer.Get(), 0, m_indexUploadBuffer.Get(), 0, mesh.indices.size() * sizeof(uint32_t));
    
    // Transition resources to vertex/index buffer state
    D3D12_RESOURCE_BARRIER vertexBarrier;
    vertexBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    vertexBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    vertexBarrier.Transition.pResource = m_vertexBuffer.Get();
    vertexBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    vertexBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
    vertexBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &vertexBarrier);
    
    D3D12_RESOURCE_BARRIER indexBarrier;
    indexBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    indexBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    indexBarrier.Transition.pResource = m_indexBuffer.Get();
    indexBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    indexBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
    indexBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList->ResourceBarrier(1, &indexBarrier);
}

} // namespace Mesh
} // namespace Procedural
} // namespace Engine
