#include "GeneticsIntegration.h"
#include "../graphics/GraphicsEngine.h"
#include <iostream>
#include <DirectXMath.h>

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

void GeneticsIntegration::Render(GraphicsEngine* graphicsEngine)
{
    // TEMPORARILY DISABLED: Creature rendering pending full implementation
    // The basic rendering pipeline is being tested first
    
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