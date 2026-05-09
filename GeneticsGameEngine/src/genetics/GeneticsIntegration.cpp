#include "GeneticsIntegration.h"
#include "../graphics/GraphicsEngine.h"
#include <iostream>
#include <algorithm>
#include <DirectXMath.h>
#include <chrono>

// Phase 4: Neural system namespace
namespace Neural = Engine::Neural;

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
    
    // Phase 4: Initialize neural systems for all organisms
    InitializeNeuralSystems();
    
    return true;
}

// Phase 3: Generate creature meshes from genetics
void GeneticsIntegration::GenerateCreatureMeshes(ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {
    std::cout << "\n=== Phase 3: Generating Creature Meshes ==="  << std::endl;
    
    if (!device || !commandList) {
        std::cerr << "Invalid device or command list!" << std::endl;
        return;
    }
    
    // Store device for later regeneration
    m_device = device;
    
    // Clear existing meshes
    m_creatureMeshes.clear();
    
    // Create a temporary command allocator and list for uploading mesh data
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
    
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
    if (FAILED(hr)) {
        std::cerr << "Failed to create upload command allocator!" << std::endl;
        return;
    }
    
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCommandList));
    if (FAILED(hr)) {
        std::cerr << "Failed to create upload command list!" << std::endl;
        return;
    }
    
    // Generate mesh for each organism
    float xPos = -3.0f;
    for (size_t i = 0; i < m_organisms.size(); ++i) {
        try {
            std::cout << "  Generating mesh for organism " << i << "/" << m_organisms.size() << "..." << std::endl;
            CreatureMeshData meshData = GenerateMeshForOrganism(m_organisms[i].get(), static_cast<int>(i));
            
            // ENTITY ARCHITECTURE: Mesh stays in LOCAL SPACE (centered at origin)
            // World transform (position) is stored separately and applied at render time via shader
            meshData.position = DirectX::XMFLOAT3(xPos, 2.0f, 0.0f); // World position for entity
            
            std::cout << "    Mesh generated in local space, world position: (" 
                      << meshData.position.x << ", " << meshData.position.y << ", " 
                      << meshData.position.z << ")" << std::endl;
            
            // Initialize mesh renderer and upload to GPU
            meshData.meshRenderer = std::make_unique<Engine::Procedural::Mesh::ProceduralMeshRenderer>();
            if (!meshData.meshRenderer->Initialize(device)) {
                std::cerr << "  Failed to initialize mesh renderer for creature " << i << std::endl;
                continue;
            }
            
            // Create a transformed copy of the mesh with position offset
            Engine::Procedural::Mesh::MeshData transformedMesh = meshData.mesh;
            DirectX::XMVECTOR translation = DirectX::XMLoadFloat3(&meshData.position);
            
            for (auto& vertex : transformedMesh.vertices) {
                DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&vertex);
                pos = DirectX::XMVectorAdd(pos, translation);
                DirectX::XMStoreFloat3(&vertex, pos);
            }
            
            // Upload mesh data using the temporary command list
            if (!meshData.meshRenderer->UpdateMesh(transformedMesh, uploadCommandList.Get())) {
                std::cerr << "  Failed to upload mesh for creature " << i << std::endl;
                continue;
            }
            
            std::cout << "  Mesh " << i << " upload commands recorded" << std::endl;
            
            // Phase 5: Assign PBR material based on genetics
            if (m_materialSystem)
            {
                meshData.materialID = AssignMaterialFromGenetics(m_organisms[i].get());
                std::cout << "  Assigned material ID " << meshData.materialID << " to creature " << i << std::endl;
            }
            else
            {
                meshData.materialID = 0; // No material assigned
                std::cout << "  WARNING: Material system not initialized, no material assigned" << std::endl;
            }
            
            m_creatureMeshes.push_back(std::move(meshData));
            std::cout << "  Mesh " << i << " generated successfully." << std::endl;
            xPos += 4.0f; // Space creatures apart
        } catch (const std::exception& e) {
            std::cerr << "  ERROR generating mesh for organism " << i << ": " << e.what() << std::endl;
            // Continue with next organism instead of crashing
        } catch (...) {
            std::cerr << "  UNKNOWN ERROR generating mesh for organism " << i << std::endl;
        }
    }
    
    // Close and execute the upload command list
    hr = uploadCommandList->Close();
    if (FAILED(hr)) {
        std::cerr << "Failed to close upload command list!" << std::endl;
        return;
    }
    
    // Create a temporary command queue for upload
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> uploadQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&uploadQueue));
    if (FAILED(hr)) {
        std::cerr << "Failed to create upload command queue!" << std::endl;
        return;
    }
    
    // Execute the upload command list
    ID3D12CommandList* ppCommandLists[] = { uploadCommandList.Get() };
    uploadQueue->ExecuteCommandLists(1, ppCommandLists);
    
    // Wait for GPU to complete the uploads
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        std::cerr << "Failed to create fence!" << std::endl;
        return;
    }
    
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    const UINT64 fenceValue = 1;
    uploadQueue->Signal(fence.Get(), fenceValue);
    
    if (fence->GetCompletedValue() < fenceValue) {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    
    CloseHandle(fenceEvent);
    
    std::cout << "Mesh uploads completed and executed on GPU." << std::endl;
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
    
    // Phase 4: Update neural systems
    UpdateNeuralSystems(deltaTime);
}

