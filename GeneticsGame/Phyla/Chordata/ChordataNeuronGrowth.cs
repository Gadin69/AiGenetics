using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Chordata-specific neuron growth implementation
/// Implements neural growth patterns specific to vertebrate-like creatures
/// </summary>
public class ChordataNeuronGrowth
{
    /// <summary>
    /// Neural network for chordata creature
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Chordata genome providing genetic context
    /// </summary>
    public ChordataGenome Genome { get; set; }

    /// <summary>
    /// Constructor for ChordataNeuronGrowth
    /// </summary>
    /// <param name="neuralNetwork">Neural network</param>
    /// <param name="genome">Chordata genome</param>
    public ChordataNeuronGrowth(DynamicNeuralNetwork neuralNetwork, ChordataGenome genome)
    {
        NeuralNetwork = neuralNetwork;
        Genome = genome;
    }

    /// <summary>
    /// Grow neurons in chordata-specific patterns
    /// </summary>
    /// <returns>Number of neurons added</returns>
    public int GrowChordataNeurons()
    {
        int neuronsAdded = 0;
        
        // Get chordata-specific traits
        var traits = Genome.GetChordataTraits();
        
        // Calculate growth potential based on chordata traits
        double neuralGrowthPotential = 0.0;
        
        if (traits.ContainsKey("neuron_count"))
            neuralGrowthPotential += traits["neuron_count"] * 0.4;
        
        if (traits.ContainsKey("brain_size"))
            neuralGrowthPotential += traits["brain_size"] * 0.5;
        
        if (traits.ContainsKey("synapse_density"))
            neuralGrowthPotential += traits["synapse_density"] * 0.3;
        
        // Apply chordata-specific growth rules
        int baseGrowth = (int)(neuralGrowthPotential * 10);
        
        // Spinal cord growth pattern
        if (traits.ContainsKey("spine_length") && traits["spine_length"] > 0.6)
        {
            // Longer spines support more spinal cord neurons
            baseGrowth += (int)(traits["spine_length"] * 5);
        }
        
        // Brain size growth pattern
        if (traits.ContainsKey("brain_size") && traits["brain_size"] > 0.7)
        {
            // Larger brains support more complex neural networks
            baseGrowth += (int)(traits["brain_size"] * 8);
        }
        
        // Apply growth with chordata-specific constraints
        for (int i = 0; i < baseGrowth; i++)
        {
            // Create chordata-specific neuron
            var neuron = new Neuron();
            
            // Set chordata-specific properties
            neuron.Type = NeuronType.General;
            
            // Add specialized neurons based on traits
            if (traits.ContainsKey("vision_acuity") && traits["vision_acuity"] > 0.8)
            {
                neuron.Type = NeuronType.Visual;
            }
            else if (traits.ContainsKey("hearing_range") && traits["hearing_range"] > 0.7)
            {
                neuron.Type = NeuronType.General;
            }
            else if (traits.ContainsKey("balance_system") && traits["balance_system"] > 0.8)
            {
                neuron.Type = NeuronType.Movement;
            }
            
            // Set activation threshold based on chordata traits
            neuron.Threshold = 0.2 + (Random.Shared.NextDouble() * 0.3);
            
            NeuralNetwork.AddNeuron(neuron);
            neuronsAdded++;
        }
        
        return neuronsAdded;
    }

    /// <summary>
    /// Apply chordata-specific neural plasticity rules
    /// </summary>
    /// <returns>Number of connections modified</returns>
    public int ApplyChordataPlasticity()
    {
        int modifications = 0;
        
        // Get chordata traits
        var traits = Genome.GetChordataTraits();
        
        // Apply plasticity based on sensory traits
        if (traits.ContainsKey("vision_acuity") && traits["vision_acuity"] > 0.7)
        {
            // Visual system plasticity
            modifications += ApplyVisualPlasticity();
        }
        
        if (traits.ContainsKey("balance_system") && traits["balance_system"] > 0.6)
        {
            // Balance system plasticity
            modifications += ApplyBalancePlasticity();
        }
        
        if (traits.ContainsKey("brain_size") && traits["brain_size"] > 0.5)
        {
            // General neural plasticity
            modifications += ApplyGeneralPlasticity();
        }
        
        return modifications;
    }

    /// <summary>
    /// Apply visual system plasticity
    /// </summary>
    /// <returns>Number of connections modified</returns>
    private int ApplyVisualPlasticity()
    {
        int modifications = 0;
        
        // Find visual neurons and strengthen connections
        var visualNeurons = NeuralNetwork.Neurons.Where(n => n.Type == NeuronType.Visual).ToList();
        
        foreach (var neuron in visualNeurons)
        {
            // Strengthen connections to other visual neurons
            foreach (var connection in NeuralNetwork.Connections)
            {
                if (connection.FromNeuron == neuron && 
                    connection.ToNeuron.Type == NeuronType.Visual)
                {
                    connection.Weight = Math.Min(1.0, connection.Weight + 0.1);
                    modifications++;
                }
            }
        }
        
        return modifications;
    }

    /// <summary>
    /// Apply balance system plasticity
    /// </summary>
    /// <returns>Number of connections modified</returns>
    private int ApplyBalancePlasticity()
    {
        int modifications = 0;
        
        // Find movement neurons and strengthen connections
        var movementNeurons = NeuralNetwork.Neurons.Where(n => n.Type == NeuronType.Movement).ToList();
        
        foreach (var neuron in movementNeurons)
        {
            // Strengthen connections to balance-related neurons
            foreach (var connection in NeuralNetwork.Connections)
            {
                if (connection.FromNeuron == neuron && 
                    (connection.ToNeuron.Type == NeuronType.Movement || 
                     connection.ToNeuron.Type == NeuronType.General))
                {
                    connection.Weight = Math.Min(1.0, connection.Weight + 0.05);
                    modifications++;
                }
            }
        }
        
        return modifications;
    }

    /// <summary>
    /// Apply general neural plasticity
    /// </summary>
    /// <returns>Number of connections modified</returns>
    private int ApplyGeneralPlasticity()
    {
        int modifications = 0;
        
        // Randomly strengthen some connections
        for (int i = 0; i < NeuralNetwork.Connections.Count / 10; i++)
        {
            if (NeuralNetwork.Connections.Count > 0)
            {
                var randomConnection = NeuralNetwork.Connections[Random.Shared.Next(NeuralNetwork.Connections.Count)];
                randomConnection.Weight = Math.Min(1.0, randomConnection.Weight + 0.02);
                modifications++;
            }
        }
        
        return modifications;
    }
}