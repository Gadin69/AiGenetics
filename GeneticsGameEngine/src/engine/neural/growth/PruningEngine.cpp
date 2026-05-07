#include "PruningEngine.h"
#include <algorithm>
#include <numeric>

namespace Engine {
namespace Neural {

PruningEngine::PruningEngine(NeuralNetwork& network)
    : m_neuralNetwork(network)
{
}

int PruningEngine::PruneWeakSynapses(float weightThreshold)
{
    return m_neuralNetwork.PruneInactiveSynapses(weightThreshold);
}

int PruningEngine::PruneInactiveConnections(float activityThreshold)
{
    int pruned = 0;
    const auto& neurons = m_neuralNetwork.GetNeurons();
    const auto& synapses = m_neuralNetwork.GetSynapses();
    
    // Identify synapses connecting inactive neurons
    std::vector<size_t> synapsesToRemove;
    
    for (size_t i = 0; i < synapses.size(); ++i) {
        const auto& synapse = synapses[i];
        
        // Check if both connected neurons are inactive
        if (synapse.fromNeuronIdx < neurons.size() && synapse.toNeuronIdx < neurons.size()) {
            const auto& fromNeuron = neurons[synapse.fromNeuronIdx];
            const auto& toNeuron = neurons[synapse.toNeuronIdx];
            
            if (fromNeuron.activation < activityThreshold && toNeuron.activation < activityThreshold) {
                synapsesToRemove.push_back(i);
            }
        }
    }
    
    // Remove identified synapses (in reverse order to maintain indices)
    std::sort(synapsesToRemove.rbegin(), synapsesToRemove.rend());
    for (size_t idx : synapsesToRemove) {
        m_neuralNetwork.RemoveSynapse(idx);
        pruned++;
    }
    
    return pruned;
}

int PruningEngine::ExecutePruning(float weightThreshold, float activityThreshold)
{
    int totalPruned = 0;
    
    // First prune weak synapses
    totalPruned += PruneWeakSynapses(weightThreshold);
    
    // Then prune inactive connections
    totalPruned += PruneInactiveConnections(activityThreshold);
    
    return totalPruned;
}

PruningEngine::PruningStats PruningEngine::GetStats() const
{
    PruningStats stats;
    const auto& synapses = m_neuralNetwork.GetSynapses();
    
    stats.totalSynapses = static_cast<int>(synapses.size());
    stats.prunedSynapses = 0;  // Not tracked in this call
    
    if (synapses.empty()) {
        stats.averageWeight = 0.0f;
        stats.inactivePercentage = 0.0f;
        return stats;
    }
    
    // Calculate average weight
    float totalWeight = 0.0f;
    int inactiveCount = 0;
    
    for (const auto& synapse : synapses) {
        totalWeight += std::abs(synapse.weight);
        if (synapse.IsInactive()) {
            inactiveCount++;
        }
    }
    
    stats.averageWeight = totalWeight / static_cast<float>(synapses.size());
    stats.inactivePercentage = static_cast<float>(inactiveCount) / static_cast<float>(synapses.size()) * 100.0f;
    
    return stats;
}

} // namespace Neural
} // namespace Engine