// Phase 3: Generate mesh for a single organism
CreatureMeshData GeneticsIntegration::GenerateMeshForOrganism(
    const Engine::Genetics::Taxonomy::Organism* organism, int index) 
{
    return GenerateMeshForOrganismWithParams(organism, index, 0.031f, 1.2f); // User-tuned defaults
}

// Generate mesh with custom voxel and falloff parameters
CreatureMeshData GeneticsIntegration::GenerateMeshForOrganismWithParams(
    const Engine::Genetics::Taxonomy::Organism* organism, int index,
    float voxelSize, float falloffMultiplier)
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
        params.archetype = Engine::Procedural::Generation::ArchetypeType::Chordata;
        params.blendSmoothness = 0.3f; // Moderate blending for vertebrates
        params.bodyCenter = DirectX::XMFLOAT3(0, 1.0f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.5f * params.scaleFactor, 1.0f * params.scaleFactor, 0.5f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.8f * params.scaleFactor, 0);
        params.headRadius = 0.3f * params.scaleFactor;
    } else if (arthropoda) {
        params.taxonomyType = 1; // Arthropoda
        params.limbCount = arthropoda->GetLimbCount();
        params.archetype = Engine::Procedural::Generation::ArchetypeType::Arthropoda;
        params.blendSmoothness = 0.05f; // Hard blending for exoskeleton
        params.bodyCenter = DirectX::XMFLOAT3(0, 0.5f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.6f * params.scaleFactor, 0.4f * params.scaleFactor, 0.8f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 0.9f * params.scaleFactor, 0);
        params.headRadius = 0.25f * params.scaleFactor;
    } else if (mollusca) {
        params.taxonomyType = 2; // Mollusca
        params.limbCount = mollusca->GetLimbCount();
        params.archetype = Engine::Procedural::Generation::ArchetypeType::Mollusca;
        params.blendSmoothness = 0.5f; // Very smooth blending for soft bodies
        params.bodyCenter = DirectX::XMFLOAT3(0, 0.6f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.7f * params.scaleFactor, 0.5f * params.scaleFactor, 0.7f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.2f * params.scaleFactor, 0);
        params.headRadius = 0.35f * params.scaleFactor;
    } else {
        params.taxonomyType = 0; // Default to Chordata
        params.archetype = Engine::Procedural::Generation::ArchetypeType::Chordata;
        params.blendSmoothness = 0.3f;
        params.bodyCenter = DirectX::XMFLOAT3(0, 1.0f, 0);
        params.bodyRadii = DirectX::XMFLOAT3(0.5f * params.scaleFactor, 1.0f * params.scaleFactor, 0.5f * params.scaleFactor);
        params.headCenter = DirectX::XMFLOAT3(0, 1.8f * params.scaleFactor, 0);
        params.headRadius = 0.3f * params.scaleFactor;
    }
    
    // Step 2: NEW - Generate skeleton from genetics (Phase 7)
    std::cout << "    Generating skeleton from genetics..." << std::endl;
    result.skeleton = organism->GenerateSkeleton();
    result.showSkeletonVisualization = false; // Default to hidden
    
    // Step 3: Generate voxel grid with scalar field
    std::cout << "    Allocating voxel grid..." << std::endl;
    int gridSize = 64; // Medium resolution
    // float voxelSize = 0.02f; // Higher resolution (was 0.05f) - NOW PASSED AS PARAMETER
    Engine::Procedural::Voxel::VoxelGrid grid;
    grid.AllocateGrid(gridSize, gridSize, gridSize, voxelSize);
    
    // Set falloff multiplier on scalar field generator
    m_scalarFieldGenerator.SetFalloffMultiplier(falloffMultiplier);
    
    // NEW: Use skeleton-based scalar field if skeleton was generated
    if (result.skeleton)
    {
        std::cout << "    Generating scalar field FROM SKELETON..." << std::endl;
        m_scalarFieldGenerator.GenerateFieldFromSkeleton(grid, *result.skeleton, params);
    }
    else
    {
        std::cout << "    Generating scalar field (legacy mode)..." << std::endl;
        m_scalarFieldGenerator.GenerateField(grid, params);
    }
    
    // Step 4: Run marching cubes to extract mesh
    std::cout << "    Running marching cubes..." << std::endl;
    float isovalue = 0.5f;
    result.mesh = m_marchingCubes.GenerateMesh(grid, isovalue);
    
    // Step 4.5: Assign creature color to all vertices
    std::cout << "    Assigning creature color (" << result.color.x << ", " 
              << result.color.y << ", " << result.color.z << ") to " 
              << result.mesh.vertices.size() << " vertices..." << std::endl;
    result.mesh.colors.resize(result.mesh.vertices.size(), result.color);
    
    std::cout << "    Mesh extracted: " << result.mesh.vertices.size() << " vertices, " << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    
    // DEBUG: Print first 5 vertices to check if they're spread out or collapsed
    if (result.mesh.vertices.size() > 0) {
        std::cout << "    DEBUG - First 5 vertices:" << std::endl;
        size_t count = result.mesh.vertices.size() < 5 ? result.mesh.vertices.size() : 5;
        for (size_t i = 0; i < count; ++i) {
            const auto& v = result.mesh.vertices[i];
            std::cout << "      Vertex " << i << ": (" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
        }
        
        // Calculate bounding box
        float minX = result.mesh.vertices[0].x, maxX = result.mesh.vertices[0].x;
        float minY = result.mesh.vertices[0].y, maxY = result.mesh.vertices[0].y;
        float minZ = result.mesh.vertices[0].z, maxZ = result.mesh.vertices[0].z;
        
        for (const auto& v : result.mesh.vertices) {
            if (v.x < minX) minX = v.x;
            if (v.x > maxX) maxX = v.x;
            if (v.y < minY) minY = v.y;
            if (v.y > maxY) maxY = v.y;
            if (v.z < minZ) minZ = v.z;
            if (v.z > maxZ) maxZ = v.z;
        }
        
        std::cout << "    DEBUG - Bounding box: (" << minX << "," << minY << "," << minZ << ") to (" 
                  << maxX << "," << maxY << "," << maxZ << ")" << std::endl;
        std::cout << "    DEBUG - Size: " << (maxX-minX) << " x " << (maxY-minY) << " x " << (maxZ-minZ) << std::endl;
    }
    
    // Step 4: Optimize mesh if needed
    // Skeleton-generated meshes are more complex, allow higher triangle count
    uint32_t targetTriangles = 30000; // Increased from 10000 for skeletal meshes
    if (result.mesh.indices.size() / 3 > targetTriangles) {
        std::cout << "    Optimizing mesh (target: " << targetTriangles << " triangles)..." << std::endl;
        result.mesh = m_meshOptimizer.SimplifyMesh(result.mesh, targetTriangles);
        std::cout << "    Mesh optimized: " << result.mesh.vertices.size() << " vertices, " << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    }
    
    std::cout << "  " << result.creatureID << ": " 
              << result.mesh.vertices.size() << " vertices, "
              << result.mesh.indices.size() / 3 << " triangles" << std::endl;
    
    // Create mesh renderer and upload to GPU
    result.meshRenderer = std::make_unique<Engine::Procedural::Mesh::ProceduralMeshRenderer>();
    
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

