using System;
using System.Collections.Generic;

/// <summary>
/// Generic gene class with expression levels, mutation rates, and neural growth parameters
/// Represents a single genetic unit that can influence multiple traits
/// </summary>
/// <typeparam name="T">The data type this gene represents (e.g., double for expression level)</typeparam>
public class Gene<T>
{
    /// <summary>
    /// Unique identifier for this gene
    /// </summary>
    public string Id { get; set; }

    /// <summary>
    /// Current expression level of this gene (0.0 to 1.0)
    /// </summary>
    public double ExpressionLevel { get; set; }

    /// <summary>
    /// Base mutation rate for this gene
    /// </summary>
    public double MutationRate { get; set; }

    /// <summary>
    /// Neural growth factor - controls how much this gene influences neuron addition
    /// Higher values trigger more neuron growth when expressed
    /// </summary>
    public double NeuronGrowthFactor { get; set; }

    /// <summary>
    /// Whether this gene is currently active/expressed
    /// </summary>
    public bool IsActive { get; set; }

    /// <summary>
    /// List of genes this gene interacts with (epistatic relationships)
    /// </summary>
    public List<string> InteractionPartners { get; set; }

    /// <summary>
    /// Constructor for Gene
    /// </summary>
    /// <param name="id">Unique identifier</param>
    /// <param name="expressionLevel">Initial expression level</param>
    /// <param name="mutationRate">Base mutation rate</param>
    /// <param name="neuronGrowthFactor">Neural growth influence factor</param>
    public Gene(string id, double expressionLevel = 0.5, double mutationRate = 0.001, double neuronGrowthFactor = 0.0)
    {
        Id = id;
        ExpressionLevel = expressionLevel;
        MutationRate = mutationRate;
        NeuronGrowthFactor = neuronGrowthFactor;
        IsActive = expressionLevel > 0.1;
        InteractionPartners = new List<string>();
    }

    /// <summary>
    /// Apply mutation to this gene
    /// </summary>
    /// <returns>True if mutation occurred</returns>
    public bool Mutate()
    {
        if (Random.Shared.NextDouble() < MutationRate)
        {
            // Randomly adjust expression level
            ExpressionLevel = Math.Max(0.0, Math.Min(1.0, ExpressionLevel + (Random.Shared.NextDouble() - 0.5) * 0.2));
            
            // Randomly adjust neuron growth factor
            NeuronGrowthFactor = Math.Max(0.0, NeuronGrowthFactor + (Random.Shared.NextDouble() - 0.5) * 0.1);
            
            // Randomly activate/deactivate
            IsActive = Random.Shared.NextDouble() < 0.8;
            
            return true;
        }
        return false;
    }

    /// <summary>
    /// Get the number of neurons this gene would trigger based on expression level
    /// </summary>
    /// <returns>Number of neurons to grow</returns>
    public int GetNeuronGrowthCount()
    {
        if (!IsActive) return 0;
        
        // Calculate neuron growth: higher expression + higher growth factor = more neurons
        double baseGrowth = ExpressionLevel * NeuronGrowthFactor * 10.0;
        return Math.Max(0, Math.Min(10, (int)Math.Round(baseGrowth)));
    }
}