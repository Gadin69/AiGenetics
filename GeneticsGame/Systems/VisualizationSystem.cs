using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Visualization system for the 3D Genetics Game
/// Converts genetic and neural data to visual representations
/// </summary>
public class VisualizationSystem
{
    /// <summary>
    /// Current genome being visualized
    /// </summary>
    public Genome Genome { get; set; }

    /// <summary>
    /// Current neural network being visualized
    /// </summary>
    public DynamicNeuralNetwork NeuralNetwork { get; set; }

    /// <summary>
    /// Constructor for VisualizationSystem
    /// </summary>
    /// <param name="genome">Genome to visualize</param>
    /// <param name="neuralNetwork">Neural network to visualize</param>
    public VisualizationSystem(Genome genome, DynamicNeuralNetwork neuralNetwork)
    {
        Genome = genome;
        NeuralNetwork = neuralNetwork;
    }

    /// <summary>
    /// Generate visualization parameters from genetic and neural data
    /// </summary>
    /// <returns>Visualization parameters object</returns>
    public VisualizationParameters GenerateVisualizationParameters()
    {
        var parameters = new VisualizationParameters();
        
        // Calculate visual complexity based on genetic and neural factors
        parameters.ComplexityLevel = CalculateComplexityLevel();
        
        // Calculate color palette
        parameters.ColorPalette = CalculateColorPalette();
        
        // Calculate animation parameters
        parameters.AnimationParameters = CalculateAnimationParameters();
        
        // Calculate neural visualization parameters
        parameters.NeuralVisualization = CalculateNeuralVisualization();
        
        return parameters;
    }

    /// <summary>
    /// Calculate overall visual complexity level
    /// </summary>
    /// <returns>Complexity level (1-10)</returns>
    private int CalculateComplexityLevel()
    {
        int complexity = 1;
        
        // Base complexity from genome
        complexity += Genome.Chromosomes.Count;
        
        foreach (var chromosome in Genome.Chromosomes)
        {
            complexity += chromosome.Genes.Count / 5;
        }
        
        // Add neural complexity
        complexity += NeuralNetwork.Neurons.Count / 10;
        complexity += NeuralNetwork.Connections.Count / 20;
        
        return Math.Max(1, Math.Min(10, complexity));
    }

    /// <summary>
    /// Calculate color palette based on genetic data
    /// </summary>
    /// <returns>List of colors</returns>
    private List<Color> CalculateColorPalette()
    {
        var palette = new List<Color>();
        
        // Get base colors from genome
        var meshGenerator = new MeshProceduralGenerator();
        var meshParams = meshGenerator.GenerateMeshParameters(Genome);
        
        palette.AddRange(meshParams.Colors);
        
        // Add neural-specific colors
        if (NeuralNetwork.Neurons.Count > 0)
        {
            // Visual neurons - blue
            palette.Add(new Color(0.2, 0.4, 0.8));
            
            // Learning neurons - green
            palette.Add(new Color(0.2, 0.8, 0.4));
            
            // Mutation neurons - red
            palette.Add(new Color(0.8, 0.2, 0.4));
            
            // Movement neurons - yellow
            palette.Add(new Color(0.8, 0.8, 0.2));
        }
        
        return palette;
    }

    /// <summary>
    /// Calculate animation parameters based on movement and neural data
    /// </summary>
    /// <returns>Animation parameters</returns>
    private AnimationParameters CalculateAnimationParameters()
    {
        var parameters = new AnimationParameters();
        
        // Animation speed based on neural activity
        parameters.Speed = NeuralNetwork.ActivityLevel * 2.0 + 0.5;
        
        // Complexity based on neural connections
        parameters.Complexity = NeuralNetwork.Connections.Count / 50.0;
        
        // Smoothness based on neural control level
        parameters.Smoothness = 0.7 + (NeuralNetwork.Neurons.Count > 0 ? 
            NeuralNetwork.Neurons.Average(n => n.Threshold) * 0.3 : 0.0);
        
        return parameters;
    }

    /// <summary>
    /// Calculate neural visualization parameters
    /// </summary>
    /// <returns>Neural visualization parameters</returns>
    private NeuralVisualizationParameters CalculateNeuralVisualization()
    {
        var parameters = new NeuralVisualizationParameters();
        
        // Neuron density
        parameters.NeuronDensity = NeuralNetwork.Neurons.Count;
        
        // Connection density
        parameters.ConnectionDensity = NeuralNetwork.Connections.Count;
        
        // Activity visualization
        parameters.ActivityLevel = NeuralNetwork.ActivityLevel;
        
        // Type distribution
        parameters.NeuronTypeDistribution = new Dictionary<NeuronType, int>();
        
        foreach (var neuron in NeuralNetwork.Neurons)
        {
            if (parameters.NeuronTypeDistribution.ContainsKey(neuron.Type))
            {
                parameters.NeuronTypeDistribution[neuron.Type]++;
            }
            else
            {
                parameters.NeuronTypeDistribution[neuron.Type] = 1;
            }
        }
        
        return parameters;
    }
}

/// <summary>
/// Parameters for visualization
/// </summary>
public class VisualizationParameters
{
    /// <summary>
    /// Visual complexity level (1-10)
    /// </summary>
    public int ComplexityLevel { get; set; }

    /// <summary>
    /// Color palette
    /// </summary>
    public List<Color> ColorPalette { get; set; } = new List<Color>();

    /// <summary>
    /// Animation parameters
    /// </summary>
    public AnimationParameters AnimationParameters { get; set; } = new AnimationParameters();

    /// <summary>
    /// Neural visualization parameters
    /// </summary>
    public NeuralVisualizationParameters NeuralVisualization { get; set; } = new NeuralVisualizationParameters();
}

/// <summary>
/// Animation parameters
/// </summary>
public class AnimationParameters
{
    /// <summary>
    /// Animation speed factor
    /// </summary>
    public double Speed { get; set; }

    /// <summary>
    /// Animation complexity level
    /// </summary>
    public double Complexity { get; set; }

    /// <summary>
    /// Animation smoothness (0.0-1.0)
    /// </summary>
    public double Smoothness { get; set; }
}

/// <summary>
/// Neural visualization parameters
/// </summary>
public class NeuralVisualizationParameters
{
    /// <summary>
    /// Number of neurons to visualize
    /// </summary>
    public int NeuronDensity { get; set; }

    /// <summary>
    /// Number of connections to visualize
    /// </summary>
    public int ConnectionDensity { get; set; }

    /// <summary>
    /// Current neural activity level
    /// </summary>
    public double ActivityLevel { get; set; }

    /// <summary>
    /// Distribution of neuron types
    /// </summary>
    public Dictionary<NeuronType, int> NeuronTypeDistribution { get; set; } = new Dictionary<NeuronType, int>();
}