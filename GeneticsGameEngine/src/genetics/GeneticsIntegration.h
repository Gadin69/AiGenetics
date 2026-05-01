#pragma once

#include <vector>
#include <memory>
#include "../../GeneticsGame/Core/Gene.cs"
#include "../../GeneticsGame/Core/Chromosome.cs"
#include "../../GeneticsGame/Core/Genome.cs"
#include "../../GeneticsGame/Systems/DynamicNeuralNetwork.cs"
#include "../../GeneticsGame/Systems/Neuron.cs"
#include "../../GeneticsGame/Systems/Connection.cs"
#include "../../GeneticsGame/Procedural/Mesh/MeshProceduralGenerator.cs"
#include "../../GeneticsGame/Procedural/Movement/MovementProceduralGenerator.cs"
#include "../../GeneticsGame/Procedural/SynchronizedGenerator.cs"

// Forward declarations
class GraphicsEngine;
class GeneticsCreature;

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
    Genome* genome;
    DynamicNeuralNetwork* neuralNetwork;
    
    // Procedural data
    MeshParameters meshParams;
    MovementParameters movementParams;
    
    // Rendering data
    ID3D12Resource* meshBuffer;
    ID3D12Resource* instanceBuffer;
    
    GeneticsCreature() : genome(nullptr), neuralNetwork(nullptr), 
        meshBuffer(nullptr), instanceBuffer(nullptr) {}
    ~GeneticsCreature();
};