using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Converts genetic data to locomotion patterns
/// Implements procedural movement generation based on genetic traits
/// </summary>
public class MovementProceduralGenerator
{
    /// <summary>
    /// Generate movement parameters from a genome
    /// </summary>
    /// <param name="genome">Genome to convert</param>
    /// <returns>Movement parameters object</returns>
    public MovementParameters GenerateMovementParameters(Genome genome)
    {
        var parameters = new MovementParameters();
        
        // Calculate base movement properties from genome
        parameters.BaseSpeed = CalculateBaseSpeed(genome);
        parameters.MovementType = CalculateMovementType(genome);
        parameters.GaitComplexity = CalculateGaitComplexity(genome);
        
        // Calculate locomotion features
        parameters.LimbMovementPatterns = CalculateLimbMovementPatterns(genome);
        parameters.BodyMovementPatterns = CalculateBodyMovementPatterns(genome);
        parameters.BalanceSystem = CalculateBalanceSystem(genome);
        
        // Calculate neural control parameters
        parameters.NeuralControlLevel = CalculateNeuralControlLevel(genome);
        parameters.LearningRate = CalculateLearningRate(genome);
        
        return parameters;
    }

    /// <summary>
    /// Calculate base speed of the creature
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Speed factor (1.0 = normal speed)</returns>
    private double CalculateBaseSpeed(Genome genome)
    {
        // Speed based on neural activity and muscle-related genes
        double neuralActivity = 0.0;
        int neuralGenes = 0;
        double muscleExpression = 0.0;
        int muscleGenes = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("neuron") || gene.Id.Contains("brain") || gene.Id.Contains("nn"))
                {
                    neuralActivity += gene.ExpressionLevel;
                    neuralGenes++;
                }
                
                if (gene.Id.Contains("muscle") || gene.Id.Contains("strength") || gene.Id.Contains("power"))
                {
                    muscleExpression += gene.ExpressionLevel;
                    muscleGenes++;
                }
            }
        }
        
        double avgNeural = neuralGenes > 0 ? neuralActivity / neuralGenes : 0.5;
        double avgMuscle = muscleGenes > 0 ? muscleExpression / muscleGenes : 0.5;
        
        // Combine neural and muscle factors for speed
        double speedFactor = (avgNeural + avgMuscle) / 2.0;
        
        // Map to speed range (0.1-3.0)
        return Math.Max(0.1, Math.Min(3.0, 1.0 + (speedFactor - 0.5) * 4.0));
    }

    /// <summary>
    /// Calculate movement type based on genetic data
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Movement type</returns>
    private MovementType CalculateMovementType(Genome genome)
    {
        // Count genes related to different movement types
        int walkingGenes = 0, flyingGenes = 0, swimmingGenes = 0, crawlingGenes = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("walk") || gene.Id.Contains("leg") || gene.Id.Contains("biped"))
                    walkingGenes += (int)(gene.ExpressionLevel * 10);
                
                if (gene.Id.Contains("fly") || gene.Id.Contains("wing") || gene.Id.Contains("air"))
                    flyingGenes += (int)(gene.ExpressionLevel * 10);
                
                if (gene.Id.Contains("swim") || gene.Id.Contains("fin") || gene.Id.Contains("water"))
                    swimmingGenes += (int)(gene.ExpressionLevel * 10);
                
                if (gene.Id.Contains("crawl") || gene.Id.Contains("slither") || gene.Id.Contains("tentacle"))
                    crawlingGenes += (int)(gene.ExpressionLevel * 10);
            }
        }
        
        // Determine dominant movement type
        int maxGenes = Math.Max(Math.Max(walkingGenes, flyingGenes), Math.Max(swimmingGenes, crawlingGenes));
        
        if (maxGenes == walkingGenes && walkingGenes > 0)
            return MovementType.Walking;
        else if (maxGenes == flyingGenes && flyingGenes > 0)
            return MovementType.Flying;
        else if (maxGenes == swimmingGenes && swimmingGenes > 0)
            return MovementType.Swimming;
        else if (maxGenes == crawlingGenes && crawlingGenes > 0)
            return MovementType.Crawling;
        
        return MovementType.Walking; // Default
    }

    /// <summary>
    /// Calculate gait complexity level
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Gait complexity (1-10)</returns>
    private int CalculateGaitComplexity(Genome genome)
    {
        int complexity = 1;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("gait") || gene.Id.Contains("coordination") || gene.Id.Contains("balance"))
                {
                    complexity = Math.Max(complexity, (int)(gene.ExpressionLevel * 10));
                }
            }
        }
        
        return Math.Max(1, Math.Min(10, complexity));
    }

    /// <summary>
    /// Calculate limb movement patterns
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>List of movement patterns</returns>
    private List<string> CalculateLimbMovementPatterns(Genome genome)
    {
        var patterns = new List<string>();
        int limbCount = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("limb") || gene.Id.Contains("appendage"))
                {
                    limbCount++;
                    
                    if (gene.ExpressionLevel > 0.7)
                        patterns.Add("synchronized");
                    else if (gene.ExpressionLevel > 0.4)
                        patterns.Add("alternating");
                    else
                        patterns.Add("independent");
                }
            }
        }
        
        if (patterns.Count == 0)
        {
            patterns.Add("synchronized");
        }
        
        return patterns;
    }

    /// <summary>
    /// Calculate body movement patterns
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>List of body movement patterns</returns>
    private List<string> CalculateBodyMovementPatterns(Genome genome)
    {
        var patterns = new List<string>();
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("body") || gene.Id.Contains("spine") || gene.Id.Contains("flex"))
                {
                    if (gene.ExpressionLevel > 0.6)
                        patterns.Add("undulating");
                    else if (gene.ExpressionLevel > 0.3)
                        patterns.Add("segmented");
                    else
                        patterns.Add("rigid");
                }
            }
        }
        
        if (patterns.Count == 0)
        {
            patterns.Add("segmented");
        }
        
        return patterns;
    }

    /// <summary>
    /// Calculate balance system type
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Balance system type</returns>
    private BalanceSystem CalculateBalanceSystem(Genome genome)
    {
        int innerEarGenes = 0, visualGenes = 0, proprioceptiveGenes = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("ear") || gene.Id.Contains("vestibular") || gene.Id.Contains("inner"))
                    innerEarGenes += (int)(gene.ExpressionLevel * 10);
                
                if (gene.Id.Contains("eye") || gene.Id.Contains("vision") || gene.Id.Contains("visual"))
                    visualGenes += (int)(gene.ExpressionLevel * 10);
                
                if (gene.Id.Contains("proprio") || gene.Id.Contains("sense") || gene.Id.Contains("position"))
                    proprioceptiveGenes += (int)(gene.ExpressionLevel * 10);
            }
        }
        
        int maxGenes = Math.Max(Math.Max(innerEarGenes, visualGenes), proprioceptiveGenes);
        
        if (maxGenes == innerEarGenes && innerEarGenes > 0)
            return BalanceSystem.InnerEar;
        else if (maxGenes == visualGenes && visualGenes > 0)
            return BalanceSystem.Visual;
        else if (maxGenes == proprioceptiveGenes && proprioceptiveGenes > 0)
            return BalanceSystem.Proprioceptive;
        
        return BalanceSystem.InnerEar; // Default
    }

    /// <summary>
    /// Calculate neural control level
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Neural control level (0.0-1.0)</returns>
    private double CalculateNeuralControlLevel(Genome genome)
    {
        double controlLevel = 0.5;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("control") || gene.Id.Contains("neural") || gene.Id.Contains("motor"))
                {
                    controlLevel = Math.Max(0.1, Math.Min(1.0, gene.ExpressionLevel));
                    break;
                }
            }
        }
        
        return controlLevel;
    }

    /// <summary>
    /// Calculate learning rate for movement adaptation
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Learning rate (0.0-1.0)</returns>
    private double CalculateLearningRate(Genome genome)
    {
        double learningRate = 0.1;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("learn") || gene.Id.Contains("adapt") || gene.Id.Contains("plasticity"))
                {
                    learningRate = Math.Max(0.01, Math.Min(0.5, gene.ExpressionLevel * 0.5));
                    break;
                }
            }
        }
        
        return learningRate;
    }
}

