# 3D Genetics Game Architecture Plan

## Core Design Principles
- **Biological Taxonomy Organization**: Phylum-level folder structure with inheritance hierarchy
- **Dynamic Neural Networks**: NNs that grow new neurons based on genetic triggers, activity, and mutations
- **Simultaneous Procedural Systems**: Tight integration between mesh generation and movement systems
- **Performance-First**: Optimized for real-time simulation with efficient genetic algorithms

## Folder Structure (Phylum-Level)
```
/GeneticsGame/
├── /Core/
│   ├── GeneticsCore.cs          # Base genetic framework
│   └── MutationSystem.cs      # Core mutation engine
├── /Phyla/
│   ├── /Chordata/             # Vertebrate-like creatures
│   │   ├── ChordataCreature.cs
│   │   ├── ChordataGenome.cs
│   │   └── /Neural/
│   │       ├── ChordataNN.cs
│   │       └── ChordataNeuronGrowth.cs
│   ├── /Arthropoda/           # Insect/crustacean-like creatures
│   │   ├── ArthropodaCreature.cs
│   │   └── ArthropodaGenome.cs
│   └── /Mollusca/             # Soft-bodied creatures
│       └── MolluscaCreature.cs
├── /Procedural/
│   ├── /Mesh/
│   │   ├── MeshGenerator.cs
│   │   └── GeneToMeshMapper.cs
│   └── /Movement/
│       ├── MovementGenerator.cs
│       └── GeneToMovementMapper.cs
└── /Systems/
    ├── BreedingSystem.cs
    ├── NeuralLearningSystem.cs
    └── VisualizationSystem.cs
```

## Core Class Hierarchy

### Genetic Foundation Classes
- `Gene<T>`: Generic gene class with expression levels, mutation rates, and neural growth parameters
- `Chromosome`: Collection of genes with structural mutation support
- `Genome`: Complete genetic blueprint with multi-gene interaction rules

### Neural Network System
- `DynamicNeuralNetwork`: Base class supporting runtime neuron addition
- `NeuronGrowthController`: Manages neuron creation based on genetic triggers
- `ActivityBasedSynapseBuilder`: Creates connections based on neural activity patterns
- `MutationNeuron`: Specialized neuron type activated by specific mutations

### Procedural Generation Systems
- `MeshProceduralGenerator`: Converts genetic data to 3D mesh parameters
- `MovementProceduralGenerator`: Converts genetic data to locomotion patterns
- `SynchronizedGenerator`: Ensures mesh and movement systems stay in sync

## Key Implementation Details

### Neural Network Growth System
- **Hybrid Triggers**: 
  - *Genetic Expression*: Genes with `neuronGrowthFactor` property activate growth
  - *Mutation Events*: Specific mutations trigger immediate neuron addition
  - *Learning Feedback*: Activity patterns in existing neurons stimulate new neuron growth
- **No Predefined Purpose**: New neurons start with random connectivity, gain purpose through Hebbian learning
- **Performance Optimization**: Spatial partitioning and activity thresholds prevent runaway growth

### Multi-Gene Interactions
- **Epistatic Networks**: Genes influence each other's expression levels
- **Polygenic Traits**: Visual features controlled by multiple interacting genes
- **Regulatory Genes**: Control expression timing and intensity of structural genes

### Procedural Integration
- **Unified Gene Mapping**: Single genetic representation drives both mesh and movement systems
- **Real-time Synchronization**: Changes to genome immediately update both visual and behavioral outputs
- **Performance Constraints**: LOD system for complex neural networks and mesh detail

## Initial Development Milestones
1. **Core Genetics Framework**: Gene, Chromosome, Genome classes with inheritance
2. **Neural Growth Prototype**: Basic NN that adds neurons based on genetic triggers
3. **Mesh-Movement Integration**: Simple creature with synchronized procedural generation
4. **Full Breeding System**: Offspring generation with mutation simulation
5. **Complete Integration**: All systems working together with performance optimization