#pragma once

#include "../core/NeuralNetwork.h"

namespace Engine {
namespace Neural {

// Handles synaptic pruning to prevent memory bloat and remove unused connections
class PruningEngine {
public:
    PruningEngine(NeuralNetwork& network);
    
    // Prune synapses with weights below threshold
    int PruneWeakSynapses(float weightThreshold = 0.01f);
    
    // Prune based on activity (remove connections from inactive neurons)
    int PruneInactiveConnections(float activityThreshold = 0.1f);
    
    // Comprehensive pruning: combine weak and inactive pruning
    int ExecutePruning(float weightThreshold = 0.01f, float activityThreshold = 0.1f);
    
    // Get statistics about the network
    struct PruningStats {
        int totalSynapses;
        int prunedSynapses;
        float averageWeight;
        float inactivePercentage;
    };
    
    PruningStats GetStats() const;

private:
    NeuralNetwork& m_neuralNetwork;
};

} // namespace Neural
} // namespace Engine
