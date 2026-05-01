using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Ensures mesh and movement systems stay in sync
/// Implements unified gene mapping that drives both visual and behavioral outputs
/// </summary>
public class SynchronizedGenerator
{
    /// <summary>
    /// Mesh generator instance
    /// </summary>
    public MeshProceduralGenerator MeshGenerator { get; set; }

    /// <summary>
    /// Movement generator instance
    /// </summary>
    public MovementProceduralGenerator MovementGenerator { get; set; }

    /// <summary>
    /// Constructor for SynchronizedGenerator
    /// </summary>
    public SynchronizedGenerator()
    {
        MeshGenerator = new MeshProceduralGenerator();
        MovementGenerator = new MovementProceduralGenerator();
    }

    /// <summary>
    /// Generate synchronized parameters from a genome
    /// </summary>
    /// <param name="genome">Genome to generate from</param>
    /// <returns>Synchronized parameters object</returns>
    public SynchronizedParameters GenerateSynchronizedParameters(Genome genome)
    {
        var parameters = new SynchronizedParameters();
        
        // Generate mesh parameters
        parameters.MeshParameters = MeshGenerator.GenerateMeshParameters(genome);
        
        // Generate movement parameters
        parameters.MovementParameters = MovementGenerator.GenerateMovementParameters(genome);
        
        // Ensure synchronization between mesh and movement
        parameters = SynchronizeParameters(parameters, genome);
        
        return parameters;
    }

    /// <summary>
    /// Synchronize mesh and movement parameters to ensure consistency
    /// </summary>
    /// <param name="parameters">Parameters to synchronize</param>
    /// <param name="genome">Genome used for generation</param>
    /// <returns>Synchronized parameters</returns>
    private SynchronizedParameters SynchronizeParameters(SynchronizedParameters parameters, Genome genome)
    {
        // Synchronize limb counts
        int meshLimbCount = parameters.MeshParameters.LimbCount;
        int movementLimbCount = parameters.MovementParameters.LimbMovementPatterns.Count;
        
        if (meshLimbCount != movementLimbCount)
        {
            // Adjust movement limb patterns to match mesh
            if (movementLimbCount > meshLimbCount)
            {
                parameters.MovementParameters.LimbMovementPatterns = 
                    parameters.MovementParameters.LimbMovementPatterns.Take(meshLimbCount).ToList();
            }
            else if (movementLimbCount < meshLimbCount)
            {
                // Add default patterns
                while (parameters.MovementParameters.LimbMovementPatterns.Count < meshLimbCount)
                {
                    parameters.MovementParameters.LimbMovementPatterns.Add("synchronized");
                }
            }
        }
        
        // Synchronize body segments
        int meshSegments = parameters.MeshParameters.BodySegments;
        int movementSegments = parameters.MovementParameters.BodyMovementPatterns.Count;
        
        if (meshSegments != movementSegments)
        {
            if (movementSegments > meshSegments)
            {
                parameters.MovementParameters.BodyMovementPatterns = 
                    parameters.MovementParameters.BodyMovementPatterns.Take(meshSegments).ToList();
            }
            else if (movementSegments < meshSegments)
            {
                while (parameters.MovementParameters.BodyMovementPatterns.Count < meshSegments)
                {
                    parameters.MovementParameters.BodyMovementPatterns.Add("segmented");
                }
            }
        }
        
        // Synchronize speed and scale relationships
        // Larger creatures should generally be slower
        double sizeFactor = parameters.MeshParameters.BaseScale;
        double speedFactor = parameters.MovementParameters.BaseSpeed;
        
        // Apply size-speed relationship: larger creatures are slower (within reason)
        if (sizeFactor > 1.5 && speedFactor > 1.2)
        {
            parameters.MovementParameters.BaseSpeed = Math.Max(0.5, speedFactor * (2.0 / sizeFactor));
        }
        
        // Smaller creatures can be faster
        if (sizeFactor < 0.7 && speedFactor < 1.0)
        {
            parameters.MovementParameters.BaseSpeed = Math.Min(2.5, speedFactor * (1.5 / sizeFactor));
        }
        
        // Synchronize neural control with mesh complexity
        double meshComplexityFactor = parameters.MeshParameters.MeshComplexity / 10.0;
        parameters.MovementParameters.NeuralControlLevel = 
            Math.Max(0.1, Math.Min(1.0, parameters.MovementParameters.NeuralControlLevel + meshComplexityFactor * 0.2));
        
        return parameters;
    }
}

/// <summary>
/// Parameters for synchronized mesh and movement generation
/// </summary>
public class SynchronizedParameters
{
    /// <summary>
    /// Mesh parameters
    /// </summary>
    public MeshParameters MeshParameters { get; set; } = new MeshParameters();

    /// <summary>
    /// Movement parameters
    /// </summary>
    public MovementParameters MovementParameters { get; set; } = new MovementParameters();
}