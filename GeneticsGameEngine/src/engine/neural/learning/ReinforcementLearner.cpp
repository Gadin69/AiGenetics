#include "ReinforcementLearner.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace Engine {
namespace Neural {

ReinforcementLearner::ReinforcementLearner(NeuralNetwork& network)
    : m_neuralNetwork(network)
    , m_currentLearningRate(0.1f)
    , m_learningSteps(0)
{
    m_recentRewards.reserve(MaxRewardHistory);
}

void ReinforcementLearner::Learn(float reward, float learningRate)
{
    m_currentLearningRate = learningRate;
    m_learningSteps++;
    
    // Store reward in history
    m_recentRewards.push_back(reward);
    if (m_recentRewards.size() > MaxRewardHistory) {
        m_recentRewards.erase(m_recentRewards.begin());
    }
    
    // Apply Hebbian learning
    ApplyHebbianLearning(learningRate);
    
    // Apply reward modulation
    ApplyRewardModulation(reward, learningRate);
}

void ReinforcementLearner::ApplyHebbianLearning(float learningRate)
{
    // Hebbian learning: "neurons that fire together wire together"
    // Strengthen connections between co-active neurons
    auto& synapses = const_cast<std::vector<Synapse>&>(m_neuralNetwork.GetSynapses());
    const auto& neurons = m_neuralNetwork.GetNeurons();
    
    for (auto& synapse : synapses) {
        if (synapse.fromNeuronIdx < neurons.size() && synapse.toNeuronIdx < neurons.size()) {
            const auto& fromNeuron = neurons[synapse.fromNeuronIdx];
            const auto& toNeuron = neurons[synapse.toNeuronIdx];
            
            // Hebbian update: delta_w = learning_rate * pre_activation * post_activation
            float hebbianUpdate = learningRate * fromNeuron.activation * toNeuron.activation;
            
            // Apply update with weight clipping
            synapse.weight += hebbianUpdate;
            synapse.weight = std::clamp(synapse.weight, -1.0f, 1.0f);
            
            // Track activity for pruning
            synapse.lastActivity = std::abs(hebbianUpdate);
        }
    }
}

void ReinforcementLearner::ApplyRewardModulation(float reward, float learningRate)
{
    // Reward-modulated learning: adjust weights based on success/failure
    // Positive reward strengthens active connections, negative reward weakens them
    auto& synapses = const_cast<std::vector<Synapse>&>(m_neuralNetwork.GetSynapses());
    const auto& neurons = m_neuralNetwork.GetNeurons();
    
    float rewardSignal = std::clamp(reward, -1.0f, 1.0f);
    
    for (auto& synapse : synapses) {
        if (synapse.fromNeuronIdx < neurons.size() && synapse.toNeuronIdx < neurons.size()) {
            const auto& fromNeuron = neurons[synapse.fromNeuronIdx];
            const auto& toNeuron = neurons[synapse.toNeuronIdx];
            
            // Only modify active connections
            if (fromNeuron.activation > 0.1f && toNeuron.activation > 0.1f) {
                // Reward-modulated update
                float rewardUpdate = learningRate * rewardSignal * fromNeuron.activation;
                synapse.weight += rewardUpdate;
                
                // Clip weights to valid range
                synapse.weight = std::clamp(synapse.weight, -1.0f, 1.0f);
            }
        }
    }
}

void ReinforcementLearner::DecayLearningRate(float decayFactor)
{
    m_currentLearningRate *= decayFactor;
    m_currentLearningRate = std::max(0.001f, m_currentLearningRate);  // Minimum learning rate
}

void ReinforcementLearner::Reset()
{
    m_currentLearningRate = 0.1f;
    m_learningSteps = 0;
    m_recentRewards.clear();
}

} // namespace Neural
} // namespace Engine
