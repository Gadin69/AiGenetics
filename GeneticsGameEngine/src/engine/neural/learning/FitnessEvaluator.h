#pragma once

#include <vector>
#include <string>

namespace Engine {
namespace Neural {

// Evaluates creature performance and generates fitness scores
// Used to provide learning rewards to the reinforcement learning system
class FitnessEvaluator {
public:
    FitnessEvaluator();
    
    // Evaluate fitness based on multiple metrics
    float EvaluateFitness(
        float survivalTime,
        float movementEfficiency,
        float goalAchievement,
        float energyConsumption
    ) const;
    
    // Convert fitness score to learning reward (-1.0 to 1.0)
    float FitnessToReward(float fitnessScore) const;
    
    // Track fitness over time
    void RecordFitness(float fitnessScore);
    
    // Get average fitness over recorded history
    float GetAverageFitness() const;
    
    // Get fitness trend (positive = improving, negative = declining)
    float GetFitnessTrend() const;
    
    // Reset fitness history
    void Reset();
    
    // Configuration
    struct FitnessWeights {
        float survivalTimeWeight;
        float movementEfficiencyWeight;
        float goalAchievementWeight;
        float energyEfficiencyWeight;
        
        FitnessWeights()
            : survivalTimeWeight(0.3f)
            , movementEfficiencyWeight(0.25f)
            , goalAchievementWeight(0.35f)
            , energyEfficiencyWeight(0.1f)
        {}
    };
    
    void SetWeights(const FitnessWeights& weights);

private:
    FitnessWeights m_weights;
    std::vector<float> m_fitnessHistory;
    
    static constexpr size_t MaxFitnessHistory = 1000;
};

} // namespace Neural
} // namespace Engine
