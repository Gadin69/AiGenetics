#pragma once

#include "../core/NeuralNetwork.h"
#include <vector>
#include <memory>

namespace Engine {
namespace Neural {

// Batch processor for efficient multi-instance neural network execution
// Groups similar networks and processes them together for better cache utilization
class BatchProcessor {
public:
    BatchProcessor();
    ~BatchProcessor();
    
    // Add a network to the batch processor
    void AddNetwork(NeuralNetwork* network);
    
    // Remove a network from the batch processor
    void RemoveNetwork(NeuralNetwork* network);
    
    // Execute forward pass for all networks in batches
    void ExecuteBatchForwardPass(const std::vector<std::vector<float>>& inputs);
    
    // Execute forward pass for a single network
    void ExecuteSingleForwardPass(size_t networkIndex, const std::vector<float>& inputs);
    
    // Get network by index
    NeuralNetwork* GetNetwork(size_t index);
    
    // Get total number of networks
    size_t GetNetworkCount() const { return m_networks.size(); }
    
    // Clear all networks
    void Clear();
    
    // Performance statistics
    struct BatchStats {
        size_t totalNetworks;
        size_t totalNeurons;
        size_t totalSynapses;
        float averageNeuronsPerNetwork;
        float averageSynapsesPerNetwork;
    };
    
    BatchStats GetStats() const;

private:
    std::vector<NeuralNetwork*> m_networks;
    
    // Group networks by similar topology for batched execution
    struct NetworkBatch {
        std::vector<size_t> networkIndices;
        size_t neuronCount;
        size_t synapseCount;
    };
    
    std::vector<NetworkBatch> m_batches;
    bool m_batchesDirty;
    
    // Rebuild batches based on current networks
    void RebuildBatches();
};

} // namespace Neural
} // namespace Engine
