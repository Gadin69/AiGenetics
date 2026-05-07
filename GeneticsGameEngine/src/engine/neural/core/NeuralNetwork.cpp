#include "NeuralNetwork.h"
#include "../genetics/genome/Genome.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

namespace Engine {
namespace Neural {

NeuralNetwork::NeuralNetwork()
    : m_activityLevel(0.0f)
    , m_adjacencyListDirty(false)
{
    // Reserve initial capacity to avoid frequent reallocations
    m_neurons.reserve(64);
    m_synapses.reserve(256);
}

NeuralNetwork::~NeuralNetwork()
{
    Clear();
}

void NeuralNetwork::AddNeuron(const Neuron& neuron)
{
    m_neurons.push_back(neuron);
    
    // Update input/output indices if needed
    if (neuron.type == NeuronType::Sensory) {
        m_inputNeuronIndices.push_back(static_cast<uint32_t>(m_neurons.size() - 1));
    } else if (neuron.type == NeuronType::Motor) {
        m_outputNeuronIndices.push_back(static_cast<uint32_t>(m_neurons.size() - 1));
    }
    
    // Expand adjacency list
    m_adjacencyList.push_back(std::vector<size_t>());
}

void NeuralNetwork::AddNeuron(Neuron&& neuron)
{
    uint32_t index = static_cast<uint32_t>(m_neurons.size());
    m_neurons.push_back(std::move(neuron));
    
    // Update input/output indices if needed
    if (m_neurons.back().type == NeuronType::Sensory) {
        m_inputNeuronIndices.push_back(index);
    } else if (m_neurons.back().type == NeuronType::Motor) {
        m_outputNeuronIndices.push_back(index);
    }
    
    // Expand adjacency list
    m_adjacencyList.push_back(std::vector<size_t>());
}

void NeuralNetwork::AddSynapse(uint32_t fromNeuronIdx, uint32_t toNeuronIdx, float weight)
{
    // Validate indices
    if (fromNeuronIdx >= m_neurons.size() || toNeuronIdx >= m_neurons.size()) {
        return;  // Invalid indices
    }
    
    m_synapses.emplace_back(fromNeuronIdx, toNeuronIdx, weight);
    m_adjacencyListDirty = true;
}

void NeuralNetwork::RemoveSynapse(size_t synapseIndex)
{
    if (synapseIndex < m_synapses.size()) {
        // Swap with last element and pop for O(1) removal
        m_synapses[synapseIndex] = m_synapses.back();
        m_synapses.pop_back();
        m_adjacencyListDirty = true;
    }
}

int NeuralNetwork::GrowNeurons(const Engine::Genetics::Genome& genome, float activityThreshold)
{
    // Only grow if activity threshold is met
    if (m_activityLevel < activityThreshold) {
        return 0;
    }
    
    // Calculate growth potential from genome
    // This is a simplified version - in practice, you'd query specific neural-related genes
    int totalGrowthPotential = 0;
    
    // Count active genes with neural growth factors
    for (const auto& chromosome : genome.GetChromosomes()) {
        for (const auto& gene : chromosome.GetGenes()) {
            // Simplified: check if gene is active and has significant expression
            // Gene is considered active if it's dominant or co-dominant
            bool isActive = (gene.dominance == Genetics::DominanceType::Dominant || 
                           gene.dominance == Genetics::DominanceType::CoDominant);
            
            if (isActive && gene.alleleValue > 0.3f) {
                // Check if this gene affects neural growth
                // In a full implementation, you'd check specific locus IDs
                uint16_t locusID = gene.locusID;
                // Neural-related loci ranges (example: 0xA000-0xAFFF)
                if (locusID >= 0xA000 && locusID <= 0xAFFF) {
                    totalGrowthPotential += static_cast<int>(gene.alleleValue * 5.0f);
                }
            }
        }
    }
    
    // Limit growth to prevent runaway expansion
    int maxGrowth = std::min(totalGrowthPotential, NeuralConfig::MaxNeuronGrowthPerGeneration);
    
    int neuronsAdded = 0;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    for (int i = 0; i < maxGrowth; ++i) {
        // Determine neuron type based on genetic context
        NeuronType newType = DetermineNeuronTypeFromGenome(genome);
        
        // Create and add new neuron
        Neuron newNeuron(newType);
        AddNeuron(std::move(newNeuron));
        neuronsAdded++;
        
        // Create random connections to existing neurons
        if (m_neurons.size() > 1) {
            std::uniform_int_distribution<uint32_t> neuronDist(0, static_cast<uint32_t>(m_neurons.size()) - 2);
            std::uniform_real_distribution<float> weightDist(-NeuralConfig::InitialWeightRange, NeuralConfig::InitialWeightRange);
            
            // Connect to 1-3 existing neurons
            int connectionCount = std::min(3, static_cast<int>(m_neurons.size()) - 1);
            std::uniform_int_distribution<int> connCountDist(1, connectionCount);
            int numConnections = connCountDist(gen);
            
            for (int j = 0; j < numConnections; ++j) {
                uint32_t targetIdx = neuronDist(gen);
                float weight = weightDist(gen);
                AddSynapse(static_cast<uint32_t>(m_neurons.size()) - 1, targetIdx, weight);
            }
        }
    }
    
    return neuronsAdded;
}

int NeuralNetwork::PruneInactiveSynapses(float weightThreshold)
{
    int pruned = 0;
    
    // Remove synapses with weights below threshold
    auto it = m_synapses.begin();
    while (it != m_synapses.end()) {
        if (it->IsInactive(weightThreshold)) {
            it = m_synapses.erase(it);
            pruned++;
            m_adjacencyListDirty = true;
        } else {
            ++it;
        }
    }
    
    return pruned;
}

void NeuralNetwork::UpdateActivity()
{
    // Calculate average activation of all neurons
    if (m_neurons.empty()) {
        m_activityLevel = 0.0f;
        return;
    }
    
    float totalActivation = 0.0f;
    for (const auto& neuron : m_neurons) {
        totalActivation += neuron.activation;
    }
    
    m_activityLevel = totalActivation / static_cast<float>(m_neurons.size());
}

void NeuralNetwork::ForwardPass(const std::vector<float>& inputs)
{
    // Rebuild adjacency list if needed
    if (m_adjacencyListDirty) {
        BuildAdjacencyList();
        m_adjacencyListDirty = false;
    }
    
    // Set input neuron activations
    for (size_t i = 0; i < m_inputNeuronIndices.size() && i < inputs.size(); ++i) {
        uint32_t neuronIdx = m_inputNeuronIndices[i];
        if (neuronIdx < m_neurons.size()) {
            m_neurons[neuronIdx].activation = inputs[i];
        }
    }
    
    // Propagate activations through the network
    // Simple feedforward: iterate through neurons and update based on incoming connections
    for (size_t i = 0; i < m_neurons.size(); ++i) {
        Neuron& neuron = m_neurons[i];
        
        if (neuron.type == NeuronType::Sensory) {
            continue;  // Input neurons already set
        }
        
        // Sum weighted inputs from connected neurons
        float weightedSum = 0.0f;
        for (size_t synapseIdx : m_adjacencyList[i]) {
            if (synapseIdx < m_synapses.size()) {
                const Synapse& synapse = m_synapses[synapseIdx];
                if (synapse.fromNeuronIdx < m_neurons.size()) {
                    weightedSum += m_neurons[synapse.fromNeuronIdx].activation * synapse.weight;
                }
            }
        }
        
        // Apply activation if threshold is met
        if (std::abs(weightedSum) >= neuron.threshold) {
            neuron.activation = weightedSum;
            neuron.ApplyActivationFunction();
        }
    }
    
    // Update activity level
    UpdateActivity();
}

std::vector<float> NeuralNetwork::GetOutputs() const
{
    std::vector<float> outputs;
    outputs.reserve(m_outputNeuronIndices.size());
    
    for (uint32_t neuronIdx : m_outputNeuronIndices) {
        if (neuronIdx < m_neurons.size()) {
            outputs.push_back(m_neurons[neuronIdx].activation);
        }
    }
    
    return outputs;
}

void NeuralNetwork::Clear()
{
    m_neurons.clear();
    m_synapses.clear();
    m_inputNeuronIndices.clear();
    m_outputNeuronIndices.clear();
    m_adjacencyList.clear();
    m_activityLevel = 0.0f;
    m_adjacencyListDirty = false;
}

void NeuralNetwork::Reserve(size_t neuronCount, size_t synapseCount)
{
    m_neurons.reserve(neuronCount);
    m_synapses.reserve(synapseCount);
    m_adjacencyList.reserve(neuronCount);
}

NeuronType NeuralNetwork::DetermineNeuronTypeFromGenome(const Engine::Genetics::Genome& genome) const
{
    // Simplified implementation - in a full system, you'd check specific loci
    // For now, randomly assign types based on genome characteristics
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    float value = dist(gen);
    
    if (value < 0.4f) {
        return NeuronType::General;
    } else if (value < 0.55f) {
        return NeuronType::Movement;
    } else if (value < 0.7f) {
        return NeuronType::Sensory;
    } else if (value < 0.85f) {
        return NeuronType::Motor;
    } else {
        return NeuronType::Learning;
    }
}

void NeuralNetwork::BuildAdjacencyList()
{
    // Clear existing adjacency list
    m_adjacencyList.clear();
    m_adjacencyList.resize(m_neurons.size());
    
    // Build adjacency list from synapses
    // For each neuron, store indices of synapses where it's the source
    for (size_t i = 0; i < m_synapses.size(); ++i) {
        uint32_t fromIdx = m_synapses[i].fromNeuronIdx;
        if (fromIdx < m_neurons.size()) {
            m_adjacencyList[fromIdx].push_back(i);
        }
    }
}

} // namespace Neural
} // namespace Engine
