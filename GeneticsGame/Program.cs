using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Main program for the 3D Genetics Game
/// Demonstrates the core genetic and neural systems
/// </summary>
class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine("3D Genetics Game - Core Systems Demo");
        Console.WriteLine("=====================================");
        
        // Create a random genome
        var breedingSystem = new BreedingSystem();
        var genome = breedingSystem.GenerateRandomGenome("test_genome_001", 5, 8);
        
        Console.WriteLine($"Created genome with {genome.Chromosomes.Count} chromosomes and {genome.Chromosomes.Sum(c => c.Genes.Count)} genes");
        
        // Calculate neuron growth potential
        int neuronGrowth = genome.GetTotalNeuronGrowthCount();
        Console.WriteLine($"Neuron growth potential: {neuronGrowth} neurons");
        
        // Create neural network
        var neuralNetwork = new DynamicNeuralNetwork();
        
        // Grow neurons based on genome
        int neuronsAdded = neuralNetwork.GrowNeurons(genome);
        Console.WriteLine($"Grew {neuronsAdded} neurons");
        
        // Create creature
        var chordataCreature = new ChordataCreature("chordata_001", genome);
        Console.WriteLine($"Created Chordata creature with {chordataCreature.NeuralNetwork.Neurons.Count} neurons");
        
        // Apply mutations
        int mutationsApplied = MutationSystem.ApplyMutations(genome);
        Console.WriteLine($"Applied {mutationsApplied} mutations");
        
        // Breed two creatures
        var genome2 = breedingSystem.GenerateRandomGenome("test_genome_002", 5, 8);
        var offspring = breedingSystem.Breed(genome, genome2);
        Console.WriteLine($"Bred offspring with {offspring.Chromosomes.Count} chromosomes");
        
        // Show epistatic interactions
        var interactions = genome.CalculateEpistaticInteractions();
        Console.WriteLine($"Calculated {interactions.Count} epistatic interactions");
        
        // Update creature
        chordataCreature.Update(1.0);
        Console.WriteLine($"After update: {chordataCreature.NeuralNetwork.Neurons.Count} neurons, Activity: {chordataCreature.NeuralNetwork.ActivityLevel:F2}");
        
        Console.WriteLine("\nDemo completed successfully!");
        Console.WriteLine("Press any key to exit...");
        Console.ReadKey();
    }
}