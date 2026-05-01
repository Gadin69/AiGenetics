using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Breeding system for the 3D Genetics Game
/// Implements ARK-style breeding with mutation simulation
/// </summary>
public class BreedingSystem
{
    /// <summary>
    /// Breed two parent genomes to create offspring
    /// </summary>
    /// <param name="parent1">First parent genome</param>
    /// <param name="parent2">Second parent genome</param>
    /// <param name="mutationRate">Base mutation rate for offspring</param>
    /// <returns>New offspring genome</returns>
    public Genome Breed(Genome parent1, Genome parent2, double mutationRate = 0.001)
    {
        // Create offspring genome using inheritance rules
        var offspring = Genome.Breed(parent1, parent2);
        
        // Apply mutations to offspring
        int mutationsApplied = MutationSystem.ApplyMutations(offspring, mutationRate);
        
        return offspring;
    }

    /// <summary>
    /// Calculate compatibility between two genomes for breeding
    /// </summary>
    /// <param name="genome1">First genome</param>
    /// <param name="genome2">Second genome</param>
    /// <returns>Compatibility score (0.0-1.0)</returns>
    public double CalculateCompatibility(Genome genome1, Genome genome2)
    {
        // Compatibility based on genetic similarity and diversity
        double similarityScore = CalculateGeneticSimilarity(genome1, genome2);
        double diversityScore = CalculateGeneticDiversity(genome1, genome2);
        
        // Optimal compatibility is moderate similarity with good diversity
        double compatibility = (similarityScore * 0.4) + (diversityScore * 0.6);
        
        return Math.Max(0.0, Math.Min(1.0, compatibility));
    }

    /// <summary>
    /// Calculate genetic similarity between two genomes
    /// </summary>
    /// <param name="genome1">First genome</param>
    /// <param name="genome2">Second genome</param>
    /// <returns>Similarity score (0.0-1.0)</returns>
    private double CalculateGeneticSimilarity(Genome genome1, Genome genome2)
    {
        if (genome1.Chromosomes.Count == 0 || genome2.Chromosomes.Count == 0)
            return 0.5;
        
        int totalGenes = 0;
        int matchingGenes = 0;
        
        // Compare genes across chromosomes
        foreach (var chromosome1 in genome1.Chromosomes)
        {
            foreach (var gene1 in chromosome1.Genes)
            {
                totalGenes++;
                
                // Look for matching gene ID in genome2
                bool foundMatch = false;
                foreach (var chromosome2 in genome2.Chromosomes)
                {
                    foreach (var gene2 in chromosome2.Genes)
                    {
                        if (gene1.Id == gene2.Id)
                        {
                            foundMatch = true;
                            break;
                        }
                    }
                    if (foundMatch) break;
                }
                
                if (foundMatch) matchingGenes++;
            }
        }
        
        return totalGenes > 0 ? (double)matchingGenes / totalGenes : 0.5;
    }

    /// <summary>
    /// Calculate genetic diversity between two genomes
    /// </summary>
    /// <param name="genome1">First genome</param>
    /// <param name="genome2">Second genome</param>
    /// <returns>Diversity score (0.0-1.0)</returns>
    private double CalculateGeneticDiversity(Genome genome1, Genome genome2)
    {
        // Diversity based on expression level differences
        double totalDifference = 0.0;
        int geneCount = 0;
        
        foreach (var chromosome1 in genome1.Chromosomes)
        {
            foreach (var gene1 in chromosome1.Genes)
            {
                geneCount++;
                
                // Find matching gene in genome2
                double difference = 1.0;
                foreach (var chromosome2 in genome2.Chromosomes)
                {
                    foreach (var gene2 in chromosome2.Genes)
                    {
                        if (gene1.Id == gene2.Id)
                        {
                            difference = Math.Abs(gene1.ExpressionLevel - gene2.ExpressionLevel);
                            break;
                        }
                    }
                    if (difference < 1.0) break;
                }
                
                totalDifference += difference;
            }
        }
        
        return geneCount > 0 ? 1.0 - (totalDifference / geneCount) : 0.5;
    }

    /// <summary>
    /// Generate a random genome for initial creatures
    /// </summary>
    /// <param name="genomeId">ID for the new genome</param>
    /// <param name="chromosomeCount">Number of chromosomes</param>
    /// <param name="genesPerChromosome">Number of genes per chromosome</param>
    /// <returns>New random genome</returns>
    public Genome GenerateRandomGenome(string genomeId, int chromosomeCount = 23, int genesPerChromosome = 10)
    {
        var genome = new Genome(genomeId);
        
        for (int i = 0; i < chromosomeCount; i++)
        {
            var chromosome = new Chromosome($"chr_{i + 1}");
            
            for (int j = 0; j < genesPerChromosome; j++)
            {
                // Create different types of genes
                string geneType = j % 4 switch
                {
                    0 => "color",
                    1 => "structure",
                    2 => "neural",
                    _ => "regulatory"
                };
                
                string geneId = $"{geneType}_{i}_{j}_{Random.Shared.Next(1000)}";
                
                // Set neuron growth factor for neural genes
                double neuronGrowthFactor = geneType == "neural" ? Random.Shared.NextDouble() * 0.8 + 0.2 : 0.0;
                
                var gene = new Gene<double>(
                    geneId,
                    Random.Shared.NextDouble(),
                    0.001 + Random.Shared.NextDouble() * 0.002,
                    neuronGrowthFactor
                );
                
                // Add interaction partners for epistatic networks
                if (j > 0)
                {
                    gene.InteractionPartners.Add($"{geneType}_{i}_{j-1}_{Random.Shared.Next(1000)}");
                }
                
                chromosome.AddGene(gene);
            }
            
            genome.AddChromosome(chromosome);
        }
        
        return genome;
    }
}