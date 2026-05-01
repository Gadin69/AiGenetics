#pragma once

#include "../genome/Genome.h"
#include <random>

namespace Engine {
namespace Genetics {
namespace Breeding {

// Mutation engine for applying genetic mutations
class MutationEngine {
public:
    // Mutate an entire genome
    static void MutateGenome(Genome& genome, float mutationRate, float mutationStrength = 0.1f) {
        int mutatedGenes = 0;
        int totalGenes = 0;
        
        // Count and mutate genes
        for (auto& chromosome : genome.GetChromosomes()) {
            for (auto& gene : const_cast<std::vector<Gene>&>(chromosome.GetGenes())) {
                totalGenes++;
                if (ShouldMutate(mutationRate)) {
                    ApplyPointMutation(gene, mutationStrength);
                    mutatedGenes++;
                }
            }
        }
    }
    
    // Apply point mutation to a single gene
    static void ApplyPointMutation(Gene& gene, float strength) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::normal_distribution<float> normalDist(0.0f, strength);
        
        // Apply Gaussian mutation
        float mutation = normalDist(gen);
        gene.alleleValue += mutation;
        
        // Clamp to valid range
        gene.alleleValue = std::clamp(gene.alleleValue, 0.0f, 1.0f);
    }
    
    // Apply chromosomal mutation (rare event)
    static void ApplyChromosomalMutation(Chromosome& chromosome) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<int> mutationTypeDist(0, 2);
        
        int mutationType = mutationTypeDist(gen);
        
        switch (mutationType) {
            case 0: // Gene duplication
                if (!chromosome.GetGenes().empty()) {
                    static std::uniform_int_distribution<size_t> geneDist(0, chromosome.GetGeneCount() - 1);
                    size_t index = geneDist(gen);
                    chromosome.AddGene(chromosome.GetGenes()[index]);
                }
                break;
                
            case 1: // Gene deletion
                if (chromosome.GetGeneCount() > 1) {
                    static std::uniform_int_distribution<size_t> geneDist(0, chromosome.GetGeneCount() - 1);
                    size_t index = geneDist(gen);
                    // Note: In a real implementation, you'd remove the gene
                    // For now, we'll just mutate it heavily
                    Gene* genes = const_cast<Gene*>(chromosome.GetGenes().data());
                    genes[index].alleleValue = 0.0f; // "Silent" gene
                }
                break;
                
            case 2: // Gene inversion (reverse order)
                // Complex operation - skip for now
                break;
        }
    }
    
    // Generate a random genome for testing
    static Genome GenerateRandomGenome(const std::string& genomeID, 
                                      const std::vector<uint16_t>& locusIDs) {
        Genome genome(genomeID);
        Chromosome chromosome(1); // Single chromosome for simplicity
        
        for (uint16_t locusID : locusIDs) {
            chromosome.AddGene(Gene::Random(locusID));
        }
        
        genome.AddChromosome(chromosome);
        return genome;
    }
    
private:
    // Determine if mutation should occur based on rate
    static bool ShouldMutate(float mutationRate) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        return dist(gen) < mutationRate;
    }
};

} // namespace Breeding
} // namespace Genetics
} // namespace Engine
