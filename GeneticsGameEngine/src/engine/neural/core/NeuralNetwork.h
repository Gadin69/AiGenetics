#pragma once

#include "Neuron.h"
#include "Synapse.h"
#include <vector>
#include <memory>
#include <string>

namespace Engine {
namespace Genetics {
    class Genome;  // Forward declaration
}

namespace Neural {

// Dynamic neural network that can grow new neurons at runtime
// Optimized for performance with adjacency list representation
class NeuralNetwork {
public:
    NeuralNetwork();
    ~NeuralNetwork();
    
    // Neuron management
    void AddNeuron(const Neuron& neuron);
    void AddNeuron(Neuron&& neuron);
    
    // Synapse management
    void AddSynapse(uint32_t fromNeuronIdx, uint32_t toNeuronIdx, float weight = 1.0f);
    void RemoveSynapse(size_t synapseIndex);
    
    // Growth and pruning
    int GrowNeurons(const Engine::Genetics::Genome& genome, float activityThreshold = 0.5f);
    int PruneInactiveSynapses(float weightThreshold = 0.01f);
    
    // Network state updates
    void UpdateActivity();
    void ForwardPass(const std::vector<float>& inputs);
    
    // Accessors
    const std::vector<Neuron>& GetNeurons() const { return m_neurons; }
    const std::vector<Synapse>& GetSynapses() const { return m_synapses; }
    const std::vector<uint32_t>& GetInputNeuronIndices() const { return m_inputNeuronIndices; }
    const std::vector<uint32_t>& GetOutputNeuronIndices() const { return m_outputNeuronIndices; }
    
    size_t GetNeuronCount() const { return m_neurons.size(); }
    size_t GetSynapseCount() const { return m_synapses.size(); }
    float GetActivityLevel() const { return m_activityLevel; }
    
    // Get outputs from motor/output neurons
    std::vector<float> GetOutputs() const;
    
    // Clear all neurons and synapses
    void Clear();
    
    // Reserve memory for performance
    void Reserve(size_t neuronCount, size_t synapseCount);

private:
    std::vector<Neuron> m_neurons;
    std::vector<Synapse> m_synapses;
    float m_activityLevel;
    
    // Indices into m_neurons for quick access
    std::vector<uint32_t> m_inputNeuronIndices;   // Sensory neurons
    std::vector<uint32_t> m_outputNeuronIndices;  // Motor neurons
    
    // Determine neuron type based on genetic context
    NeuronType DetermineNeuronTypeFromGenome(const Engine::Genetics::Genome& genome) const;
    
    // Build adjacency list for efficient forward propagation
    void BuildAdjacencyList();
    
    // Adjacency list: for each neuron, store indices of outgoing synapses
    std::vector<std::vector<size_t>> m_adjacencyList;
    bool m_adjacencyListDirty;  // Flag to rebuild adjacency list when synapses change
};

} // namespace Neural
} // namespace Engine
