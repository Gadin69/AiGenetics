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
    
    // Procedural mesh renderer (initialized with GraphicsEngine)
    std::unique_ptr<Engine::Procedural::Mesh::ProceduralMeshRenderer> m_meshRenderer;
    
public:
    bool Initialize();
    void Update(float deltaTime);
    void Render(GraphicsEngine* graphicsEngine);
    
    void CreateSampleCreatures();
    void TestBreedingSystem();
    
    // Phase 3: Generate creature meshes from genetics
    void GenerateCreatureMeshes(ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
    
    // Get creature meshes for rendering
    const std::vector<CreatureMeshData>& GetCreatureMeshes() const { return m_creatureMeshes; }
    
    // Get mesh renderer
    Engine::Procedural::Mesh::ProceduralMeshRenderer* GetMeshRenderer() { return m_meshRenderer.get(); }
    
private:
    DirectX::XMFLOAT4 GetColorFromIndex(int index);
    CreatureMeshData GenerateMeshForOrganism(const Engine::Genetics::Taxonomy::Organism* organism, int index);
};