#include "BatchProcessor.h"
#include <algorithm>
#include <numeric>
#include <iostream>

namespace Engine {
namespace Neural {

BatchProcessor::BatchProcessor()
    : m_batchesDirty(true)
{
}

BatchProcessor::~BatchProcessor()
{
    Clear();
}

void BatchProcessor::AddNetwork(NeuralNetwork* network)
{
    if (network) {
        m_networks.push_back(network);
        m_batchesDirty = true;
    }
}

void BatchProcessor::RemoveNetwork(NeuralNetwork* network)
{
    auto it = std::find(m_networks.begin(), m_networks.end(), network);
    if (it != m_networks.end()) {
        m_networks.erase(it);
        m_batchesDirty = true;
    }
}

void BatchProcessor::ExecuteBatchForwardPass(const std::vector<std::vector<float>>& inputs)
{
    // Rebuild batches if needed
    if (m_batchesDirty) {
        RebuildBatches();
        m_batchesDirty = false;
    }
    
    // Execute forward pass for each network
    for (size_t i = 0; i < m_networks.size() && i < inputs.size(); ++i) {
        if (m_networks[i]) {
            m_networks[i]->ForwardPass(inputs[i]);
        }
    }
}

void BatchProcessor::ExecuteSingleForwardPass(size_t networkIndex, const std::vector<float>& inputs)
{
    if (networkIndex < m_networks.size() && m_networks[networkIndex]) {
        m_networks[networkIndex]->ForwardPass(inputs);
    }
}

NeuralNetwork* BatchProcessor::GetNetwork(size_t index)
{
    if (index < m_networks.size()) {
        return m_networks[index];
    }
    return nullptr;
}

void BatchProcessor::Clear()
{
    m_networks.clear();
    m_batches.clear();
    m_batchesDirty = true;
}

BatchProcessor::BatchStats BatchProcessor::GetStats() const
{
    BatchStats stats;
    stats.totalNetworks = m_networks.size();
    stats.totalNeurons = 0;
    stats.totalSynapses = 0;
    
    for (const auto* network : m_networks) {
        if (network) {
            stats.totalNeurons += network->GetNeuronCount();
            stats.totalSynapses += network->GetSynapseCount();
        }
    }
    
    if (stats.totalNetworks > 0) {
        stats.averageNeuronsPerNetwork = static_cast<float>(stats.totalNeurons) / stats.totalNetworks;
        stats.averageSynapsesPerNetwork = static_cast<float>(stats.totalSynapses) / stats.totalNetworks;
    } else {
        stats.averageNeuronsPerNetwork = 0.0f;
        stats.averageSynapsesPerNetwork = 0.0f;
    }
    
    return stats;
}

void BatchProcessor::RebuildBatches()
{
    m_batches.clear();
    
    if (m_networks.empty()) {
        return;
    }
    
    // Simple batching: group all networks together
    // In a more sophisticated implementation, you'd group by topology similarity
    NetworkBatch batch;
    batch.neuronCount = 0;
    batch.synapseCount = 0;
    
    for (size_t i = 0; i < m_networks.size(); ++i) {
        batch.networkIndices.push_back(i);
        if (m_networks[i]) {
            batch.neuronCount += m_networks[i]->GetNeuronCount();
            batch.synapseCount += m_networks[i]->GetSynapseCount();
        }
    }
    
    m_batches.push_back(std::move(batch));
}

} // namespace Neural
} // namespace Engine
