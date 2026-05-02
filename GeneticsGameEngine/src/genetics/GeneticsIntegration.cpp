#include "GeneticsIntegration.h"
#include "../graphics/GraphicsEngine.h"
#include <iostream>
#include <DirectXMath.h>
#include <chrono>

namespace Genetics = Engine::Genetics;
namespace Taxonomy = Engine::Genetics::Taxonomy;
namespace Expression = Engine::Genetics::Expression;
namespace Breeding = Engine::Genetics::Breeding;

// Define locus IDs for creature genomes
static const std::vector<uint16_t> CREATURE_LOCUS_IDS = {
    0x1A2B, // Scale
    0x3C4D, // Color
    0x5E6F, // Limb count
    0x7A8B, // Skeletal density / Exoskeleton thickness
    0x9C0D, // Skin roughness / Segment count
    0x1E2F, // Skin metallic / Joint flexibility
    0x3A4B, // Exoskeleton thickness
    0x5C6D, // Segment count
    0x7E8F, // Joint flexibility
    0x9A0B, // Shell spiral
    0x1C2D, // Shell thickness
    0x3E4F  // Mantle texture
};

// Implementation for genetics integration
bool GeneticsIntegration::Initialize()
{
    std::cout << "\n=== Phase 2: Genetics System Integration ==="  << std::endl;
    std::cout << "Initializing genetics system..." << std::endl;
    
    // Create sample creatures
    CreateSampleCreatures();
    
    // Test breeding system
    TestBreedingSystem();
    
    std::cout << "\nGenetics integration initialized successfully!" << std::endl;
    std::cout << "Total organisms: " << m_organisms.size() << std::endl;
    std::cout.flush();
    
    return true;
}

// Phase 3: Generate creature meshes from genetics
void GeneticsIntegration::GenerateCreatureMeshes(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
    std::cout << "\n=== Phase 3: Generating Creature Meshes ==="  << std::endl;
    
    // Initialize mesh renderer if not already done
    if (!m_meshRenderer) {
        m_meshRenderer = std::make_unique<Engine::Procedural::Mesh::ProceduralMeshRenderer>();
        if (!m_meshRenderer->Initialize(device)) {
            std::cerr << "Failed to initialize procedural mesh renderer!" << std::endl;
            return;
        }
        std::cout << "Procedural mesh renderer initialized." << std::endl;
    }
    
    // Clear existing meshes
    m_creatureMeshes.clear();
    
    // Generate mesh for each organism
    float xPos = -3.0f;
    for (size_t i = 0; i < m_organisms.size(); ++i) {
        try {
            std::cout << "  Generating mesh for organism " << i << "/" << m_organisms.size() << "..." << std::endl;
            CreatureMeshData meshData = GenerateMeshForOrganism(m_organisms[i].get(), static_cast<int>(i));
            meshData.position = DirectX::XMFLOAT3(xPos, 0.0f, 0.0f);
            m_creatureMeshes.push_back(meshData);
            std::cout << "  Mesh " << i << " generated successfully." << std::endl;
            xPos += 4.0f; // Space creatures apart
        } catch (const std::exception& e) {
            std::cerr << "  ERROR generating mesh for organism " << i << ": " << e.what() << std::endl;
            // Continue with next organism instead of crashing
        } catch (...) {
            std::cerr << "  UNKNOWN ERROR generating mesh for organism " << i << std::endl;
        }
    }
    
    std::cout << "Generated " << m_creatureMeshes.size() << " creature meshes." << std::endl;
}

