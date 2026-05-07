#pragma once

#include "PBRMaterial.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <d3d12.h>
#include <wrl.h>
#include <memory>

namespace GeneticsGameEngine {
namespace Rendering {

// Material system manages all PBR materials in the scene
class MaterialSystem
{
public:
    MaterialSystem();
    ~MaterialSystem();
    
    bool Initialize(ID3D12Device* device);
    void Shutdown();
    
    // Material management
    MaterialID CreateMaterial(const PBRMaterial& material, const std::string& name = "");
    PBRMaterial* GetMaterial(MaterialID id);
    const PBRMaterial* GetMaterial(MaterialID id) const;
    
    // Create standard materials
    MaterialID CreateDefaultMaterial();
    MaterialID CreateSkinMaterial();
    MaterialID CreateExoskeletonMaterial();
    MaterialID CreateShellMaterial();
    
    // Update all material constant buffers
    void UpdateAllMaterials();
    
    // Bind material for rendering
    void BindMaterial(ID3D12GraphicsCommandList* commandList, MaterialID id);
    
    // Get material count
    size_t GetMaterialCount() const { return materials_.size(); }
    
private:
    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    
    std::unordered_map<MaterialID, PBRMaterial> materials_;
    std::unordered_map<MaterialID, std::string> materialNames_;
    MaterialID nextMaterialID_ = 1;
    
    std::mutex mutex_;
};

} // namespace Rendering
} // namespace GeneticsGameEngine