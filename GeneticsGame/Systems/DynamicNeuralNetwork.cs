using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Base class supporting runtime neuron addition
/// Implements dynamic neural network that can grow new neurons based on genetic triggers
/// </summary>
public class DynamicNeuralNetwork
{
    /// <summary>
    /// List of neurons in this network
    /// </summary>
    public List<Neuron> Neurons { get; set; }

    /// <summary>
    /// List of connections between neurons
    /// </summary>
    public List<Connection> Connections { get; set; }

    /// <summary>
    /// Current activity level of the network
    /// </summary>
    public double ActivityLevel { get; set; }

    /// <summary>
    /// Constructor for DynamicNeuralNetwork
    /// </summary>
    public DynamicNeuralNetwork()
    {
        Neurons = new List<Neuron>();
        Connections = new List<Connection>();
        ActivityLevel = 0.0;
    }

    /// <summary>
    /// Add a new neuron to the network
    /// </summary>
    /// <param name="neuron">Neuron to add</param>
    public void AddNeuron(Neuron neuron)
    {
        Neurons.Add(neuron);
    }

    /// <summary>
    /// Add a connection between two neurons
    /// </summary>
    /// <param name="fromNeuron">Source neuron</param>
    /// <param name="toNeuron">Target neuron</param>
    /// <param name="weight">Connection weight</param>
    public void AddConnection(Neuron fromNeuron, Neuron toNeuron, double weight = 1.0)
    {
        var connection = new Connection(fromNeuron, toNeuron, weight);
        Connections.Add(connection);
    }

    /// <summary>
    /// Grow new neurons based on genetic triggers and activity
    /// </summary>
    /// <param name="genome">Genome providing growth signals</param>
    /// <param name="activityThreshold">Minimum activity level to trigger growth</param>
    /// <returns>Number of neurons added</returns>
    public int GrowNeurons(Genome genome, double activityThreshold = 0.5)
    {
        if (ActivityLevel < activityThreshold) return 0;
        
        int neuronsAdded = 0;
        int totalGrowthPotential = genome.GetTotalNeuronGrowthCount();
        
        // Limit growth to prevent runaway expansion
        int maxGrowth = Math.Min(totalGrowthPotential, GeneticsCore.Config.MaxNeuronGrowthPerGeneration);
        
        for (int i = 0; i < maxGrowth; i++)
        {
            // Create new neuron with random properties
            var newNeuron = new Neuron
            {
                Id = $"neuron_{Neurons.Count + i}_{Random.Shared.Next(1000)}",
                Activation = Random.Shared.NextDouble(),
                Threshold = Random.Shared.NextDouble() * 0.5 + 0.2, // 0.2-0.7 range
                Type = NeuronType.General
            };
            
            // Determine neuron type based on genetic context
            if (genome.CalculateEpistaticInteractions().Any(kvp => kvp.Key.Contains("neuron") && kvp.Value > 0.8))
            {
                newNeuron.Type = NeuronType.Mutation;
            }
            else if (genome.CalculateEpistaticInteractions().Any(kvp => kvp.Key.Contains("learning") && kvp.Value > 0.7))
            {
                newNeuron.Type = NeuronType.Learning;
            }
            
            AddNeuron(newNeuron);
            neuronsAdded++;
        }
        
        return neuronsAdded;
    }

    /// <summary>
    /// Update network activity based on current state
    /// </summary>
    public void UpdateActivity()
    {
        // Simple activity calculation: average activation of all neurons
        if (Neurons.Count > 0)
        {
            ActivityLevel = Neurons.Average(n => n.Activation);
        }
        else
        {
            ActivityLevel = 0.0;
        }
    }
}