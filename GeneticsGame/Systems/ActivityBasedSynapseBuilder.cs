using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Creates connections based on neural activity patterns
/// Implements Hebbian learning: "neurons that fire together, wire together"
/// </summary>
public class ActivityBasedSynapseBuilder
{
    /// <summary>
    /// Neural network to build synapses for
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Constructor for ActivityBasedSynapseBuilder
    /// </summary>
    /// <param name="neuralNetwork">Neural network to build synapses for</param>
    public ActivityBasedSynapseBuilder(DynamicNeuralNetwork neuralNetwork)
    {
        NeuralNetwork = neuralNetwork;
    }

    /// <summary>
    /// Build new synapses based on current neural activity
    /// </summary>
    /// <param name="activityThreshold">Minimum activity level to form connections</param>
    /// <param name="maxConnections">Maximum number of new connections to create</param>
    /// <returns>Number of connections created</returns>
    public int BuildSynapses(double activityThreshold = 0.5, int maxConnections = 10)
    {
        if (NeuralNetwork.Neurons.Count < 2) return 0;
        
        int connectionsCreated = 0;
        
        // Find active neurons
        var activeNeurons = NeuralNetwork.Neurons
            .Where(n => n.Activation > activityThreshold)
            .ToList();
        
        if (activeNeurons.Count < 2) return 0;
        
        // Create connections between active neurons
        for (int i = 0; i < activeNeurons.Count && connectionsCreated < maxConnections; i++)
        {
            for (int j = 0; j < activeNeurons.Count && connectionsCreated < maxConnections; j++)
            {
                if (i != j)
                {
                    // Calculate connection weight based on activity correlation
                    double weight = Math.Min(1.0, (activeNeurons[i].Activation + activeNeurons[j].Activation) / 2.0);
                    
                    // Only create connection if it doesn't already exist
                    bool connectionExists = NeuralNetwork.Connections.Any(c => 
                        c.FromNeuron == activeNeurons[i] && c.ToNeuron == activeNeurons[j]);
                    
                    if (!connectionExists)
                    {
                        NeuralNetwork.AddConnection(activeNeurons[i], activeNeurons[j], weight);
                        connectionsCreated++;
                    }
                }
            }
        }
        
        return connectionsCreated;
    }

    /// <summary>
    /// Strengthen existing synapses based on activity patterns
    /// </summary>
    /// <param name="strengthFactor">How much to strengthen connections</param>
    /// <returns>Number of connections strengthened</returns>
    public int StrengthenSynapses(double strengthFactor = 0.1)
    {
        int strengthened = 0;
        
        foreach (var connection in NeuralNetwork.Connections)
        {
            // Strengthen connection based on activity of both neurons
            double activityProduct = connection.FromNeuron.Activation * connection.ToNeuron.Activation;
            connection.Weight = Math.Min(1.0, connection.Weight + activityProduct * strengthFactor);
            strengthened++;
        }
        
        return strengthened;
    }

    /// <summary>
    /// Prune weak synapses based on activity thresholds
    /// </summary>
    /// <param name="pruneThreshold">Minimum weight to keep a connection</param>
    /// <returns>Number of connections pruned</returns>
    public int PruneSynapses(double pruneThreshold = 0.1)
    {
        int pruned = 0;
        
        // Remove connections with weight below threshold
        var connectionsToRemove = NeuralNetwork.Connections
            .Where(c => c.Weight < pruneThreshold)
            .ToList();
        
        foreach (var connection in connectionsToRemove)
        {
            NeuralNetwork.Connections.Remove(connection);
            pruned++;
        }
        
        return pruned;
    }
}