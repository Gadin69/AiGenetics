#pragma once

#include "Organism.h"

namespace Engine {
namespace Genetics {
namespace Taxonomy {

// Animal class - inherits from Organism
class Animal : public Organism {
protected:
    int m_limbCount;
    float m_movementSpeed;
    
public:
    Animal() : m_limbCount(4), m_movementSpeed(1.0f) {}
    virtual ~Animal() = default;
    
    // Getters
    int GetLimbCount() const { return m_limbCount; }
    float GetMovementSpeed() const { return m_movementSpeed; }
    
    // Setters
    void SetLimbCount(int limbs) { m_limbCount = limbs; }
    void SetMovementSpeed(float speed) { m_movementSpeed = speed; }
    
    // Implement breeding logic
    std::vector<Genome> BreedWith(const Organism& other) const override {
        std::vector<Genome> offspring;
        
        // Check if same species can breed
        if (m_speciesName != other.GetSpeciesName()) {
            return offspring; // Cannot breed different species
        }
        
        // Perform crossover
        Genome childGenome = m_genome.Crossover(other.GetGenome());
        offspring.push_back(childGenome);
        
        return offspring;
    }
    
    // Apply mutation
    void ApplyMutation(float mutationRate) override {
        m_genome.Mutate(mutationRate);
        // Re-apply genetic expression after mutation
        ApplyGeneticExpression(m_genome);
    }
};

} // namespace Taxonomy
} // namespace Genetics
} // namespace Engine
