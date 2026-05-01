using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Specialized genome for Arthropoda creatures
/// Includes arthropod-specific genetic features and regulatory elements
/// </summary>
public class ArthropodaGenome : Genome
{
    /// <summary>
    /// Constructor for ArthropodaGenome
    /// </summary>
    /// <param name="id">Unique identifier</param>
    public ArthropodaGenome(string id) : base(id)
    {
        // Initialize with arthropoda-specific chromosomes
        InitializeArthropodaChromosomes();
    }

    /// <summary>
    /// Initialize arthropoda-specific chromosomes
    /// </summary>
    private void InitializeArthropodaChromosomes()
    {
        // Chromosome 1: Exoskeleton development genes
        var chr1 = new Chromosome("chr1_exoskeleton");
        AddChromosome(chr1);
        
        // Add exoskeleton development genes
        chr1.AddGene(new Gene<double>("exoskeleton_thickness", 0.6, 0.002, 0.0));
        chr1.AddGene(new Gene<double>("exoskeleton_hardness", 0.7, 0.002, 0.0));
        chr1.AddGene(new Gene<double>("molting_cycle", 0.5, 0.003, 0.0));
        
        // Chromosome 2: Segmentation genes
        var chr2 = new Chromosome("chr2_segmentation");
        AddChromosome(chr2);
        
        // Add segmentation genes
        chr2.AddGene(new Gene<double>("segment_count", 0.8, 0.001, 0.0));
        chr2.AddGene(new Gene<double>("segment_size_variation", 0.6, 0.002, 0.0));
        chr2.AddGene(new Gene<double>("joint_complexity", 0.7, 0.002, 0.0));
        
        // Chromosome 3: Limb development genes
        var chr3 = new Chromosome("chr3_limb");
        AddChromosome(chr3);
        
        // Add limb development genes
        chr3.AddGene(new Gene<double>("limb_count", 0.9, 0.001, 0.0));
        chr3.AddGene(new Gene<double>("limb_joint_count", 0.7, 0.002, 0.0));
        chr3.AddGene(new Gene<double>("sensory_appendage_count", 0.6, 0.002, 0.0));
        
        // Chromosome 4: Neural development genes
        var chr4 = new Chromosome("chr4_neural");
        AddChromosome(chr4);
        
        // Add neural development genes
        chr4.AddGene(new Gene<double>("ganglion_count", 0.5, 0.003, 0.6)); // High neuron growth factor
        chr4.AddGene(new Gene<double>("nerve_cord_length", 0.6, 0.002, 0.4));
        chr4.AddGene(new Gene<double>("sensory_neuron_density", 0.7, 0.002, 0.5));
        
        // Chromosome 5: Metabolic genes
        var chr5 = new Chromosome("chr5_metabolism");
        AddChromosome(chr5);
        
        // Add metabolic genes
        chr5.AddGene(new Gene<double>("metabolic_rate", 0.8, 0.002, 0.0));
        chr5.AddGene(new Gene<double>("oxygen_efficiency", 0.5, 0.001, 0.0));
        chr5.AddGene(new Gene<double>("temperature_tolerance", 0.6, 0.002, 0.0));
    }

    /// <summary>
    /// Get specialized arthropoda traits
    /// </summary>
    /// <returns>Dictionary of arthropoda-specific traits</returns>
    public Dictionary<string, double> GetArthropodaTraits()
    {
        var traits = new Dictionary<string, double>();
        
        // Extract arthropoda-specific traits from chromosomes
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("exoskeleton") || gene.Id.Contains("segment") || 
                    gene.Id.Contains("limb") || gene.Id.Contains("neural") ||
                    gene.Id.Contains("metabolism"))
                {
                    traits[gene.Id] = gene.ExpressionLevel;
                }
            }
        }
        
        return traits;
    }

    /// <summary>
    /// Apply arthropoda-specific mutation rules
    /// </summary>
    /// <returns>Number of mutations applied</returns>
    public int ApplyArthropodaSpecificMutations()
    {
        int mutationsApplied = 0;
        
        // Apply special mutation rates for arthropoda-specific genes
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                double mutationRate = gene.MutationRate;
                
                // Increase mutation rate for neural genes
                if (gene.Id.Contains("neural") || gene.Id.Contains("ganglion") || gene.Id.Contains("nerve"))
                {
                    mutationRate *= 2.0;
                }
                
                // Increase mutation rate for exoskeleton genes
                if (gene.Id.Contains("exoskeleton") || gene.Id.Contains("shell") || gene.Id.Contains("molting"))
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