using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Converts genetic data to 3D mesh parameters
/// Implements procedural mesh generation based on genetic traits
/// </summary>
public class MeshProceduralGenerator
{
    /// <summary>
    /// Generate mesh parameters from a genome
    /// </summary>
    /// <param name="genome">Genome to convert</param>
    /// <returns>Mesh parameters object</returns>
    public MeshParameters GenerateMeshParameters(Genome genome)
    {
        var parameters = new MeshParameters();
        
        // Calculate base mesh properties from genome
        parameters.BaseScale = CalculateBaseScale(genome);
        parameters.MeshComplexity = CalculateMeshComplexity(genome);
        parameters.VertexCount = CalculateVertexCount(genome);
        
        // Calculate visual features
        parameters.Colors = CalculateColors(genome);
        parameters.Patterns = CalculatePatterns(genome);
        parameters.TextureDetail = CalculateTextureDetail(genome);
        
        // Calculate structural features
        parameters.LimbCount = CalculateLimbCount(genome);
        parameters.LimbLengths = CalculateLimbLengths(genome);
        parameters.BodySegments = CalculateBodySegments(genome);
        
        return parameters;
    }

    /// <summary>
    /// Calculate base scale of the creature
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Scale factor (1.0 = normal size)</returns>
    private double CalculateBaseScale(Genome genome)
    {
        // Scale based on overall gene expression levels
        double totalExpression = 0.0;
        int geneCount = 0;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                totalExpression += gene.ExpressionLevel;
                geneCount++;
            }
        }
        
        double averageExpression = geneCount > 0 ? totalExpression / geneCount : 0.5;
        
        // Map expression to scale (0.5-2.0 range)
        return Math.Max(0.5, Math.Min(2.0, 1.0 + (averageExpression - 0.5) * 2.0));
    }

    /// <summary>
    /// Calculate mesh complexity level
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Complexity level (0-10)</returns>
    private int CalculateMeshComplexity(Genome genome)
    {
        // Complexity based on number of genes and chromosomes
        int complexity = genome.Chromosomes.Count * 2;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            complexity += chromosome.Genes.Count / 5;
        }
        
        return Math.Max(1, Math.Min(10, complexity));
    }

    /// <summary>
    /// Calculate vertex count for the mesh
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Vertex count</returns>
    private int CalculateVertexCount(Genome genome)
    {
        int baseVertices = 1000;
        int complexity = CalculateMeshComplexity(genome);
        
        // More complex creatures have more vertices
        return baseVertices * complexity;
    }

    /// <summary>
    /// Calculate color scheme from genetic data
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>List of colors</returns>
    private List<Color> CalculateColors(Genome genome)
    {
        var colors = new List<Color>();
        
        // Find color-related genes
        var colorGenes = new List<Gene<double>>();
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("color") || gene.Id.Contains("pigment") || gene.Id.Contains("melanin"))
                {
                    colorGenes.Add(gene);
                }
            }
        }
        
        if (colorGenes.Count == 0)
        {
            // Default colors
            colors.Add(new Color(0.8, 0.6, 0.4)); // Brownish
            colors.Add(new Color(0.9, 0.9, 0.9)); // Light
        }
        else
        {
            // Generate colors based on gene expression
            foreach (var gene in colorGenes)
            {
                colors.Add(new Color(
                    Math.Max(0.1, Math.Min(1.0, gene.ExpressionLevel * 0.8 + 0.2)),
                    Math.Max(0.1, Math.Min(1.0, gene.ExpressionLevel * 0.6 + 0.1)),
                    Math.Max(0.1, Math.Min(1.0, gene.ExpressionLevel * 0.4 + 0.05))
                ));
            }
        }
        
        return colors;
    }

    /// <summary>
    /// Calculate pattern types from genetic data
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>List of patterns</returns>
    private List<string> CalculatePatterns(Genome genome)
    {
        var patterns = new List<string>();
        
        // Find pattern-related genes
        var patternGenes = new List<Gene<double>>();
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("pattern") || gene.Id.Contains("stripe") || gene.Id.Contains("spot"))
                {
                    patternGenes.Add(gene);
                }
            }
        }
        
        if (patternGenes.Count == 0)
        {
            patterns.Add("solid");
        }
        else
        {
            foreach (var gene in patternGenes)
            {
                if (gene.ExpressionLevel > 0.7)
                {
                    patterns.Add("striped");
                }
                else if (gene.ExpressionLevel > 0.4)
                {
                    patterns.Add("spotted");
                }
                else
                {
                    patterns.Add("mottled");
                }
            }
        }
        
        return patterns;
    }

    /// <summary>
    /// Calculate texture detail level
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Texture detail level (0-5)</returns>
    private int CalculateTextureDetail(Genome genome)
    {
        int detail = 1;
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("texture") || gene.Id.Contains("detail") || gene.Id.Contains("roughness"))
                {
                    detail = Math.Max(detail, (int)(gene.ExpressionLevel * 5));
                }
            }
        }
        
        return Math.Max(1, Math.Min(5, detail));
    }

    /// <summary>
    /// Calculate number of limbs
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Limb count</returns>
    private int CalculateLimbCount(Genome genome)
    {
        int limbCount = 4; // Default
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("limb") || gene.Id.Contains("appendage"))
                {
                    limbCount = Math.Max(0, Math.Min(12, (int)(gene.ExpressionLevel * 12)));
                    break;
                }
            }
        }
        
        return limbCount;
    }

    /// <summary>
    /// Calculate limb lengths
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>List of limb lengths</returns>
    private List<double> CalculateLimbLengths(Genome genome)
    {
        var lengths = new List<double>();
        int limbCount = CalculateLimbCount(genome);
        
        for (int i = 0; i < limbCount; i++)
        {
            // Vary limb lengths based on genetic expression
            double length = 1.0 + (Random.Shared.NextDouble() - 0.5) * 0.5;
            lengths.Add(length);
        }
        
        return lengths;
    }

    /// <summary>
    /// Calculate body segments
    /// </summary>
    /// <param name="genome">Genome to calculate from</param>
    /// <returns>Number of body segments</returns>
    private int CalculateBodySegments(Genome genome)
    {
        int segments = 3; // Default head, torso, tail
        
        foreach (var chromosome in genome.Chromosomes)
        {
            foreach (var gene in chromosome.Genes)
            {
                if (gene.Id.Contains("segment") || gene.Id.Contains("body"))
                {
                    segments = Math.Max(1, Math.Min(10, (int)(gene.ExpressionLevel * 10)));
                    break;
                }
            }
        }
        
        return segments;
    }
}

