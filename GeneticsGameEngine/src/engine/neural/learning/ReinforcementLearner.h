#pragma once

#include "../core/NeuralNetwork.h"
#include <vector>
#include <memory>

namespace Engine {
namespace Neural {

// Reinforcement learning system with reward-based weight adjustment
// Combines Hebbian learning with reward modulation
class ReinforcementLearner {
public:
    ReinforcementLearner(NeuralNetwork& network);
    
    // Apply learning step with reward signal
    void Learn(float reward, float learningRate = 0.1f);
    
    // Hebbian learning: strengthen connections between co-active neurons
    void ApplyHebbianLearning(float learningRate = 0.05f);
    
    // Reward-modulated learning: adjust weights based on success/failure
    void ApplyRewardModulation(float reward, float learningRate = 0.1f);
    
    // Decay learning rate over time for stability
    void DecayLearningRate(float decayFactor = 0.99f);
    
    // Get current learning rate
    float GetLearningRate() const { return m_currentLearningRate; }
    
    // Reset learning state
    void Reset();

private:
    NeuralNetwork& m_neuralNetwork;
    float m_currentLearningRate;
    int m_learningSteps;
    
    // Track recent rewards for moving average
    std::vector<float> m_recentRewards;
    static constexpr size_t MaxRewardHistory = 100;
};

} // namespace Neural
} // namespace Engine
