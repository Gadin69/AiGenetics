#pragma once

#include <cstdint>
#include <string>
#include <random>

namespace Engine {
namespace Neural {

// Types of neurons in the network
enum class NeuronType : uint8_t {
    General,        // General purpose neuron
    Mutation,       // Activated by genetic mutations
    Learning,       // Involved in learning processes
    Movement,       // Movement control
    Visual,         // Visual processing
    Sensory,        // Environmental input (NEW)
    Motor           // Behavior output (NEW)
};

// Configuration constants for neural network
struct NeuralConfig {
    static constexpr float DefaultActivationThreshold = 0.5f;
    static constexpr float MutationGrowthThreshold = 0.3f;
    static constexpr float LearningGrowthThreshold = 0.4f;
    static constexpr int MaxNeuronGrowthPerGeneration = 100;
    static constexpr float InitialWeightRange = 0.5f;  // Weights initialized in [-0.5, 0.5]
};

// Represents a single neuron in the neural network
// Can be dynamically added or removed based on genetic triggers
struct Neuron {
    uint32_t id;                    // Unique identifier
    float activation;               // Current activation level (0.0 to 1.0)
    float threshold;                // Activation threshold
    NeuronType type;                // Type of neuron
    bool isActive;                  // Whether neuron is currently active
    
    Neuron() 
        : id(0)
        , activation(0.0f)
        , threshold(0.5f)
        , type(NeuronType::General)
        , isActive(true)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> actDist(0.0f, 1.0f);
        static std::uniform_real_distribution<float> threshDist(0.2f, 0.7f);
        static uint32_t nextId = 0;
        
        id = nextId++;
        activation = actDist(gen);
        threshold = threshDist(gen);
    }
    
    Neuron(NeuronType neuronType)
        : Neuron()
    {
        type = neuronType;
    }
    
    // Check if neuron should fire based on activation and threshold
    bool ShouldFire() const {
        return isActive && activation >= threshold;
    }
    
    // Apply activation function (sigmoid)
    void ApplyActivationFunction() {
        activation = 1.0f / (1.0f + std::exp(-activation));
    }
};

} // namespace Neural
} // namespace Engine
