using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Core mutation engine for the 3D Genetics Game
/// Handles all types of genetic mutations with performance optimization
/// </summary>
public static class MutationSystem
{
    /// <summary>
    /// Apply mutations to a genome based on mutation rates and types
    /// </summary>
    /// <param name="genome">Genome to mutate</param>
    /// <param name="mutationRate">Base mutation rate (0.0 to 1.0)</param>
    /// <returns>Number of mutations applied</returns>
    public static int ApplyMutations(Genome genome, double mutationRate = 0.001)
    {
        // Apply point mutations to individual genes
        int pointMutations = ApplyPointMutations(genome, mutationRate);
        
        // Apply structural mutations to chromosomes
        int structuralMutations = ApplyStructuralMutations(genome, mutationRate * 0.1); // Structural mutations are rarer
        
        // Apply epigenetic modifications
        int epigeneticMutations = ApplyEpigeneticModifications(genome, mutationRate * 0.05);
        
        return pointMutations + structuralMutations + epigeneticMutations;
    }

    /// <summary>
    /// Apply point mutations to individual genes
    /// </summary>
    /// <param name="genome">Genome to mutate</param>
    /// <param name="mutationRate">Mutation rate for point mutations</param>
    /// <returns>Number of point mutations applied</returns>
    private static int ApplyPointMutations(Genome genome, double mutationRate)
    {
        int count = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (Random.Shared.NextDouble() < mutationRate * gene.MutationRate)
                {
                    gene.Mutate();
                    count++;
                }
            }
        }
        
        return count;
    }

    /// <summary>
    /// Apply structural mutations to chromosomes
    /// </summary>
    /// <param name="genome">Genome to mutate</param>
    /// <param name="mutationRate">Mutation rate for structural mutations</param>
    /// <returns>Number of structural mutations applied</returns>
    private static int ApplyStructuralMutations(Genome genome, double mutationRate)
    {
        int count = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            if (Random.Shared.NextDouble() < mutationRate)
            {
                if (chromosome.ApplyStructuralMutation())
                    count++;
            }
        }
        
        return count;
    }

    /// <summary>
    /// Apply epigenetic modifications that affect gene expression without changing DNA sequence
    /// </summary>
    /// <param name="genome">Genome to modify</param>
    /// <param name="mutationRate">Mutation rate for epigenetic modifications</param>
    /// <returns>Number of epigenetic modifications applied</returns>
    private static int ApplyEpigeneticModifications(Genome genome, double mutationRate)
    {
        int count = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (Random.Shared.NextDouble() < mutationRate)
                {
                    // Epigenetic modification: change expression level without altering other properties
                    gene.ExpressionLevel = Math.Max(0.0, Math.Min(1.0, gene.ExpressionLevel + (Random.Shared.NextDouble() - 0.5) * 0.3));
                    gene.IsActive = gene.ExpressionLevel > 0.1;
                    count++;
                }
            }
        }
        
        return count;
    }

    /// <summary>
    /// Apply neural-specific mutations that affect neuron growth patterns
    /// </summary>
    /// <param name="genome">Genome to mutate</param>
    /// <param name="mutationRate">Mutation rate for neural mutations</param>
    /// <returns>Number of neural mutations applied</returns>
    public static int ApplyNeuralMutations(Genome genome, double mutationRate = 0.01)
    {
        int count = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (Random.Shared.NextDouble() < mutationRate && gene.NeuronGrowthFactor > 0.0)
                {
                    // Neural mutation: specifically target neuron growth parameters
                    gene.NeuronGrowthFactor = Math.Max(0.0, gene.NeuronGrowthFactor + (Random.Shared.NextDouble() - 0.5) * 0.2);
                    
                    // May also affect expression level for neural genes
                    if (gene.Id.Contains("neuron") || gene.Id.Contains("brain") || gene.Id.Contains("nn"))
                    {
                        gene.ExpressionLevel = Math.Max(0.0, Math.Min(1.0, gene.ExpressionLevel + (Random.Shared.NextDouble() - 0.5) * 0.2));
                    }
                    
                    count++;
                }
            }
        }
        
        return count;
    }
}