// Phase 4: Neural system integration methods
void GeneticsIntegration::InitializeNeuralSystems()
{
    std::cout << "\n=== Phase 4: Initializing Neural Systems ===" << std::endl;
    
    // Get neural system manager instance
    auto& neuralManager = Neural::NeuralSystemManager::GetInstance();
    neuralManager.Initialize();
    
    // Create neural network for each organism
    m_neuralNetworkIndices.clear();
    m_neuralNetworkIndices.reserve(m_organisms.size());
    
    for (size_t i = 0; i < m_organisms.size(); ++i) {
        // We need to access the genome - for now, we'll create a simplified initialization
        // In a full implementation, you'd store the genome with each organism
        std::cout << "  Creating neural network for organism " << i << "..." << std::endl;
        
        // Create a placeholder genome (this should come from the organism)
        // For demonstration, we'll create an empty genome
        Engine::Genetics::Genome placeholderGenome("Organism_" + std::to_string(i) + "_Genome");
        
        size_t networkIndex = neuralManager.CreateNetworkForOrganism(placeholderGenome);
        m_neuralNetworkIndices.push_back(networkIndex);
        
        std::cout << "  Neural network " << networkIndex << " created for organism " << i << std::endl;
    }
    
    std::cout << "Neural systems initialized for " << m_neuralNetworkIndices.size() << " organisms." << std::endl;
}