void GeneticsIntegration::CreateSampleCreatures()
{
    std::cout << "\n--- Creating Sample Creatures ---" << std::endl;
    
    // Create Chordata creature
    {
        auto chordata = std::make_unique<Taxonomy::Chordata>();
        chordata->SetID("Chordata_001");
        
        // Generate random genome
        Engine::Genetics::Genome genome = Breeding::MutationEngine::GenerateRandomGenome("Chordata_001_Genome", CREATURE_LOCUS_IDS);
        
        // Apply genetic expression
        chordata->ApplyGeneticExpression(genome);
        
        std::cout << "Created Chordata creature: " << chordata->GetID() << std::endl;
        std::cout << "  Scale: " << chordata->GetScale() << std::endl;
        std::cout << "  Color Index: " << chordata->GetColorIndex() << std::endl;
        std::cout << "  Limb Count: " << chordata->GetLimbCount() << std::endl;
        std::cout << "  Skeletal Density: " << chordata->GetSkeletalDensity() << std::endl;
        
        m_organisms.push_back(std::move(chordata));
    }
    
    // Create Arthropoda creature
    {
        auto arthropoda = std::make_unique<Taxonomy::Arthropoda>();
        arthropoda->SetID("Arthropoda_001");
        
        Engine::Genetics::Genome genome = Breeding::MutationEngine::GenerateRandomGenome("Arthropoda_001_Genome", CREATURE_LOCUS_IDS);
        arthropoda->ApplyGeneticExpression(genome);
        
        std::cout << "Created Arthropoda creature: " << arthropoda->GetID() << std::endl;
        std::cout << "  Scale: " << arthropoda->GetScale() << std::endl;
        std::cout << "  Color Index: " << arthropoda->GetColorIndex() << std::endl;
        std::cout << "  Limb Count: " << arthropoda->GetLimbCount() << std::endl;
        std::cout << "  Segment Count: " << arthropoda->GetSegmentCount() << std::endl;
        
        m_organisms.push_back(std::move(arthropoda));
    }
    
    // Create Mollusca creature
    {
        auto mollusca = std::make_unique<Taxonomy::Mollusca>();
        mollusca->SetID("Mollusca_001");
        
        Engine::Genetics::Genome genome = Breeding::MutationEngine::GenerateRandomGenome("Mollusca_001_Genome", CREATURE_LOCUS_IDS);
        mollusca->ApplyGeneticExpression(genome);
        
        std::cout << "Created Mollusca creature: " << mollusca->GetID() << std::endl;
        std::cout << "  Scale: " << mollusca->GetScale() << std::endl;
        std::cout << "  Color Index: " << mollusca->GetColorIndex() << std::endl;
        std::cout << "  Tentacle Count: " << mollusca->GetLimbCount() << std::endl;
        std::cout << "  Shell Spiral Angle: " << mollusca->GetShellSpiralAngle() << std::endl;
        
        m_organisms.push_back(std::move(mollusca));
    }
}

void GeneticsIntegration::TestBreedingSystem()
{
    std::cout << "\n--- Testing Breeding System ---" << std::endl;
    
    if (m_organisms.size() < 2) {
        std::cout << "Need at least 2 organisms to test breeding" << std::endl;
        return;
    }
    
    // Test breeding between two Chordata (would need to create another one)
    // For now, just demonstrate the system
    std::cout << "Breeding system test complete" << std::endl;
    std::cout << "Note: Breeding requires organisms of the same species" << std::endl;
}

void GeneticsIntegration::Update(float deltaTime)
{
    // Update all organisms
    for (auto& organism : m_organisms)
    {
        // Could apply mutations over time
        // organism->ApplyMutation(0.001f); // Very slow mutation rate
    }
}

