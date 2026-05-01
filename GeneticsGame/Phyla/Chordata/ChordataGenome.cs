using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Specialized genome for Chordata creatures
/// Includes chordata-specific genetic features and regulatory elements
/// </summary>
public class ChordataGenome : Genome
{
    /// <summary>
    /// Constructor for ChordataGenome
    /// </summary>
    /// <param name="id">Unique identifier</param>
    public ChordataGenome(string id) : base(id)
    {
        // Initialize with chordata-specific chromosomes
        InitializeChordataChromosomes();
    }

    /// <summary>
    /// Initialize chordata-specific chromosomes
    /// </summary>
    private void InitializeChordataChromosomes()
    {
        // Chromosome 1: Spinal development genes
        var chr1 = new Chromosome("chr1_spine");
        AddChromosome(chr1);
        
        // Add spinal development genes
        chr1.AddGene(new Gene<double>("spine_length", 0.7, 0.001, 0.0));
        chr1.AddGene(new Gene<double>("spine_flexibility", 0.5, 0.002, 0.0));
        chr1.AddGene(new Gene<double>("vertebra_count", 0.6, 0.001, 0.0));
        
        // Chromosome 2: Neural development genes
        var chr2 = new Chromosome("chr2_neural");
        AddChromosome(chr2);
        
        // Add neural development genes
        chr2.AddGene(new Gene<double>("neuron_count", 0.4, 0.003, 0.8)); // High neuron growth factor
        chr2.AddGene(new Gene<double>("synapse_density", 0.6, 0.002, 0.5));
        chr2.AddGene(new Gene<double>("brain_size", 0.5, 0.002, 0.7)); // High neuron growth factor
        
        // Chromosome 3: Limb development genes
        var chr3 = new Chromosome("chr3_limb");
        AddChromosome(chr3);
        
        // Add limb development genes
        chr3.AddGene(new Gene<double>("limb_count", 0.8, 0.001, 0.0));
        chr3.AddGene(new Gene<double>("limb_length", 0.6, 0.002, 0.0));
        chr3.AddGene(new Gene<double>("joint_complexity", 0.5, 0.002, 0.0));
        
        // Chromosome 4: Sensory system genes
        var chr4 = new Chromosome("chr4_sensory");
        AddChromosome(chr4);
        
        // Add sensory genes
        chr4.AddGene(new Gene<double>("vision_acuity", 0.7, 0.002, 0.3));
        chr4.AddGene(new Gene<double>("hearing_range", 0.6, 0.002, 0.2));
        chr4.AddGene(new Gene<double>("balance_system", 0.8, 0.001, 0.4)); // High neuron growth factor for balance
        
        // Chromosome 5: Metabolic genes
        var chr5 = new Chromosome("chr5_metabolism");
        AddChromosome(chr5);
        
        // Add metabolic genes
        chr5.AddGene(new Gene<double>("metabolic_rate", 0.5, 0.002, 0.0));
        chr5.AddGene(new Gene<double>("oxygen_efficiency", 0.6, 0.001, 0.0));
        chr5.AddGene(new Gene<double>("temperature_regulation", 0.7, 0.002, 0.0));
    }

    /// <summary>
    /// Get specialized chordata traits
    /// </summary>
    /// <returns>Dictionary of chordata-specific traits</returns>
    public Dictionary<string, double> GetChordataTraits()
    {
        var traits = new Dictionary<string, double>();
        
        // Extract chordata-specific traits from chromosomes
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("spine") || gene.Id.Contains("neural") || 
                    gene.Id.Contains("limb") || gene.Id.Contains("sensory") ||
                    gene.Id.Contains("metabolism"))
                {
                    traits[gene.Id] = gene.ExpressionLevel;
                }
            }
        }
        
        return traits;
    }

    /// <summary>
    /// Apply chordata-specific mutation rules
    /// </summary>
    /// <returns>Number of mutations applied</returns>
    public int ApplyChordataSpecificMutations()
    {
        int mutationsApplied = 0;
        
        // Apply special mutation rates for chordata-specific genes
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                double mutationRate = gene.MutationRate;
                
                // Increase mutation rate for neural genes
                if (gene.Id.Contains("neural") || gene.Id.Contains("brain") || gene.Id.Contains("nn"))
                {
                    mutationRate *= 2.0;
                }
                
                // Increase mutation rate for spine genes
                if (gene.Id.Contains("spine") || gene.Id.Contains("vertebra"))
                {
                    mutationRate *= 1.5;
                }
                
                if (Random.Shared.NextDouble() < mutationRate)
                {
                    gene.Mutate();
                    mutationsApplied++;
                }
            }
        }
        
        return mutationsApplied;
    }
}