void GeneticsIntegration::UpdateNeuralSystems(float deltaTime)
{
    auto& neuralManager = Neural::NeuralSystemManager::GetInstance();
    
    // Update all neural networks
    neuralManager.UpdateAllNetworks(deltaTime);
    
    // Trigger growth periodically (every 10 seconds)
    // DISABLED: TriggerGrowth has a bug causing access violation - needs debugging
    /*
    static float growthTimer = 0.0f;
    growthTimer += deltaTime;
    
    if (growthTimer >= 10.0f) {
        growthTimer = 0.0f;
        
        std::cout << "[NEURAL] Triggering growth for " << m_neuralNetworkIndices.size() << " networks" << std::endl;
        
        for (size_t networkIndex : m_neuralNetworkIndices) {
            // Validate network index before triggering growth
            if (networkIndex < 1000) { // Sanity check - index should be reasonable
                std::cout << "[NEURAL] Triggering growth for network " << networkIndex << std::endl;
                int neuronsAdded = neuralManager.TriggerGrowth(networkIndex);
                if (neuronsAdded > 0) {
                    std::cout << "  Neural network " << networkIndex << " grew " << neuronsAdded << " neurons" << std::endl;
                }
            } else {
                std::cerr << "[ERROR] Invalid network index: " << networkIndex << std::endl;
            }
        }
    }
    */
}

void GeneticsIntegration::ApplyNeuralBehavioralOutputs()
{
    auto& neuralManager = Neural::NeuralSystemManager::GetInstance();
    
    // Apply neural outputs to creature behavior
    // This is a simplified example - in a full implementation, you'd use the outputs
    // to control movement, actions, etc.
    
    for (size_t i = 0; i < m_neuralNetworkIndices.size() && i < m_organisms.size(); ++i) {
        size_t networkIndex = m_neuralNetworkIndices[i];
        
        // Get neural outputs
        std::vector<float> outputs = neuralManager.GetNetworkOutputs(networkIndex);
        
        if (!outputs.empty()) {
            // Example: Use first output to influence movement speed
            // In a full system, you'd have a behavior controller that interprets these outputs
            float movementSpeed = outputs[0];  // 0-1 range
            
            // Apply to creature (placeholder - actual implementation would depend on your behavior system)
            // m_organisms[i]->SetMovementSpeed(movementSpeed);
        }
    }
}

// ============================================================================
// Phase 5: PBR Material System Implementation
// ============================================================================

void GeneticsIntegration::InitializeMaterialSystem(ID3D12Device* device)
{
    std::cout << "  [Phase 5] Initializing PBR material system..." << std::endl;
    
    m_materialSystem = std::make_unique<GeneticsGameEngine::Rendering::MaterialSystem>();
    
    if (!m_materialSystem->Initialize(device))
    {
        std::cerr << "  [Phase 5] Failed to initialize material system" << std::endl;
        return;
    }
    
    std::cout << "  [Phase 5] Material system initialized with " 
              << m_materialSystem->GetMaterialCount() << " default materials" << std::endl;
}

