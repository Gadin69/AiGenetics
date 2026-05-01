using System;
using System.Collections.Generic;
using System.Linq;

/// <summary>
/// Base genetic framework for the 3D Genetics Game
/// Implements biological taxonomy organization with inheritance hierarchy
/// </summary>
public static class GeneticsCore
{
    /// <summary>
    /// Global configuration for genetic systems
    /// </summary>
    public static class Config
    {
        public const double DefaultMutationRate = 0.001;
        public const int MaxNeuronGrowthPerGeneration = 100;
        public const double NeuralActivityThreshold = 0.7;
    }
}