/// <summary>
/// Parameters for mesh generation
/// </summary>
public class MeshParameters
{
    /// <summary>
    /// Base scale factor
    /// </summary>
    public double BaseScale { get; set; }

    /// <summary>
    /// Mesh complexity level (1-10)
    /// </summary>
    public int MeshComplexity { get; set; }

    /// <summary>
    /// Number of vertices
    /// </summary>
    public int VertexCount { get; set; }

    /// <summary>
    /// List of colors
    /// </summary>
    public List<Color> Colors { get; set; } = new List<Color>();

    /// <summary>
    /// List of patterns
    /// </summary>
    public List<string> Patterns { get; set; } = new List<string>();

    /// <summary>
    /// Texture detail level (1-5)
    /// </summary>
    public int TextureDetail { get; set; }

    /// <summary>
    /// Number of limbs
    /// </summary>
    public int LimbCount { get; set; }

    /// <summary>
    /// List of limb lengths
    /// </summary>
    public List<double> LimbLengths { get; set; } = new List<double>();

    /// <summary>
    /// Number of body segments
    /// </summary>
    public int BodySegments { get; set; }
}

/// <summary>
/// Represents a color in RGB format
/// </summary>
public class Color
{
    /// <summary>
    /// Red component (0.0-1.0)
    /// </summary>
    public double R { get; set; }

    /// <summary>
    /// Green component (0.0-1.0)
    /// </summary>
    public double G { get; set; }

    /// <summary>
    /// Blue component (0.0-1.0)
    /// </summary>
    public double B { get; set; }

    /// <summary>
    /// Constructor for Color
    /// </summary>
    /// <param name="r">Red component</param>
    /// <param name="g">Green component</param>
    /// <param name="b">Blue component</param>
    public Color(double r, double g, double b)
    {
        R = r;
        G = g;
        B = b;
    }
}