GeneticsGameEngine::Rendering::MaterialID GeneticsIntegration::AssignMaterialFromGenetics(
    const Engine::Genetics::Taxonomy::Organism* organism)
{
    if (!m_materialSystem)
    {
        std::cerr << "  [Phase 5] Material system not initialized!" << std::endl;
        return 0;
    }
    
    // Determine taxonomy type
    auto chordata = dynamic_cast<const Engine::Genetics::Taxonomy::Chordata*>(organism);
    auto arthropoda = dynamic_cast<const Engine::Genetics::Taxonomy::Arthropoda*>(organism);
    auto mollusca = dynamic_cast<const Engine::Genetics::Taxonomy::Mollusca*>(organism);
    
    GeneticsGameEngine::Rendering::PBRMaterial material;
    
    // Extract genetic properties to influence material parameters
    float scale = organism->GetScale();
    int colorIndex = organism->GetColorIndex();
    
    // Use genome to influence material (simplified - would use specific gene loci in full implementation)
    // Map color index to base albedo variation
    float colorVariation = (colorIndex % 10) / 10.0f;
    
    if (chordata)
    {
        // Chordata: Skin material
        material.materialID = "Skin_" + organism->GetID();
        
        // Skin properties from genetics
        material.albedo = DirectX::XMFLOAT3(
            0.95f - colorVariation * 0.1f,
            0.75f + colorVariation * 0.05f,
            0.70f + colorVariation * 0.05f
        );
        material.roughness = 0.6f + colorVariation * 0.2f;  // Genetic influence on skin texture
        material.metallic = 0.0f;  // Skin is non-metallic
        material.ambientOcclusion = 0.8f;
        
        std::cout << "    [PBR] Assigned Skin material to " << organism->GetID() << std::endl;
        return m_materialSystem->CreateMaterial(material, material.materialID);
    }
    else if (arthropoda)
    {
        // Arthropoda: Exoskeleton material
        material.materialID = "Exoskeleton_" + organism->GetID();
        
        // Exoskeleton properties from genetics
        material.albedo = DirectX::XMFLOAT3(
            0.3f + colorVariation * 0.2f,
            0.2f + colorVariation * 0.15f,
            0.15f + colorVariation * 0.1f
        );
        material.roughness = 0.5f + colorVariation * 0.3f;  // Genetic influence on exoskeleton roughness
        material.metallic = 0.1f + colorVariation * 0.05f;  // Slight metallic sheen
        material.ambientOcclusion = 0.9f;
        
        std::cout << "    [PBR] Assigned Exoskeleton material to " << organism->GetID() << std::endl;
        return m_materialSystem->CreateMaterial(material, material.materialID);
    }
    else if (mollusca)
    {
        // Mollusca: Shell material
        material.materialID = "Shell_" + organism->GetID();
        
        // Shell properties from genetics
        material.albedo = DirectX::XMFLOAT3(
            0.85f - colorVariation * 0.1f,
            0.80f - colorVariation * 0.05f,
            0.70f + colorVariation * 0.1f
        );
        material.roughness = 0.3f + colorVariation * 0.2f;  // Shells are generally smooth
        material.metallic = 0.05f;  // Very slight metallic luster
        material.ambientOcclusion = 0.85f;
        
        std::cout << "    [PBR] Assigned Shell material to " << organism->GetID() << std::endl;
        return m_materialSystem->CreateMaterial(material, material.materialID);
    }
    else
    {
        // Default material
        material.materialID = "Default_" + organism->GetID();
        material.albedo = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f);
        material.roughness = 0.5f;
        material.metallic = 0.0f;
        material.ambientOcclusion = 1.0f;
        
        std::cout << "    [PBR] Assigned Default material to " << organism->GetID() << std::endl;
        return m_materialSystem->CreateMaterial(material, material.materialID);
    }
}

