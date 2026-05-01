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

// Include actual C++ header files
#include "../graphics/GraphicsEngine.h"

// Forward declarations for engine classes
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

class GeneticsIntegration
{
private:
    std::vector<std::unique_ptr<Engine::Genetics::Taxonomy::Organism>> m_organisms;
    
public:
    bool Initialize();
    void Update(float deltaTime);
    void Render(GraphicsEngine* graphicsEngine);
    
    void CreateSampleCreatures();
    void TestBreedingSystem();
    
private:
    DirectX::XMFLOAT4 GetColorFromIndex(int index);
};