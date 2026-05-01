#pragma once

#include "../genome/Genome.h"
#include "../taxonomy/Organism.h"
#include <vector>
#include <string>

namespace Engine {
namespace Genetics {
namespace Breeding {

// Breeding system for creating offspring from parent organisms
class BreedingSystem {
public:
    // Breed two organisms to produce offspring
    static std::vector<Genome> Breed(const Taxonomy::Organism& parent1, 
                                     const Taxonomy::Organism& parent2, 
                                     int offspringCount = 1) {
        std::vector<Genome> offspring;
        
        // Use parent's breeding method
        offspring = parent1.BreedWith(parent2);
        
        // Generate additional offspring if requested
        for (int i = 1; i < offspringCount; ++i) {
            // Perform another crossover
            Genome childGenome = parent1.GetGenome().Crossover(parent2.GetGenome());
            offspring.push_back(childGenome);
        }
        
        return offspring;
    }
    
    // Create a genome with specific name
    static Genome CreateOffspringGenome(const Taxonomy::Organism& parent1,
                                       const Taxonomy::Organism& parent2,
                                       const std::string& offspringID) {
        Genome offspring = parent1.GetGenome().Crossover(parent2.GetGenome());
        offspring.SetID(offspringID);
        return offspring;
    }
};

} // namespace Breeding
} // namespace Genetics
} // namespace Engine
