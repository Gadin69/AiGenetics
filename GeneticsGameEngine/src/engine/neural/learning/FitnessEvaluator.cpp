#include "FitnessEvaluator.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace Engine {
namespace Neural {

FitnessEvaluator::FitnessEvaluator()
{
    m_fitnessHistory.reserve(MaxFitnessHistory);
}

float FitnessEvaluator::EvaluateFitness(
    float survivalTime,
    float movementEfficiency,
    float goalAchievement,
    float energyConsumption
) const {
    // Normalize metrics to 0-1 range
    float normalizedSurvival = std::clamp(survivalTime / 100.0f, 0.0f, 1.0f);  // Assume 100s is max
    float normalizedMovement = std::clamp(movementEfficiency, 0.0f, 1.0f);
    float normalizedGoal = std::clamp(goalAchievement, 0.0f, 1.0f);
    
    // Energy consumption: lower is better, invert the metric
    float normalizedEnergy = std::clamp(1.0f - (energyConsumption / 100.0f), 0.0f, 1.0f);
    
    // Weighted sum
    float fitness = 
        normalizedSurvival * m_weights.survivalTimeWeight +
        normalizedMovement * m_weights.movementEfficiencyWeight +
        normalizedGoal * m_weights.goalAchievementWeight +
        normalizedEnergy * m_weights.energyEfficiencyWeight;
    
    return std::clamp(fitness, 0.0f, 1.0f);
}

float FitnessEvaluator::FitnessToReward(float fitnessScore) const
{
    // Convert fitness (0-1) to reward (-1 to 1)
    // Above average fitness = positive reward, below average = negative
    float averageFitness = GetAverageFitness();
    
    if (m_fitnessHistory.empty()) {
        // No history yet, use neutral reward
        return (fitnessScore - 0.5f) * 2.0f;
    }
    
    // Compare to average
    float relativeFitness = fitnessScore - averageFitness;
    
    // Scale to -1 to 1 range
    float reward = std::clamp(relativeFitness * 2.0f, -1.0f, 1.0f);
    
    return reward;
}

void FitnessEvaluator::RecordFitness(float fitnessScore)
{
    m_fitnessHistory.push_back(std::clamp(fitnessScore, 0.0f, 1.0f));
    
    // Limit history size
    if (m_fitnessHistory.size() > MaxFitnessHistory) {
        m_fitnessHistory.erase(m_fitnessHistory.begin());
    }
}

float FitnessEvaluator::GetAverageFitness() const
{
    if (m_fitnessHistory.empty()) {
        return 0.5f;  // Neutral default
    }
    
    float sum = std::accumulate(m_fitnessHistory.begin(), m_fitnessHistory.end(), 0.0f);
    return sum / static_cast<float>(m_fitnessHistory.size());
}

float FitnessEvaluator::GetFitnessTrend() const
{
    if (m_fitnessHistory.size() < 2) {
        return 0.0f;  // No trend yet
    }
    
    // Calculate trend using linear regression (simplified)
    size_t n = m_fitnessHistory.size();
    size_t halfN = n / 2;
    
    // Compare first half average to second half average
    float firstHalfSum = 0.0f;
    float secondHalfSum = 0.0f;
    
    for (size_t i = 0; i < halfN; ++i) {
        firstHalfSum += m_fitnessHistory[i];
    }
    for (size_t i = halfN; i < n; ++i) {
        secondHalfSum += m_fitnessHistory[i];
    }
    
    float firstHalfAvg = firstHalfSum / static_cast<float>(halfN);
    float secondHalfAvg = secondHalfSum / static_cast<float>(n - halfN);
    
    // Positive trend means improving
    return secondHalfAvg - firstHalfAvg;
}

void FitnessEvaluator::Reset()
{
    m_fitnessHistory.clear();
}

void FitnessEvaluator::SetWeights(const FitnessWeights& weights)
{
    m_weights = weights;
    
    // Normalize weights to sum to 1.0
    float total = m_weights.survivalTimeWeight + 
                  m_weights.movementEfficiencyWeight + 
                  m_weights.goalAchievementWeight + 
                  m_weights.energyEfficiencyWeight;
    
    if (total > 0.0f) {
        m_weights.survivalTimeWeight /= total;
        m_weights.movementEfficiencyWeight /= total;
        m_weights.goalAchievementWeight /= total;
        m_weights.energyEfficiencyWeight /= total;
    }
}

} // namespace Neural
} // namespace Engine
