using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Complete genetic blueprint with multi-gene interaction rules
/// Represents the entire genetic makeup of a creature
/// </summary>
public class Genome
{
    /// <summary>
    /// Unique identifier for this genome
    /// </summary>
    public string Id { get; set; }

    /// <summary>
    /// List of chromosomes in this genome
    /// </summary>
    public List<Chromosome> Chromosomes { get; set; }

    /// <summary>
    /// Constructor for Genome
    /// </summary>
    /// <param name="id">Unique identifier</param>
    public Genome(string id)
    {
        Id = id;
        Chromosomes = new List<Chromosome>();
    }

    /// <summary>
    /// Add a chromosome to this genome
    /// </summary>
    /// <param name="chromosome">Chromosome to add</param>
    public void AddChromosome(Chromosome chromosome)
    {
        Chromosomes.Add(chromosome);
    }

    /// <summary>
    /// Apply mutations to the entire genome
    /// </summary>
    /// <returns>Number of mutations applied</returns>
    public int ApplyMutations()
    {
        int totalMutations = 0;
        
        // Apply point mutations to individual genes
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Mutate())
                    totalMutations++;
            }
        }
        
        // Apply structural mutations to chromosomes
        foreach (var chromosome in Chromosomes)
        {
            if (chromosome.ApplyStructuralMutation())
                totalMutations++;
        }
        
        return totalMutations;
    }

    /// <summary>
    /// Get total neuron growth potential from the entire genome
    /// </summary>
    /// <returns>Total neuron growth count</returns>
    public int GetTotalNeuronGrowthCount()
    {
        return Chromosomes.Sum(chromosome => chromosome.GetTotalNeuronGrowthCount());
    }

    /// <summary>
    /// Calculate epistatic interactions between genes
    /// </summary>
    /// <returns>Dictionary of gene interactions</returns>
    public Dictionary<string, double> CalculateEpistaticInteractions()
    {
        var interactions = new Dictionary<string, double>();
        
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                // Calculate interaction strength based on expression levels and partners
                double interactionStrength = gene.ExpressionLevel * 0.5;
                
                foreach (var partnerId in gene.InteractionPartners)
                {
                    // Find partner gene and calculate combined effect
                    var partnerGene = FindGeneById(partnerId);
                    if (partnerGene != null)
                    {
                        interactionStrength += partnerGene.ExpressionLevel * 0.3;
                    }
                }
                
                interactions[gene.Id] = Math.Min(1.0, interactionStrength);
            }
        }
        
        return interactions;
    }

    /// <summary>
    /// Find a gene by its ID across all chromosomes
    /// </summary>
    /// <param name="geneId">ID of the gene to find</param>
    /// <returns>The gene if found, null otherwise</returns>
    private Gene<double> FindGeneById(string geneId)
    {
        foreach (var chromosome in Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id == geneId)
                    return gene;
            }
        }
        return null;
    }

    /// <summary>
    /// Create a new genome by breeding two parent genomes
    /// Implements Mendelian inheritance with multi-gene interactions
    /// </summary>
    /// <param name="parent1">First parent genome</param>
    /// <param name="parent2">Second parent genome</param>
    /// <returns>New offspring genome</returns>
    public static Genome Breed(Genome parent1, Genome parent2)
    {
        var offspring = new Genome($"{parent1.Id}_x_{parent2.Id}_{Random.Shared.Next(1000)}");
        
        // For each chromosome, randomly select from either parent
        int chromosomeCount = Math.Max(parent1.Chromosomes.Count, parent2.Chromosomes.Count);
        
        for (int i = 0; i < chromosomeCount; i++)
        {
            Chromosome parentChromosome;
            
            if (i < parent1.Chromosomes.Count && i < parent2.Chromosomes.Count)
            {
                // Both parents have this chromosome, choose randomly
                parentChromosome = Random.Shared.NextDouble() < 0.5 ? 
                    parent1.Chromosomes[i] : parent2.Chromosomes[i];
            }
            else if (i < parent1.Chromosomes.Count)
            {
                // Only parent1 has this chromosome
                parentChromosome = parent1.Chromosomes[i];
            }
            else if (i < parent2.Chromosomes.Count)
            {
                // Only parent2 has this chromosome
                parentChromosome = parent2.Chromosomes[i];
            }
            else
            {
                continue;
            }
            
            // Create new chromosome with genes from parent
            var newChromosome = new Chromosome(parentChromosome.Id);
            
            foreach (var gene in parentChromosome.Genes)
            {
                // Create new gene instance with inherited properties
                var newGene = new Gene<double>(
                    gene.Id,
                    (gene.ExpressionLevel + Random.Shared.NextDouble() * 0.1) / 2.0, // Average with small variation
                    gene.MutationRate,
                    gene.NeuronGrowthFactor
                );
                
                // Inherit interaction partners
                newGene.InteractionPartners = new List<string>(gene.InteractionPartners);
                
                newChromosome.AddGene(newGene);
            }
            
            offspring.AddChromosome(newChromosome);
        }
        
        return offspring;
    }
}