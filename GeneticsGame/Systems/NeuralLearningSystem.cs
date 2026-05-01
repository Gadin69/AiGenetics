using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Neural learning system for the 3D Genetics Game
/// Implements learning-based neuron growth and adaptation
/// </summary>
public class NeuralLearningSystem
{
    /// <summary>
    /// Current neural network being learned
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Current genome providing genetic context
    /// </summary>
    public Genome Genome { get; set; }

    /// <summary>
    /// Constructor for NeuralLearningSystem
    /// </summary>
    /// <param name="neuralNetwork">Neural network to learn</param>
    /// <param name="genome">Genome providing genetic context</param>
    public NeuralLearningSystem(DynamicNeuralNetwork neuralNetwork, Genome genome)
    {
        NeuralNetwork = neuralNetwork;
        Genome = genome;
    }

    /// <summary>
    /// Perform one learning cycle
    /// </summary>
    /// <param name="learningRate">Learning rate for this cycle</param>
    /// <returns>Number of neurons added during learning</returns>
    public int Learn(double learningRate = 0.1)
    {
        // Update neural network activity
        NeuralNetwork.UpdateActivity();
        
        // Build new synapses based on activity
        var synapseBuilder = new ActivityBasedSynapseBuilder(NeuralNetwork);
        int synapsesBuilt = synapseBuilder.BuildSynapses(0.3, 5);
        
        // Strengthen existing synapses
        int synapsesStrengthened = synapseBuilder.StrengthenSynapses(learningRate * 0.5);
        
        // Prune weak synapses
        int synapsesPruned = synapseBuilder.PruneSynapses(0.05);
        
        // Trigger neuron growth based on learning feedback
        var growthController = new NeuronGrowthController(NeuralNetwork, Genome);
        int neuronsAdded = growthController.TriggerByLearning(learningRate);
        
        return neuronsAdded;
    }

    /// <summary>
    /// Adapt neural network to new environment or task
    /// </summary>
    /// <param name="environmentFactors">Environmental factors affecting learning</param>
    /// <param name="taskRequirements">Task requirements for adaptation</param>
    /// <returns>Adaptation success score</returns>
    public double AdaptToEnvironment(Dictionary<string, double> environmentFactors, Dictionary<string, double> taskRequirements)
    {
        double adaptationScore = 0.0;
        
        // Calculate environmental adaptation based on neural activity
        foreach (var factor in environmentFactors)
        {
            if (factor.Key.Contains("light") || factor.Key.Contains("visual"))
            {
                // Visual environment adaptation
                adaptationScore += NeuralNetwork.Neurons.Count(n => n.Type == NeuronType.Visual) * factor.Value * 0.2;
            }
            else if (factor.Key.Contains("sound") || factor.Key.Contains("auditory"))
            {
                // Auditory environment adaptation
                adaptationScore += NeuralNetwork.Neurons.Count(n => n.Type == NeuronType.General) * factor.Value * 0.1;
            }
        }
        
        // Calculate task adaptation based on neural control level
        foreach (var requirement in taskRequirements)
        {
            if (requirement.Key.Contains("precision") || requirement.Key.Contains("fine"))
            {
                // Precision task adaptation
                adaptationScore += NeuralNetwork.Neurons.Count(n => n.Type == NeuronType.Learning) * requirement.Value * 0.3;
            }
            else if (requirement.Key.Contains("strength") || requirement.Key.Contains("power"))
            {
                // Strength task adaptation
                adaptationScore += NeuralNetwork.Neurons.Count(n => n.Type == NeuronType.Movement) * requirement.Value * 0.2;
            }
        }
        
        // Apply genetic constraints
        adaptationScore *= Genome.GetTotalNeuronGrowthCount() * 0.1 + 0.5;
        
        return Math.Max(0.0, Math.Min(1.0, adaptationScore));
    }

    /// <summary>
    /// Simulate learning over multiple cycles
    /// </summary>
    /// <param name="cycles">Number of learning cycles</param>
    /// <param name="learningRate">Learning rate per cycle</param>
    /// <returns>Total neurons added</returns>
    public int LearnOverTime(int cycles, double learningRate = 0.1)
    {
        int totalNeuronsAdded = 0;
        
        for (int i = 0; i < cycles; i++)
        {
            totalNeuronsAdded += Learn(learningRate * (1.0 - (double)i / cycles)); // Decreasing learning rate
        }
        
        return totalNeuronsAdded;
    }
}