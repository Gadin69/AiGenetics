using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Manages neuron creation based on genetic triggers
/// Implements hybrid triggering system (genetic expression + mutation + learning)
/// </summary>
public class NeuronGrowthController
{
    /// <summary>
    /// Current neural network being managed
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Current genome providing genetic triggers
    /// </summary>
    public Genome Genome { get; set; }

    /// <summary>
    /// Constructor for NeuronGrowthController
    /// </summary>
    /// <param name="neuralNetwork">Neural network to manage</param>
    /// <param name="genome">Genome providing genetic triggers</param>
    public NeuronGrowthController(DynamicNeuralNetwork neuralNetwork, Genome genome)
    {
        NeuralNetwork = neuralNetwork;
        Genome = genome;
    }

    /// <summary>
    /// Trigger neuron growth based on genetic expression
    /// </summary>
    /// <returns>Number of neurons added</returns>
    public int TriggerByGeneticExpression()
    {
        // Find genes with high neuron growth factor and expression level
        var growthGenes = new List<Gene<double>>();
        
        foreach (var chromosome in Genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.IsActive && gene.NeuronGrowthFactor > 0.5 && gene.ExpressionLevel > 0.3)
                {
                    growthGenes.Add(gene);
                }
            }
        }
        
        if (growthGenes.Count == 0) return 0;
        
        // Calculate total growth potential
        int growthCount = 0;
        foreach (var gene in growthGenes)
        {
            growthCount += gene.GetNeuronGrowthCount();
        }
        
        // Apply growth to neural network
        return NeuralNetwork.GrowNeurons(Genome, GeneticsCore.Config.NeuralActivityThreshold);
    }

    /// <summary>
    /// Trigger neuron growth based on mutation events
    /// </summary>
    /// <returns>Number of neurons added</returns>
    public int TriggerByMutation()
    {
        // Apply neural-specific mutations
        int mutationCount = MutationSystem.ApplyNeuralMutations(Genome);
        
        if (mutationCount > 0)
        {
            // Growth triggered by mutations
            return NeuralNetwork.GrowNeurons(Genome, 0.3); // Lower threshold for mutation-triggered growth
        }
        
        return 0;
    }

    /// <summary>
    /// Trigger neuron growth based on learning feedback
    /// </summary>
    /// <param name="learningRate">Current learning rate</param>
    /// <returns>Number of neurons added</returns>
    public int TriggerByLearning(double learningRate = 0.1)
    {
        // Growth triggered by learning activity
        if (NeuralNetwork.ActivityLevel > GeneticsCore.Config.NeuralActivityThreshold * 0.8)
        {
            // Higher activity leads to more growth
            var growthFactor = NeuralNetwork.ActivityLevel * 2.0;
            
            // Temporarily increase activity threshold for learning-triggered growth
            return NeuralNetwork.GrowNeurons(Genome, GeneticsCore.Config.NeuralActivityThreshold * 0.5);
        }
        
        return 0;
    }

    /// <summary>
    /// Execute hybrid growth trigger system
    /// </summary>
    /// <returns>Total number of neurons added</returns>
    public int ExecuteHybridGrowthTrigger()
    {
        int totalAdded = 0;
        
        // Genetic expression trigger (highest priority)
        totalAdded += TriggerByGeneticExpression();
        
        // Mutation trigger (medium priority)
        totalAdded += TriggerByMutation();
        
        // Learning trigger (lowest priority)
        totalAdded += TriggerByLearning();
        
        return totalAdded;
    }
}