using System;

/// <summary>
/// Represents a connection between two neurons in the neural network
/// </summary>
public class Connection
{
    /// <summary>
    /// Source neuron
    /// </summary>
    public Neuron FromNeuron { get; set; }

    /// <summary>
    /// Target neuron
    /// </summary>
    public Neuron ToNeuron { get; set; }

    /// <summary>
    /// Connection weight
    /// </summary>
    public double Weight { get; set; }

    /// <summary>
    /// Constructor for Connection
    /// </summary>
    /// <param name="fromNeuron">Source neuron</param>
    /// <param name="toNeuron">Target neuron</param>
    /// <param name="weight">Connection weight</param>
    public Connection(Neuron fromNeuron, Neuron toNeuron, double weight = 1.0)
    {
        FromNeuron = fromNeuron;
        ToNeuron = toNeuron;
        Weight = weight;
    }
}