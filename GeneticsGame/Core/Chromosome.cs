using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Collection of genes with structural mutation support
/// Represents a chromosome in the genetic system
/// </summary>
public class Chromosome
{
    /// <summary>
    /// Unique identifier for this chromosome
    /// </summary>
    public string Id { get; set; }

    /// <summary>
    /// List of genes in this chromosome
    /// </summary>
    public List<Gene<double>> Genes { get; set; }

    /// <summary>
    /// Constructor for Chromosome
    /// </summary>
    /// <param name="id">Unique identifier</param>
    public Chromosome(string id)
    {
        Id = id;
        Genes = new List<Gene<double>>();
    }

    /// <summary>
    /// Add a gene to this chromosome
    /// </summary>
    /// <param name="gene">Gene to add</param>
    public void AddGene(Gene<double> gene)
    {
        Genes.Add(gene);
    }

    /// <summary>
    /// Apply structural mutations to this chromosome
    /// </summary>
    /// <returns>True if structural mutation occurred</returns>
    public bool ApplyStructuralMutation()
    {
        // Randomly choose structural mutation type
        int mutationType = Random.Shared.Next(0, 4);
        
        switch (mutationType)
        {
            case 0:
                return ApplyDeletion();
            case 1:
                return ApplyDuplication();
            case 2:
                return ApplyInversion();
            case 3:
                return ApplyTranslocation();
            default:
                return false;
        }
    }

    /// <summary>
    /// Apply deletion mutation - remove a random segment of genes
    /// </summary>
    /// <returns>True if deletion occurred</returns>
    private bool ApplyDeletion()
    {
        if (Genes.Count < 2) return false;
        
        int startIndex = Random.Shared.Next(0, Genes.Count - 1);
        int length = Math.Max(1, Random.Shared.Next(1, Math.Min(5, Genes.Count - startIndex)));
        
        Genes.RemoveRange(startIndex, length);
        return true;
    }

    /// <summary>
    /// Apply duplication mutation - duplicate a random segment of genes
    /// </summary>
    /// <returns>True if duplication occurred</returns>
    private bool ApplyDuplication()
    {
        if (Genes.Count < 2) return false;
        
        int startIndex = Random.Shared.Next(0, Genes.Count - 1);
        int length = Math.Max(1, Random.Shared.Next(1, Math.Min(5, Genes.Count - startIndex)));
        
        var segment = Genes.Skip(startIndex).Take(length).ToList();
        Genes.InsertRange(startIndex, segment);
        return true;
    }

    /// <summary>
    /// Apply inversion mutation - invert a random segment of genes
    /// </summary>
    /// <returns>True if inversion occurred</returns>
    private bool ApplyInversion()
    {
        if (Genes.Count < 2) return false;
        
        int startIndex = Random.Shared.Next(0, Genes.Count - 1);
        int length = Math.Max(1, Random.Shared.Next(1, Math.Min(5, Genes.Count - startIndex)));
        
        var segment = Genes.Skip(startIndex).Take(length).ToList();
        segment.Reverse();
        
        for (int i = 0; i < segment.Count; i++)
        {
            Genes[startIndex + i] = segment[i];
        }
        
        return true;
    }

    /// <summary>
    /// Apply translocation mutation - move a segment to a different location
    /// </summary>
    /// <returns>True if translocation occurred</returns>
    private bool ApplyTranslocation()
    {
        if (Genes.Count < 3) return false;
        
        int startIndex = Random.Shared.Next(0, Genes.Count - 2);
        int length = Math.Max(1, Random.Shared.Next(1, Math.Min(5, Genes.Count - startIndex)));
        
        var segment = Genes.Skip(startIndex).Take(length).ToList();
        Genes.RemoveRange(startIndex, length);
        
        // Insert at random position
        int insertPosition = Random.Shared.Next(0, Math.Max(1, Genes.Count));
        Genes.InsertRange(insertPosition, segment);
        
        return true;
    }

    /// <summary>
    /// Get total neuron growth potential from all active genes in this chromosome
    /// </summary>
    /// <returns>Total neuron growth count</returns>
    public int GetTotalNeuronGrowthCount()
    {
        return Genes.Sum(gene => gene.GetNeuronGrowthCount());
    }
}