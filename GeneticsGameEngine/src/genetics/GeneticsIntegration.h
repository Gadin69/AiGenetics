#pragma once

#include <vector>
#include <memory>
#include <string>
#include <windows.h>
#include <d3d12.h>

// Forward declarations
class GraphicsEngine;

// Include engine genetics system
#include "../engine/genetics/genome/Genome.h"
#include "../engine/genetics/taxonomy/Organism.h"
#include "../engine/genetics/taxonomy/Chordata.h"
#include "../engine/genetics/taxonomy/Arthropoda.h"
#include "../engine/genetics/taxonomy/Mollusca.h"
#include "../engine/genetics/expression/GeneticExpression.h"
#include "../engine/genetics/breeding/BreedingSystem.h"
#include "../engine/genetics/breeding/MutationEngine.h"
#include "../engine/animation/Skeleton.h"

// Phase 3: Procedural mesh generation includes
#include "../engine/procedural/voxel/VoxelGrid.h"
#include "../engine/procedural/generation/CreatureParams.h"
#include "../engine/procedural/generation/GeneticMapper.h"
#include "../engine/procedural/generation/ScalarFieldGenerator.h"
#include "../engine/procedural/mesh/MarchingCubes.h"
#include "../engine/procedural/mesh/MeshOptimizer.h"
#include "../engine/procedural/mesh/ProceduralMeshRenderer.h"
#include "../engine/procedural/voxel/VoxelLODManager.h"

// Include actual C++ header files
#include "../graphics/GraphicsEngine.h"

// Phase 4: Neural network system includes
#include "../engine/neural/NeuralSystemManager.h"

// Phase 5: PBR material system includes
#include "../engine/rendering/materials/pbr/MaterialSystem.h"
#include "../engine/rendering/textures/ProceduralTextureGenerator.h"

// Forward declarations for engine classes
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

// Creature mesh data for rendering
struct CreatureMeshData {
    std::string creatureID;
    Engine::Procedural::Mesh::MeshData mesh;
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
    float scale;
    int currentLOD;
    
    // GPU resources for this creature
    std::unique_ptr<Engine::Procedural::Mesh::ProceduralMeshRenderer> meshRenderer;
    
    // Phase 5: PBR material
    GeneticsGameEngine::Rendering::MaterialID materialID;
    
    // Phase 7: Skeleton (NEW)
    std::unique_ptr<Engine::Animation::Skeleton> skeleton;
    bool showSkeletonVisualization; // Toggle for debug visualization
};

class GeneticsIntegration
{
private:
    std::vector<std::unique_ptr<Engine::Genetics::Taxonomy::Organism>> m_organisms;
    
    // Phase 3: Procedural generation components
    std::vector<CreatureMeshData> m_creatureMeshes;
    Engine::Procedural::Generation::GeneticMapper m_geneticMapper;
    Engine::Procedural::Generation::ScalarFieldGenerator m_scalarFieldGenerator;
    Engine::Procedural::Mesh::MarchingCubes m_marchingCubes;
    Engine::Procedural::Mesh::MeshOptimizer m_meshOptimizer;
    Engine::Procedural::Voxel::VoxelLODManager m_lodManager;
    
    // Stored device for mesh regeneration
    ID3D12Device* m_device = nullptr;
    
    // Phase 4: Neural network indices (one per organism)
    std::vector<size_t> m_neuralNetworkIndices;
    
    // Phase 5: PBR material system
    std::unique_ptr<GeneticsGameEngine::Rendering::MaterialSystem> m_materialSystem;
    GeneticsGameEngine::Rendering::ProceduralTextureGenerator m_textureGenerator;
    
public:
    bool Initialize();
    void Update(float deltaTime);
    void Render(GraphicsEngine* graphicsEngine);
    
    void CreateSampleCreatures();
    void TestBreedingSystem();
    
    // Phase 3: Generate creature meshes from genetics
    void GenerateCreatureMeshes(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    
    // Phase 4: Neural system integration
    void InitializeNeuralSystems();
    void UpdateNeuralSystems(float deltaTime);
    void ApplyNeuralBehavioralOutputs();
    
    // Phase 5: PBR material assignment
    void InitializeMaterialSystem(ID3D12Device* device);
    GeneticsGameEngine::Rendering::MaterialID AssignMaterialFromGenetics(
        const Engine::Genetics::Taxonomy::Organism* organism);
    
    // Get creature meshes for rendering
    const std::vector<CreatureMeshData>& GetCreatureMeshes() const { return m_creatureMeshes; }
    
    // Get scalar field generator for UI control
    Engine::Procedural::Generation::ScalarFieldGenerator& GetScalarFieldGenerator() { return m_scalarFieldGenerator; }
    
    // Regenerate all meshes with custom parameters
    void RegenerateMeshes(float voxelSize, float falloffMultiplier);
    
    // Regenerate creatures with a specific seed
    void RegenerateCreaturesWithSeed(uint32_t seed, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    
private:
    DirectX::XMFLOAT4 GetColorFromIndex(int index);
    CreatureMeshData GenerateMeshForOrganism(const Engine::Genetics::Taxonomy::Organism* organism, int index);
    CreatureMeshData GenerateMeshForOrganismWithParams(
        const Engine::Genetics::Taxonomy::Organism* organism, int index,
        float voxelSize, float falloffMultiplier);
};