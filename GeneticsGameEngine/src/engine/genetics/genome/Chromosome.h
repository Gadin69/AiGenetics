#pragma once

#include "Gene.h"
#include <vector>
#include <algorithm>
#include <random>

namespace Engine {
namespace Genetics {

// Chromosome containing multiple genes
class Chromosome {
private:
    std::vector<Gene> m_genes;
    uint16_t m_chromosomeID;
    
public:
    Chromosome() : m_chromosomeID(0) {}
    
    Chromosome(uint16_t id) : m_chromosomeID(id) {}
    
    // Add a gene to this chromosome
    void AddGene(const Gene& gene) {
        m_genes.push_back(gene);
    }
    
    // Get gene by locus ID
    const Gene* GetGene(uint16_t locusID) const {
        for (const auto& gene : m_genes) {
            if (gene.locusID == locusID) {
                return &gene;
            }
        }
        return nullptr;
    }
    
    // Get all genes
    const std::vector<Gene>& GetGenes() const {
        return m_genes;
    }
    
    // Get chromosome ID
    uint16_t GetID() const {
        return m_chromosomeID;
    }
    
    // Get gene count
    size_t GetGeneCount() const {
        return m_genes.size();
    }
    
    // Crossover with another chromosome (returns offspring chromosome)
    Chromosome Crossover(const Chromosome& other) const {
        if (m_chromosomeID != other.m_chromosomeID) {
            throw std::runtime_error("Cannot crossover chromosomes with different IDs");
        }
        
        Chromosome offspring(m_chromosomeID);
        
        // Static random device for crossover
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // For each gene position, randomly select from either parent
        size_t maxGenes = (std::max)(m_genes.size(), other.m_genes.size());
        for (size_t i = 0; i < maxGenes; ++i) {
            if (i < m_genes.size() && i < other.m_genes.size()) {
                // Both parents have this gene - combine them
                Gene offspringGene = m_genes[i].CombineWith(other.m_genes[i]);
                offspring.AddGene(offspringGene);
            } else if (i < m_genes.size()) {
                // Only this parent has the gene
                offspring.AddGene(m_genes[i]);
            } else {
                // Only other parent has the gene
                offspring.AddGene(other.m_genes[i]);
            }
        }
        
        return offspring;
    }
    
    // Mutate genes in this chromosome
    void Mutate(float mutationRate, float mutationStrength) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        static std::normal_distribution<float> normalDist(0.0f, mutationStrength);
        
        for (auto& gene : m_genes) {
            if (dist(gen) < mutationRate) {
                // Apply mutation
                float mutation = normalDist(gen);
                gene.alleleValue += mutation;
                gene.alleleValue = (std::clamp)(gene.alleleValue, 0.0f, 1.0f);
            }
        }
    }
};

} // namespace Genetics
} // namespace Engine
