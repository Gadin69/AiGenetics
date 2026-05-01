#pragma once

#include "Chromosome.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace Engine {
namespace Genetics {

// Complete genome containing multiple chromosomes
class Genome {
private:
    std::vector<Chromosome> m_chromosomes;
    std::string m_genomeID;
    
public:
    Genome() : m_genomeID("") {}
    
    Genome(const std::string& id) : m_genomeID(id) {}
    
    // Add a chromosome to this genome
    void AddChromosome(const Chromosome& chromosome) {
        m_chromosomes.push_back(chromosome);
    }
    
    // Get chromosome by ID
    const Chromosome* GetChromosome(uint16_t chromosomeID) const {
        for (const auto& chrom : m_chromosomes) {
            if (chrom.GetID() == chromosomeID) {
                return &chrom;
            }
        }
        return nullptr;
    }
    
    // Get gene by locus ID (searches all chromosomes)
    const Gene* GetGene(uint16_t locusID) const {
        for (const auto& chrom : m_chromosomes) {
            const Gene* gene = chrom.GetGene(locusID);
            if (gene) {
                return gene;
            }
        }
        return nullptr;
    }
    
    // Get all chromosomes
    const std::vector<Chromosome>& GetChromosomes() const {
        return m_chromosomes;
    }
    
    // Get genome ID
    const std::string& GetID() const {
        return m_genomeID;
    }
    
    // Set genome ID
    void SetID(const std::string& id) {
        m_genomeID = id;
    }
    
    // Mutate entire genome
    void Mutate(float mutationRate, float mutationStrength = 0.1f) {
        for (auto& chromosome : m_chromosomes) {
            chromosome.Mutate(mutationRate, mutationStrength);
        }
    }
    
    // Crossover with another genome (returns offspring genome)
    Genome Crossover(const Genome& other) const {
        Genome offspring(m_genomeID + "_" + other.m_genomeID);
        
        // For each chromosome, perform crossover
        size_t maxChroms = (std::max)(m_chromosomes.size(), other.m_chromosomes.size());
        for (size_t i = 0; i < maxChroms; ++i) {
            if (i < m_chromosomes.size() && i < other.m_chromosomes.size()) {
                // Both parents have this chromosome
                Chromosome offspringChrom = m_chromosomes[i].Crossover(other.m_chromosomes[i]);
                offspring.AddChromosome(offspringChrom);
            } else if (i < m_chromosomes.size()) {
                // Only this parent has the chromosome
                offspring.AddChromosome(m_chromosomes[i]);
            } else {
                // Only other parent has the chromosome
                offspring.AddChromosome(other.m_chromosomes[i]);
            }
        }
        
        return offspring;
    }
    
    // Serialize genome to string
    std::string Serialize() const {
        std::stringstream ss;
        ss << "Genome[" << m_genomeID << "]:";
        
        for (const auto& chrom : m_chromosomes) {
            ss << "\n  Chromosome[" << chrom.GetID() << "]:";
            for (const auto& gene : chrom.GetGenes()) {
                ss << "\n    Locus[0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') 
                   << gene.locusID << std::dec << "]=" << gene.alleleValue 
                   << " (Dominance: " << static_cast<int>(gene.dominance) << ")";
            }
        }
        
        return ss.str();
    }
    
    // Get chromosome count
    size_t GetChromosomeCount() const {
        return m_chromosomes.size();
    }
    
    // Get total gene count
    size_t GetTotalGeneCount() const {
        size_t count = 0;
        for (const auto& chrom : m_chromosomes) {
            count += chrom.GetGeneCount();
        }
        return count;
    }
};

} // namespace Genetics
} // namespace Engine
