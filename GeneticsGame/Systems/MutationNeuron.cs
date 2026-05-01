using System;

/// <summary>
/// Specialized neuron type activated by specific mutations
/// Gains purpose through activity and neural network integration
/// </summary>
public class MutationNeuron : Neuron
{
    /// <summary>
    /// Type of mutation this neuron responds to
    /// </summary>
    public MutationType MutationType { get; set; }

    /// <summary>
    /// Genes that activated this mutation neuron
    /// </summary>
    public List<string> ActivatingGenes { get; set; }

    /// <summary>
    /// Constructor for MutationNeuron
    /// </summary>
    /// <param name="mutationType">Type of mutation</param>
    public MutationNeuron(MutationType mutationType)
    {
        MutationType = mutationType;
        ActivatingGenes = new List<string>();
        Type = NeuronType.Mutation;
        
        // Set specific properties for mutation neurons
        Threshold = Random.Shared.NextDouble() * 0.3 + 0.1; // Lower threshold for sensitivity
        Activation = Random.Shared.NextDouble() * 0.2; // Start with low activation
    }

    /// <summary>
    /// Activate this mutation neuron based on genetic triggers
    /// </summary>
    /// <param name="geneId">ID of activating gene</param>
    /// <param name="activationStrength">Strength of activation</param>
    public void Activate(string geneId, double activationStrength = 1.0)
    {
        if (!ActivatingGenes.Contains(geneId))
        {
            ActivatingGenes.Add(geneId);
        }
        
        // Increase activation based on strength
        Activation = Math.Min(1.0, Activation + activationStrength * 0.3);
    }
}

/// <summary>
/// Types of mutations that can activate mutation neurons
/// </summary>
public enum MutationType
{
    /// <summary>
    /// Structural mutation (deletion, duplication, etc.)
    /// </summary>
    Structural,
    
    /// <summary>
    /// Regulatory mutation affecting gene expression
    /// </summary>
    Regulatory,
    
    /// <summary>
    /// Epigenetic modification
    /// </summary>
    Epigenetic,
    
    /// <summary>
    /// Neural-specific mutation
    /// </summary>
    Neural
}