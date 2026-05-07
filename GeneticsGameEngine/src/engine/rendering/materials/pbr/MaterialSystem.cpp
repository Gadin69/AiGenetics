#include "MaterialSystem.h"
#include <iostream>

using namespace GeneticsGameEngine::Rendering;

MaterialSystem::MaterialSystem()
{
}

MaterialSystem::~MaterialSystem()
{
    Shutdown();
}

bool MaterialSystem::Initialize(ID3D12Device* device)
{
    if (!device)
    {
        std::cerr << "[MaterialSystem] Invalid device pointer" << std::endl;
        return false;
    }
    
    device_ = device;
    
    // Create default material
    CreateDefaultMaterial();
    
    std::cout << "[MaterialSystem] Initialized" << std::endl;
    return true;
}

void MaterialSystem::Shutdown()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : materials_)
    {
        pair.second.Cleanup();
    }
    
    materials_.clear();
    materialNames_.clear();
    device_.Reset();
    
    std::cout << "[MaterialSystem] Shutdown complete" << std::endl;
}

MaterialID MaterialSystem::CreateMaterial(const PBRMaterial& material, const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    MaterialID id = nextMaterialID_++;
    
    PBRMaterial newMaterial = material;
    if (!newMaterial.InitializeConstantBuffer(device_.Get()))
    {
        std::cerr << "[MaterialSystem] Failed to initialize constant buffer for material " << id << std::endl;
        return 0;
    }
    
    materials_[id] = std::move(newMaterial);
    materialNames_[id] = name;
    
    return id;
}

PBRMaterial* MaterialSystem::GetMaterial(MaterialID id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = materials_.find(id);
    if (it != materials_.end())
    {
        return &it->second;
    }
    
    return nullptr;
}

const PBRMaterial* MaterialSystem::GetMaterial(MaterialID id) const
{
    auto it = materials_.find(id);
    if (it != materials_.end())
    {
        return &it->second;
    }
    
    return nullptr;
}

MaterialID MaterialSystem::CreateDefaultMaterial()
{
    PBRMaterial material;
    material.albedo = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
    material.roughness = 0.5f;
    material.metallic = 0.0f;
    material.ambientOcclusion = 1.0f;
    
    return CreateMaterial(material, "Default");
}

MaterialID MaterialSystem::CreateSkinMaterial()
{
    PBRMaterial material;
    material.albedo = DirectX::XMFLOAT3(0.95f, 0.75f, 0.70f);
    material.roughness = 0.7f;
    material.metallic = 0.0f;
    material.ambientOcclusion = 0.8f;
    material.emissive = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    material.emissiveIntensity = 0.0f;
    
    return CreateMaterial(material, "Skin");
}

MaterialID MaterialSystem::CreateExoskeletonMaterial()
{
    PBRMaterial material;
    material.albedo = DirectX::XMFLOAT3(0.3f, 0.2f, 0.15f);
    material.roughness = 0.6f;
    material.metallic = 0.1f;
    material.ambientOcclusion = 0.9f;
    material.emissive = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    material.emissiveIntensity = 0.0f;
    
    return CreateMaterial(material, "Exoskeleton");
}

MaterialID MaterialSystem::CreateShellMaterial()
{
    PBRMaterial material;
    material.albedo = DirectX::XMFLOAT3(0.85f, 0.80f, 0.70f);
    material.roughness = 0.3f;
    material.metallic = 0.05f;
    material.ambientOcclusion = 0.85f;
    material.emissive = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    material.emissiveIntensity = 0.0f;
    
    return CreateMaterial(material, "Shell");
}

void MaterialSystem::UpdateAllMaterials()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& pair : materials_)
    {
        pair.second.UpdateConstantBuffer();
    }
}

void MaterialSystem::BindMaterial(ID3D12GraphicsCommandList* commandList, MaterialID id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = materials_.find(id);
    if (it != materials_.end())
    {
        commandList->SetGraphicsRootConstantBufferView(
            0,  // Root parameter index
            it->second.constantBuffer->GetGPUVirtualAddress()
        );
    }
}