/// <summary>
/// Parameters for movement generation
/// </summary>
public class MovementParameters
{
    /// <summary>
    /// Base speed factor
    /// </summary>
    public double BaseSpeed { get; set; }

    /// <summary>
    /// Movement type
    /// </summary>
    public MovementType MovementType { get; set; }

    /// <summary>
    /// Gait complexity level (1-10)
    /// </summary>
    public int GaitComplexity { get; set; }

    /// <summary>
    /// List of limb movement patterns
    /// </summary>
    public List<string> LimbMovementPatterns { get; set; } = new List<string>();

    /// <summary>
    /// List of body movement patterns
    /// </summary>
    public List<string> BodyMovementPatterns { get; set; } = new List<string>();

    /// <summary>
    /// Balance system type
    /// </summary>
    public BalanceSystem BalanceSystem { get; set; }

    /// <summary>
    /// Neural control level (0.0-1.0)
    /// </summary>
    public double NeuralControlLevel { get; set; }

    /// <summary>
    /// Learning rate for movement adaptation
    /// </summary>
    public double LearningRate { get; set; }
}

/// <summary>
/// Types of movement
/// </summary>
public enum MovementType
{
    /// <summary>
    /// Walking/running on legs
    /// </summary>
    Walking,
    
    /// <summary>
    /// Flying through air
    /// </summary>
    Flying,
    
    /// <summary>
    /// Swimming through water
    /// </summary>
    Swimming,
    
    /// <summary>
    /// Crawling/slithering along surfaces
    /// </summary>
    Crawling
}

/// <summary>
/// Types of balance systems
/// </summary>
public enum BalanceSystem
{
    /// <summary>
    /// Inner ear vestibular system
    /// </summary>
    InnerEar,
    
    /// <summary>
    /// Visual system
    /// </summary>
    Visual,
    
    /// <summary>
    /// Proprioceptive system (body position sensing)
    /// </summary>
    Proprioceptive
}