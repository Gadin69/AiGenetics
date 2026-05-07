#include "GrowthController.h"
#include <algorithm>
#include <iostream>

namespace Engine {
namespace Neural {

GrowthController::GrowthController(NeuralNetwork& network, const Engine::Genetics::Genome* genome)
    : m_neuralNetwork(network)
    , m_genome(genome)
    , m_mutationOccurred(false)
    , m_learningRate(0.1f)
{
}

int GrowthController::TriggerByGeneticExpression()
{
    if (!m_genome) {
        std::cerr << "[GrowthController] WARNING: m_genome is null!" << std::endl;
        return 0;
    }
    
    // Find genes with high neuron growth factor and expression level
    int growthGeneCount = 0;
    
    for (const auto& chromosome : m_genome->GetChromosomes()) {
        for (const auto& gene : chromosome.GetGenes()) {
            // Check if gene is active and has neural growth influence
            // Gene is considered active if it's dominant or co-dominant
            bool isActive = (gene.dominance == Engine::Genetics::DominanceType::Dominant || 
                           gene.dominance == Engine::Genetics::DominanceType::CoDominant);
            
            if (isActive && gene.alleleValue > 0.3f) {
                uint16_t locusID = gene.locusID;
                
                // Neural growth-related loci (example range)
                if (locusID >= 0xA000 && locusID <= 0xAFFF) {
                    growthGeneCount++;
                }
            }
        }
    }
    
    if (growthGeneCount == 0) {
        return 0;
    }
    
    // Apply growth to neural network
    int neuronsAdded = m_neuralNetwork.GrowNeurons(*m_genome, GeneticExpressionThreshold);
    
    return neuronsAdded;
}

int GrowthController::TriggerByMutation()
{
    if (!m_mutationOccurred) {
        return 0;
    }
    
    // Reset mutation flag
    m_mutationOccurred = false;
    
    // Growth triggered by mutations - use lower threshold
    int neuronsAdded = m_neuralNetwork.GrowNeurons(*m_genome, MutationGrowthThreshold);
    
    return neuronsAdded;
}

int GrowthController::TriggerByLearning(float learningRate)
{
    m_learningRate = learningRate;
    
    // Growth triggered by learning activity
    float activityThreshold = NeuralConfig::DefaultActivationThreshold * LearningActivityMultiplier;
    
    if (m_neuralNetwork.GetActivityLevel() > activityThreshold) {
        // Higher activity leads to more growth
        // Temporarily use lower threshold for learning-triggered growth
        int neuronsAdded = m_neuralNetwork.GrowNeurons(*m_genome, activityThreshold * 0.5f);
        return neuronsAdded;
    }
    
    return 0;
}

int GrowthController::ExecuteHybridGrowthTrigger()
{
    int totalAdded = 0;
    
    // Genetic expression trigger (highest priority)
    totalAdded += TriggerByGeneticExpression();
    
    // Mutation trigger (medium priority)
    totalAdded += TriggerByMutation();
    
    // Learning trigger (lowest priority)
    totalAdded += TriggerByLearning(m_learningRate);
    
    return totalAdded;
}

void GrowthController::NotifyMutationOccurred()
{
    m_mutationOccurred = true;
}

void GrowthController::SetLearningRate(float rate)
{
    m_learningRate = std::max(0.0f, std::min(1.0f, rate));
}

} // namespace Neural
} // namespace Engine
