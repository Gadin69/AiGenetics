using System;

/// <summary>
/// Represents a single neuron in the neural network
/// Can be dynamically added or removed based on genetic triggers
/// </summary>
public class Neuron
{
    /// <summary>
    /// Unique identifier for this neuron
    /// </summary>
    public string Id { get; set; }

    /// <summary>
    /// Current activation level (0.0 to 1.0)
    /// </summary>
    public double Activation { get; set; }

    /// <summary>
    /// Activation threshold
    /// </summary>
    public double Threshold { get; set; }

    /// <summary>
    /// Type of neuron
    /// </summary>
    public NeuronType Type { get; set; }

    /// <summary>
    /// Constructor for Neuron
    /// </summary>
    public Neuron()
    {
        Id = $"neuron_{Random.Shared.Next(1000000)}";
        Activation = Random.Shared.NextDouble();
        Threshold = Random.Shared.NextDouble() * 0.5 + 0.2;
        Type = NeuronType.General;
    }
}

/// <summary>
/// Types of neurons in the network
/// </summary>
public enum NeuronType
{
    /// <summary>
    /// General purpose neuron
    /// </summary>
    General,
    
    /// <summary>
    /// Specialized neuron activated by mutations
    /// </summary>
    Mutation,
    
    /// <summary>
    /// Specialized neuron involved in learning processes
    /// </summary>
    Learning,
    
    /// <summary>
    /// Specialized neuron for movement control
    /// </summary>
    Movement,
    
    /// <summary>
    /// Specialized neuron for visual processing
    /// </summary>
    Visual
}