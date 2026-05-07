#pragma once

#include "core/NeuralNetwork.h"
#include "growth/GrowthController.h"
#include "growth/PruningEngine.h"
#include "learning/ReinforcementLearner.h"
#include "learning/FitnessEvaluator.h"
#include "optimization/BatchProcessor.h"
#include "../genetics/genome/Genome.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace Engine {
namespace Neural {

// Manages all neural networks in the game
// Provides integration point between genetics, learning, and behavior systems
class NeuralSystemManager {
public:
    static NeuralSystemManager& GetInstance();
    
    // Initialize the neural system
    void Initialize();
    
    // Create a neural network for an organism
    size_t CreateNetworkForOrganism(const Engine::Genetics::Genome& genome);
    
    // Update all neural networks (called each frame)
    void UpdateAllNetworks(float deltaTime);
    
    // Trigger growth for a specific network
    int TriggerGrowth(size_t networkIndex);
    
    // Apply learning with reward
    void ApplyLearning(size_t networkIndex, float reward);
    
    // Execute forward pass with inputs
    void ExecuteForwardPass(size_t networkIndex, const std::vector<float>& inputs);
    
    // Get outputs from a network
    std::vector<float> GetNetworkOutputs(size_t networkIndex) const;
    
    // Get network by index
    NeuralNetwork* GetNetwork(size_t index);
    
    // Get total network count
    size_t GetNetworkCount() const { return m_networks.size(); }
    
    // Prune inactive synapses across all networks
    int PruneAllNetworks(float weightThreshold = 0.01f);
    
    // Get batch processor for performance statistics
    const BatchProcessor& GetBatchProcessor() const { return m_batchProcessor; }
    
    // Clear all networks
    void Clear();

private:
    NeuralSystemManager();
    ~NeuralSystemManager();
    
    // Prevent copying
    NeuralSystemManager(const NeuralSystemManager&) = delete;
    NeuralSystemManager& operator=(const NeuralSystemManager&) = delete;
    
    struct NetworkContext {
        std::unique_ptr<NeuralNetwork> network;
        std::unique_ptr<Engine::Genetics::Genome> genome;  // OWN the genome
        std::unique_ptr<GrowthController> growthController;
        std::unique_ptr<PruningEngine> pruningEngine;
        std::unique_ptr<ReinforcementLearner> learner;
        std::unique_ptr<FitnessEvaluator> fitnessEvaluator;
        float survivalTime;
        float movementEfficiency;
        float goalAchievement;
        float energyConsumption;
        
        NetworkContext();
    };
    
    std::vector<NetworkContext> m_networks;
    BatchProcessor m_batchProcessor;
    
    int m_pruningInterval;  // Frames between pruning operations
    int m_frameCounter;
    
    // Initialize a single network with default sensory/motor neurons
    void InitializeNetwork(NetworkContext& context);
};

} // namespace Neural
} // namespace Engine