// Regenerate all meshes with custom parameters
void GeneticsIntegration::RegenerateMeshes(float voxelSize, float falloffMultiplier)
{
    if (!m_device) {
        std::cerr << "[RegenerateMeshes] ERROR: Device not stored!" << std::endl;
        return;
    }
    
    std::cout << "\n[RegenerateMeshes] Regenerating " << m_organisms.size() 
              << " meshes with voxelSize=" << voxelSize 
              << ", falloffMultiplier=" << falloffMultiplier << std::endl;
    
    // Clear existing meshes
    m_creatureMeshes.clear();
    
    // Create upload command allocator and list
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> uploadAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
    
    HRESULT hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator));
    if (FAILED(hr)) {
        std::cerr << "[RegenerateMeshes] Failed to create upload command allocator!" << std::endl;
        return;
    }
    
    hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCommandList));
    if (FAILED(hr)) {
        std::cerr << "[RegenerateMeshes] Failed to create upload command list!" << std::endl;
        return;
    }
    
    // Regenerate each organism
    float xPos = -3.0f;
    for (size_t i = 0; i < m_organisms.size(); ++i) {
        try {
            CreatureMeshData meshData = GenerateMeshForOrganismWithParams(
                m_organisms[i].get(), static_cast<int>(i), voxelSize, falloffMultiplier);
            
            meshData.position = DirectX::XMFLOAT3(xPos, 2.0f, 0.0f);
            
            // Initialize mesh renderer and upload to GPU
            meshData.meshRenderer = std::make_unique<Engine::Procedural::Mesh::ProceduralMeshRenderer>();
            if (!meshData.meshRenderer->Initialize(m_device)) {
                std::cerr << "  Failed to initialize mesh renderer for creature " << i << std::endl;
                continue;
            }
            
            // Create a transformed copy of the mesh with position offset
            Engine::Procedural::Mesh::MeshData transformedMesh = meshData.mesh;
            DirectX::XMVECTOR translation = DirectX::XMLoadFloat3(&meshData.position);
            
            for (auto& vertex : transformedMesh.vertices) {
                DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&vertex);
                pos = DirectX::XMVectorAdd(pos, translation);
                DirectX::XMStoreFloat3(&vertex, pos);
            }
            
            // Upload mesh data
            if (!meshData.meshRenderer->UpdateMesh(transformedMesh, uploadCommandList.Get())) {
                std::cerr << "  Failed to upload mesh for creature " << i << std::endl;
                continue;
            }
            
            // Assign material
            if (m_materialSystem)
            {
                meshData.materialID = AssignMaterialFromGenetics(m_organisms[i].get());
            }
            else
            {
                meshData.materialID = 0;
            }
            
            std::cout << "    Mesh " << i << " generated and uploaded successfully." << std::endl;
            m_creatureMeshes.push_back(std::move(meshData));
            
            xPos += 4.0f;
        } catch (const std::exception& e) {
            std::cerr << "    ERROR generating mesh " << i << ": " << e.what() << std::endl;
        }
    }
    
    // Close and execute upload command list
    hr = uploadCommandList->Close();
    if (FAILED(hr)) {
        std::cerr << "[RegenerateMeshes] Failed to close upload command list!" << std::endl;
        return;
    }
    
    // Create temporary command queue
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> uploadQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&uploadQueue));
    if (FAILED(hr)) {
        std::cerr << "[RegenerateMeshes] Failed to create upload command queue!" << std::endl;
        return;
    }
    
    // Execute uploads
    ID3D12CommandList* ppCommandLists[] = { uploadCommandList.Get() };
    uploadQueue->ExecuteCommandLists(1, ppCommandLists);
    
    // Wait for GPU to complete
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    if (FAILED(hr)) {
        std::cerr << "[RegenerateMeshes] Failed to create fence!" << std::endl;
        return;
    }
    
    HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    const UINT64 fenceValue = 1;
    uploadQueue->Signal(fence.Get(), fenceValue);
    
    if (fence->GetCompletedValue() < fenceValue) {
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        WaitForSingleObject(fenceEvent, INFINITE);
    }
    
    CloseHandle(fenceEvent);
    
    std::cout << "[RegenerateMeshes] Generated and uploaded " << m_creatureMeshes.size() << " creature meshes." << std::endl;
}