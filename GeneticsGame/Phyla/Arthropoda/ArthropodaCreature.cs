using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Insect/crustacean-like creature implementation
/// Represents creatures with exoskeletons and segmented bodies
/// </summary>
public class ArthropodaCreature
{
    /// <summary>
    /// Unique identifier for this creature
    /// </summary>
    public string Id { get; set; }

    /// <summary>
    /// Genetic blueprint for this creature
    /// </summary>
    public Genome Genome { get; set; }

    /// <summary>
    /// Neural network for this creature
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Current movement parameters
    /// </summary>
    public MovementParameters MovementParameters { get; set; }

    /// <summary>
    /// Current mesh parameters
    /// </summary>
    public MeshParameters MeshParameters { get; set; }

    /// <summary>
    /// Constructor for ArthropodaCreature
    /// </summary>
    /// <param name="id">Unique identifier</param>
    /// <param name="genome">Genetic blueprint</param>
    public ArthropodaCreature(string id, Genome genome)
    {
        Id = id;
        Genome = genome;
        
        // Initialize neural network
        NeuralNetwork = new DynamicNeuralNetwork();
        
        // Initialize procedural systems
        var synchronizedGenerator = new SynchronizedGenerator();
        var parameters = synchronizedGenerator.GenerateSynchronizedParameters(genome);
        
        MovementParameters = parameters.MovementParameters;
        MeshParameters = parameters.MeshParameters;
    }

    /// <summary>
    /// Update creature state for one time step
    /// </summary>
    /// <param name="timeStep">Time step duration</param>
    public void Update(double timeStep)
    {
        // Update neural network activity
        NeuralNetwork.UpdateActivity();
        
        // Apply learning if needed
        if (Random.Shared.NextDouble() < 0.05) // 5% chance per update (lower than chordata)
        {
            var learningSystem = new NeuralLearningSystem(NeuralNetwork, Genome);
            learningSystem.Learn(0.02);
        }
        
        // Update movement parameters based on neural activity
        UpdateMovementParameters();
        
        // Update mesh parameters based on genetic expression
        UpdateMeshParameters();
    }

    /// <summary>
    /// Update movement parameters based on current neural activity
    /// </summary>
    private void UpdateMovementParameters()
    {
        // Adjust speed based on neural activity
        double activityFactor = NeuralNetwork.ActivityLevel * 0.3 + 0.7;
        MovementParameters.BaseSpeed = Math.Max(0.1, Math.Min(3.0, 
            MovementParameters.BaseSpeed * activityFactor));
        
        // Adjust gait complexity based on limb count
        int limbCount = Genome.CalculateEpistaticInteractions().Count(kvp => kvp.Key.Contains("limb"));
        MovementParameters.GaitComplexity = Math.Max(1, Math.Min(10, 
            limbCount + 1));
    }

    /// <summary>
    /// Update mesh parameters based on current genetic expression
    /// </summary>
    private void UpdateMeshParameters()
    {
        // Update scale based on overall gene expression
        double totalExpression = 0.0;
        int geneCount = 0;
        
        foreach (var chromosome in Genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                totalExpression += gene.ExpressionLevel;
                geneCount++;
            }
        }
        
        double averageExpression = geneCount > 0 ? totalExpression / geneCount : 0.5;
        MeshParameters.BaseScale = Math.Max(0.1, Math.Min(1.0, 
            0.5 + (averageExpression - 0.5) * 1.0));
        
        // Update vertex count based on exoskeleton complexity
        int exoskeletonGenes = Genome.CalculateEpistaticInteractions().Count(kvp => kvp.Key.Contains("exoskeleton") || kvp.Key.Contains("shell"));
        MeshParameters.VertexCount = Math.Max(500, 
            MeshParameters.VertexCount + exoskeletonGenes * 100);
    }

    /// <summary>
    /// Get visualization parameters for this creature
    /// </summary>
    /// <returns>Visualization parameters</returns>
    public VisualizationParameters GetVisualizationParameters()
    {
        var visualizationSystem = new VisualizationSystem(Genome, NeuralNetwork);
        return visualizationSystem.GenerateVisualizationParameters();
    }
}