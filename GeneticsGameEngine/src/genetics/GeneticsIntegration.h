#pragma once

#include <vector>
#include <memory>
#include <string>
#include <windows.h>
#include <d3d12.h>

// Forward declarations
class GraphicsEngine;
class GeneticsCreature;

// C++ wrapper classes for genetics data
struct Genome;
struct DynamicNeuralNetwork;

// Simple placeholder definitions for procedural parameters
struct MeshParameters {
    float scale = 1.0f;
    int vertexCount = 0;
    int indexCount = 0;
};

struct MovementParameters {
    float speed = 1.0f;
    float acceleration = 0.1f;
    float rotationSpeed = 0.01f;
};

// Include actual C++ header files (not C#)
#include "../graphics/GraphicsEngine.h"

// Forward declarations for C++ genetics classes
struct GeneticsCreature;

// Forward declarations for engine classes
namespace Engine {
    namespace Rendering {
        class BaseCameraController;
    }
}

class GeneticsIntegration
{
private:
    std::vector<std::unique_ptr<GeneticsCreature>> m_creatures;
    
public:
    bool Initialize();
    void Update(float deltaTime);
    void Render(GraphicsEngine* graphicsEngine);
    
    void AddCreature(const std::string& creatureType, const std::string& genomeId);
};

// Wrapper for genetics creatures
struct GeneticsCreature
{
    std::string type;
    std::string id;
    
    // Genetic data
    struct Genome* genome;
    struct DynamicNeuralNetwork* neuralNetwork;
    
    // Procedural data
    struct MeshParameters meshParams;
    struct MovementParameters movementParams;
    
    // Rendering data
    ID3D12Resource* meshBuffer;
    ID3D12Resource* instanceBuffer;
    
    GeneticsCreature() : genome(nullptr), neuralNetwork(nullptr), 
        meshBuffer(nullptr), instanceBuffer(nullptr) {}
    ~GeneticsCreature();
};