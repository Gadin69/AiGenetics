#pragma once

#include "../core/NeuralNetwork.h"
#include "../genetics/genome/Genome.h"
#include <memory>

namespace Engine {
namespace Neural {

// Manages neuron creation based on genetic and environmental triggers
// Implements hybrid triggering system (genetic expression + mutation + learning)
class GrowthController {
public:
    GrowthController(NeuralNetwork& network, const Engine::Genetics::Genome* genome);
    
    // Trigger neuron growth based on genetic expression
    int TriggerByGeneticExpression();
    
    // Trigger neuron growth based on mutation events
    int TriggerByMutation();
    
    // Trigger neuron growth based on learning feedback
    int TriggerByLearning(float learningRate = 0.1f);
    
    // Execute hybrid growth trigger system (all three mechanisms)
    int ExecuteHybridGrowthTrigger();
    
    // Set mutation trigger flag (called by MutationEngine)
    void NotifyMutationOccurred();
    
    // Set learning rate from external learning system
    void SetLearningRate(float rate);

private:
    NeuralNetwork& m_neuralNetwork;
    const Engine::Genetics::Genome* m_genome;  // Pointer to genome (must remain valid)
    
    bool m_mutationOccurred;
    float m_learningRate;
    
    // Configuration thresholds
    static constexpr float GeneticExpressionThreshold = 0.5f;
    static constexpr float MutationGrowthThreshold = 0.3f;
    static constexpr float LearningActivityMultiplier = 0.8f;
};

} // namespace Neural
} // namespace Engine
