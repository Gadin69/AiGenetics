#pragma once

#include <cstdint>
#include <random>

namespace Engine {
namespace Genetics {

// Dominance types for gene expression
enum class DominanceType {
    Dominant,      // Gene always expresses
    Recessive,     // Gene only expresses if both alleles match
    CoDominant     // Both alleles contribute to expression
};

// Gene structure representing a single genetic locus
struct Gene {
    uint16_t locusID;           // Unique identifier for this gene locus
    float alleleValue;          // Expression value (0.0-1.0 normalized)
    DominanceType dominance;    // How this gene expresses
    
    Gene() : locusID(0), alleleValue(0.5f), dominance(DominanceType::Dominant) {}
    
    Gene(uint16_t id, float value, DominanceType dom = DominanceType::Dominant)
        : locusID(id), alleleValue(value), dominance(dom) {}
    
    // Express this gene's value based on dominance type
    float ExpressValue() const {
        return alleleValue;
    }
    
    // Combine with another gene for breeding (returns offspring gene)
    Gene CombineWith(const Gene& other) const {
        // Ensure same locus
        if (locusID != other.locusID) {
            throw std::runtime_error("Cannot combine genes with different loci");
        }
        
        Gene offspring;
        offspring.locusID = locusID;
        
        // Apply dominance rules
        switch (dominance) {
            case DominanceType::Dominant:
                // Dominant gene expresses, mix with recessive
                offspring.alleleValue = alleleValue * 0.7f + other.alleleValue * 0.3f;
                offspring.dominance = DominanceType::Dominant;
                break;
                
            case DominanceType::Recessive:
                // Recessive only expresses if both are recessive
                if (other.dominance == DominanceType::Recessive) {
                    offspring.alleleValue = (alleleValue + other.alleleValue) * 0.5f;
                    offspring.dominance = DominanceType::Recessive;
                } else {
                    offspring.alleleValue = other.alleleValue;
                    offspring.dominance = other.dominance;
                }
                break;
                
            case DominanceType::CoDominant:
                // Both contribute equally
                offspring.alleleValue = (alleleValue + other.alleleValue) * 0.5f;
                offspring.dominance = DominanceType::CoDominant;
                break;
        }
        
        // Clamp to valid range
        offspring.alleleValue = std::clamp(offspring.alleleValue, 0.0f, 1.0f);
        
        return offspring;
    }
    
    // Generate random gene for a specific locus
    static Gene Random(uint16_t locusID) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> valueDist(0.0f, 1.0f);
        static std::uniform_int_distribution<int> domDist(0, 2);
        
        return Gene(
            locusID,
            valueDist(gen),
            static_cast<DominanceType>(domDist(gen))
        );
    }
};

} // namespace Genetics
} // namespace Engine