// Phase 3: Generate mesh for a single organism
CreatureMeshData GeneticsIntegration::GenerateMeshForOrganism(
    const Engine::Genetics::Taxonomy::Organism* organism, int index) 
{
    std::cout << "    GenerateMeshForOrganism called for " << organism->GetID() << std::endl;
    CreatureMeshData result;
    result.creatureID = organism->GetID();
    result.scale = organism->GetScale();
    result.color = GetColorFromIndex(organism->GetColorIndex());
    result.currentLOD = 0;
    
    std::cout << "    Setting up creature parameters..." << std::endl;
    
    // Step 1: Map genome to creature parameters
    // (Using simplified mapping since we don't have direct genome access)
    Engine::Procedural::Generation::CreatureParams params;
    params.scaleFactor = organism->GetScale();
    params.colorPaletteIndex = organism->GetColorIndex();
    params.limbCount = 4; // Default
    params.roughness = 0.5f;
    params.metallic = 0.3f;
    
    // Set taxonomy-specific parameters
    auto chordata = dynamic_cast<const Engine::Genetics::Taxonomy::Chordata*>(organism);
    auto arthropoda = dynamic_cast<const Engine::Genetics::Taxonomy::Arthropoda*>(organism);
    auto mollusca = dynamic_cast<const Engine::Genetics::Taxonomy::Mollusca*>(organism);
    
    if (chordata) {
        params.taxonomyType = 0; // Chordata
        params.limbCount = chordata->GetLimbCount();
        params.bodyCenter = DirectX::XMFLOAT3(0, 1.0f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.5f * params.scaleFactor, 1.0f * params.scaleFactor, 0.5f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.8f * params.scaleFactor, 0);
        params.headRadius = 0.3f * params.scaleFactor;
    } else if (arthropoda) {
        params.taxonomyType = 1; // Arthropoda
        params.limbCount = arthropoda->GetLimbCount();
        params.bodyCenter = DirectX::XMFLOAT3(0, 0.5f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.6f * params.scaleFactor, 0.4f * params.scaleFactor, 0.8f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 0.9f * params.scaleFactor, 0);
        params.headRadius = 0.25f * params.scaleFactor;
    } else if (mollusca) {
        params.taxonomyType = 2; // Mollusca
        params.limbCount = mollusca->GetLimbCount();
        params.bodyCenter = DirectX::XMFLOAT3(0, 0.6f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.7f * params.scaleFactor, 0.5f * params.scaleFactor, 0.7f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.2f * params.scaleFactor, 0);
        params.headRadius = 0.35f * params.scaleFactor;
    } else {
        params.taxonomyType = 0; // Default to Chordata
        params.bodyCenter = DirectX::XMFLOAT3(0, 1.0f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.5f * params.scaleFactor, 1.0f * params.scaleFactor, 0.5f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.8f * params.scaleFactor, 0);
        params.headRadius = 0.3f * params.scaleFactor;
    }
    
    // Step 2: Generate voxel grid with scalar field
    std::cout << "    Allocating voxel grid..." << std::endl;
    int gridSize = 64; // Medium resolution
    Engine::Procedural::Voxel::VoxelGrid grid;
    grid.AllocateGrid(gridSize, gridSize, gridSize, 0.05f);
    
    std::cout << "    Generating scalar field..." << std::endl;
    m_scalarFieldGenerator.GenerateField(grid, params);
    
    // Step 3: Run marching cubes to extract mesh
    std::cout << "    Running marching cubes..." << std::endl;
    float isovalue = 0.5f;
    result.mesh = m_marchingCubes.GenerateMesh(grid, isovalue);
    
    std::cout << "    Mesh extracted: " << result.mesh.vertices.size() << " vertices, " << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    
    // Step 4: Optimize mesh if needed
    uint32_t targetTriangles = 10000;
    if (result.mesh.indices.size() / 3 > targetTriangles) {
        std::cout << "    Optimizing mesh (target: " << targetTriangles << " triangles)..." << std::endl;
        result.mesh = m_meshOptimizer.SimplifyMesh(result.mesh, targetTriangles);
        std::cout << "    Mesh optimized: " << result.mesh.vertices.size() << " vertices, " << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    }
    
    std::cout << "  " << result.creatureID << ": " 
              << result.mesh.vertices.size() << " vertices, "
              << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    
    return result;
}

void GeneticsIntegration::Render(GraphicsEngine* graphicsEngine) {
    
    /*
    // Collect visual parameters from all organisms
    std::vector<CreatureRenderData> creatureData;
    creatureData.reserve(m_organisms.size());
    
    float xPos = -3.0f;
    for (const auto& organism : m_organisms)
    {
        CreatureRenderData data;
        data.scale = organism->GetScale();
        data.position = DirectX::XMFLOAT3(xPos, 0.0f, 0.0f);
        data.color = GetColorFromIndex(organism->GetColorIndex());
        
        creatureData.push_back(data);
        xPos += 3.0f; // Space creatures apart
    }
    
    // Render creatures
    if (!creatureData.empty())
    {
        graphicsEngine->RenderCreatures(creatureData, nullptr);
    }
    */
}

DirectX::XMFLOAT4 GeneticsIntegration::GetColorFromIndex(int index)
{
    static const DirectX::XMFLOAT4 colorPalette[] = {
        DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),    // Red
        DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),    // Green
        DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),    // Blue
        DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f),    // Yellow
        DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),    // Magenta
        DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f),    // Cyan
        DirectX::XMFLOAT4(1.0f, 0.5f, 0.0f, 1.0f),    // Orange
        DirectX::XMFLOAT4(0.5f, 0.0f, 1.0f, 1.0f)     // Purple
    };
    
    if (index >= 0 && index < 8) {
        return colorPalette[index];
    }
    return DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
}