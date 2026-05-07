#pragma once

#include <cstdint>

namespace Engine {
namespace Neural {

// Represents a synaptic connection between two neurons
struct Synapse {
    uint32_t fromNeuronIdx;   // Index of source neuron in network's neuron array
    uint32_t toNeuronIdx;     // Index of target neuron in network's neuron array
    float weight;              // Connection weight (-1.0 to 1.0)
    float lastActivity;        // Track recent activity for pruning
    
    Synapse()
        : fromNeuronIdx(0)
        , toNeuronIdx(0)
        , weight(0.0f)
        , lastActivity(0.0f)
    {}
    
    Synapse(uint32_t fromIdx, uint32_t toIdx, float initialWeight)
        : fromNeuronIdx(fromIdx)
        , toNeuronIdx(toIdx)
        , weight(initialWeight)
        , lastActivity(0.0f)
    {}
    
    // Check if synapse is effectively inactive (very small weight)
    bool IsInactive(float threshold = 0.01f) const {
        return std::abs(weight) < threshold;
    }
};

} // namespace Neural
} // namespace Engine
