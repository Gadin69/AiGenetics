# 3D Genetics Game Engine Development Roadmap

## Overview

This document outlines the comprehensive development roadmap for the 3D Genetics Game Engine, implementing ARK-style breeding with Dragon Adventures genetics. The architecture follows OOP principles with inheritance, polymorphism, and proper folder organization as requested.

The engine will support dynamic neural networks that grow at runtime, voxel-based procedural mesh generation synchronized with genetic data, and advanced rendering features including frustum culling, LOD systems, and PBR materials.

## Phase 1: Advanced 3D Rendering System

### Objective
Implement a full-featured camera system with frustum culling, LOD systems, and advanced projection modes while maintaining strict separation between engine and game logic.

### Technical Specifications
- **Camera System**: Hierarchical camera controller supporting orbit, first-person, and cinematic modes with smooth interpolation
- **Frustum Culling**: CPU-side frustum culling using bounding spheres and AABBs with spatial partitioning
- **LOD System**: Multi-level detail system with automatic switching based on distance, screen coverage, and performance metrics
- **Projection Modes**: Support for perspective, orthographic, and custom projection matrices
- **Memory Management**: GPU memory pools for camera-related resources with efficient resource recycling

### File Organization
```
src/engine/rendering/
├── camera/
│   ├── CameraController.h/cpp
│   ├── CameraSystem.h/cpp
│   └── CameraTypes.h
├── culling/
│   ├── FrustumCuller.h/cpp
│   ├── SpatialPartition.h/cpp
│   └── BoundingVolume.h
├── lod/
│   ├── LODManager.h/cpp
│   ├── LODLevel.h/cpp
│   └── LODTransition.h
└── projection/
    ├── ProjectionMatrix.h/cpp
    └── ProjectionModes.h
```

### Implementation Details
1. **Camera Controller**: Implement `CameraController` class with inheritance hierarchy:
   - `BaseCameraController` (abstract base)
   - `OrbitCameraController` (for scene inspection)
   - `FirstPersonCameraController` (for gameplay)
   - `CinematicCameraController` (for cutscenes)

2. **Frustum Culling**: Use hierarchical spatial partitioning with octree structure for efficient culling of thousands of creatures

3. **LOD System**: Implement adaptive LOD switching based on:
   - Distance from camera
   - Screen-space coverage percentage
   - Current frame time budget
   - GPU memory pressure

4. **Engine/Game Separation**: All camera and rendering systems will be in `src/engine/` namespace, while game-specific camera behaviors will be in `src/game/` namespace.

## Phase 2: Genetics System Integration

### Objective
Connect the genetics framework to drive visual properties through inheritance-based biological taxonomy (Chordata, Arthropoda, Mollusca) with proper polymorphism.

### Technical Specifications
- **Taxonomy Hierarchy**: C++ inheritance tree with virtual methods for genetic expression
- **Genetic Expression**: Runtime binding between genetic data and visual parameters
- **Polymorphic Rendering**: Each taxonomic class implements its own rendering behavior
- **Inheritance Chain**: `Organism` → `Animal` → `Chordata`/`Arthropoda`/`Mollusca` → `Species` → `Individual`

### File Organization
```
src/engine/genetics/
├── taxonomy/
│   ├── Organism.h/cpp
│   ├── Animal.h/cpp
│   ├── Chordata.h/cpp
│   ├── Arthropoda.h/cpp
│   └── Mollusca.h/cpp
├── genome/
│   ├── Genome.h/cpp
│   ├── Gene.h/cpp
│   └── Chromosome.h/cpp
├── expression/
│   ├── GeneticExpression.h/cpp
│   └── ExpressionRules.h
└── breeding/
    ├── BreedingSystem.h/cpp
    └── MutationEngine.h/cpp
```

### Implementation Details
1. **Taxonomy Classes**: Each class inherits from parent and implements virtual methods:
   - `virtual void ApplyGeneticExpression(const Genome& genome) = 0;`
   - `virtual std::vector<Genome> BreedWith(const GeneticsCreature& other) const = 0;`
   - `virtual void ApplyMutation(float mutationRate) = 0;`

2. **Genetic Expression**: Map genetic loci to visual parameters using expression rules:
   - `Locus 0x1A2B`: Controls scale factor (range 0.5-3.0)
   - `Locus 0x3C4D`: Controls color palette index (0-7)
   - `Locus 0x5E6F`: Controls limb count (1-8)

3. **Visual Parameter Binding**: Each taxonomic class defines how genetic data maps to rendering parameters:
   - `Chordata`: Scale, limb count, skin texture, skeletal structure
   - `Arthropoda`: Exoskeleton thickness, segment count, joint flexibility
   - `Mollusca`: Shell spiral parameters, mantle texture, movement pattern

## Phase 3: Voxel-Based Procedural Mesh Generation

### Objective
Create creatures based on genetic data using voxel-based procedural generation with real-time mesh optimization.

### Technical Specifications
- **Voxel Representation**: 3D voxel grid with genetic parameter mapping
- **Real-time Optimization**: Dynamic mesh simplification and LOD generation
- **Genetic Synchronization**: Direct mapping from genetic loci to voxel parameters
- **Performance**: Support for 100+ creatures with 10k+ voxels each at 60 FPS

### File Organization
```
src/engine/procedural/
├── voxel/
│   ├── VoxelGrid.h/cpp
│   ├── VoxelRenderer.h/cpp
│   └── VoxelOptimizer.h/cpp
├── mesh/
│   ├── ProceduralMesh.h/cpp
│   ├── MeshGenerator.h/cpp
│   └── MeshOptimizer.h/cpp
└── generation/
    ├── CreatureGenerator.h/cpp
    └── GeneticMapper.h/cpp
```

### Implementation Details
1. **Voxel Grid System**: 
   - 64x64x64 voxel grid per creature (configurable)
   - Genetic loci map to voxel density, material type, and connectivity
   - Marching cubes algorithm for surface extraction

2. **Real-time Optimization**:
   - Adaptive mesh simplification based on viewing distance
   - Instanced rendering for identical genetic profiles
   - GPU-accelerated voxel processing where available

3. **Genetic Mapping**:
   - `Locus 0x7G8H`: Controls voxel resolution (32, 64, 128)
   - `Locus 0x9I0J`: Controls surface smoothness (0-100)
   - `Locus 0x1K2L`: Controls internal cavity formation

## Phase 4: Dynamic Neural Network System

### Objective
Implement neural networks that can grow and add new neurons at runtime, optimized for running many instances simultaneously.

### Technical Specifications
- **Dynamic Architecture**: Runtime graph expansion with no predefined limits
- **Multi-instance Optimization**: Shared weights and batched computation
- **Learning Integration**: Reinforcement learning with genetic fitness feedback
- **Performance**: Optimized for 1000+ concurrent neural networks

### File Organization
```
src/engine/neural/
├── core/
│   ├── NeuralNetwork.h/cpp
│   ├── Neuron.h/cpp
│   └── Synapse.h/cpp
├── growth/
│   ├── GrowthController.h/cpp
│   └── PruningEngine.h/cpp
├── learning/
│   ├── ReinforcementLearner.h/cpp
│   └── FitnessEvaluator.h/cpp
└── optimization/
    ├── BatchProcessor.h/cpp
    └── SharedWeights.h/cpp
```

### Implementation Details
1. **Dynamic Growth**: 
   - `NeuralNetwork::AddNeuron()` method that dynamically allocates memory
   - Graph-based neuron connections with adjacency lists
   - Memory pooling for efficient allocation/deallocation

2. **Multi-instance Optimization**:
   - Shared weight matrices across similar genetic profiles
   - Batched forward/backward propagation
   - GPU acceleration for matrix operations

3. **Learning Integration**: 
   - Genetic fitness scores feed into reinforcement learning rewards
   - Neural network outputs influence genetic mutation rates
   - Real-time learning during gameplay simulation

## Phase 5: PBR Lighting and Materials System

### Objective
Implement physically based rendering with roughness/metallic workflow for realistic creature appearance.

### Technical Specifications
- **PBR Workflow**: Roughness/Metallic material model with proper energy conservation
- **Material Library**: Predefined material presets for different biological tissues
- **Real-time Updates**: Dynamic material parameter updates based on genetic expression
- **Optimization**: Efficient GPU resource management for thousands of materials

### File Organization
```
src/engine/rendering/materials/
├── pbr/
│   ├── PBRMaterial.h/cpp
│   ├── MaterialSystem.h/cpp
│   └── MaterialCache.h/cpp
├── textures/
│   ├── TextureAtlas.h/cpp
│   └── ProceduralTexture.h/cpp
└── biological/
    ├── SkinMaterial.h/cpp
    ├── ExoskeletonMaterial.h/cpp
    └── ShellMaterial.h/cpp
```

### Implementation Details
1. **PBR Implementation**: 
   - Cook-Torrance BRDF with GGX normal distribution
   - Energy-conserving Fresnel and geometry terms
   - Proper handling of metallic vs dielectric surfaces

2. **Biological Materials**:
   - `SkinMaterial`: Subsurface scattering, moisture control, pigment variation
   - `ExoskeletonMaterial`: Anisotropic scaling, chitin reflection, wear patterns
   - `ShellMaterial`: Iridescence, nacre layers, spiral parameter mapping

3. **Genetic Integration**: 
   - `Locus 0x3M4N`: Controls roughness (0.0-1.0)
   - `Locus 0x5O6P`: Controls metallic property (0.0-1.0)
   - `Locus 0x7Q8R`: Controls subsurface scattering intensity

## Phase 6: Developer UI Tools (Dear ImGui Integration)

### Objective
Integrate Dear ImGui into the DX12 engine to create developer tools for inspecting creatures, visualizing neural networks, controlling breeding, and monitoring simulation.

### Implementation Steps

**6.1: Dear ImGui Integration**
- Add Dear ImGui library via vcpkg or submodule
- Create `ImGuiRenderer` class with DX12 backend
- Integrate into main render loop (render UI after main scene)
- Test with basic demo window

**6.2: Creature Inspector Panel**
- Window: "Creature Inspector"
- Dropdown to select creature (list all active organisms)
- Display sections:
  - **Basic Info**: Species, age, position, health
  - **Genetic Profile**: Display genome data (chromosomes, gene values, active loci)
  - **Visual Properties**: Current material settings, size, color
  - **Neural Network Stats**: Neuron count, synapse count, activity level

**6.3: Neural Network Visualizer**
- Window: "Neural Network View"
- 2D graph visualization of creature's neural network
- Show neurons as nodes, synapses as connections
- Color-code by activity level (real-time update)
- Options: Filter by neuron type, highlight active paths

**6.4: Breeding Control Panel**
- Window: "Breeding System"
- Select two parent creatures from dropdowns
- Display predicted offspring traits (genetic expression preview)
- "Breed" button to execute breeding
- Show offspring results with genetic comparison
- Mutation rate slider and controls

**6.5: Simulation Controls**
- Window: "Simulation Control"
- Play/Pause/Step controls
- Time scale slider (0.1x - 10x speed)
- Spawn creature buttons (by species)
- World reset button
- Performance metrics (FPS, creature count, memory usage)

**Files to Create:**
```
src/engine/ui/
├── ImGuiRenderer.h/cpp          # DX12 ImGui backend
├── panels/
│   ├── CreatureInspector.h/cpp  # Creature data display
│   ├── NeuralNetworkView.h/cpp  # NN visualization
│   ├── BreedingControl.h/cpp    # Breeding UI
│   └── SimulationControl.h/cpp  # Simulation controls
└── UIManager.h/cpp              # Panel management
```

---

## Phase 7: Procedural Animation System

### Objective
Create a dual-layer animation system: NN-driven skeletal animation AND genetic mesh deformation for full biological realism.

### Implementation Steps

**7.1: Skeletal Rig Generation**
- Procedurally generate bone hierarchy based on species:
  - **Chordata**: Spine, limbs, head, tail
  - **Arthropoda**: Segmented body, jointed legs, antennae
  - **Mollusca**: Tentacles, foot, mantle (if applicable)
- Store as `Skeleton` class with bone hierarchy
- Each bone: position, rotation, parent index, genetic influence parameters

**7.2: Neural Network-Driven Animation**
- Map neural network outputs to bone rotations
- Example mappings:
  - Output neuron 0-3: Leg joint angles (walk cycle)
  - Output neuron 4-6: Spine curvature
  - Output neuron 7-9: Head/tail movement
- Use activity-based animation: creature moves based on what its NN "wants"
- Smooth interpolation between frames

**7.3: Genetic Mesh Deformation**
- Vertex shader reads genetic parameters
- Deform mesh based on:
  - Muscle flexing (based on activity level)
  - Breathing animation (chest expansion)
  - Wing flapping (if genetic wing parameter exists)
  - Tail swaying (genetic tail length parameter)
- Use vertex displacement in shader (GPU-accelerated)

**7.4: Animation Blending**
- Combine skeletal animation + mesh deformation
- Blend weights based on:
  - Creature state (idle, walking, running, attacking)
  - Genetic expression strength
  - Neural network activity patterns
- State machine for animation transitions

**7.5: Animation Data Pipeline**
- Extend creature mesh data to include bone weights
- Modify `ProceduralMeshRenderer` to support skeletal animation
- Add uniform buffers for bone matrices (updated per frame)
- Optimize with instanced rendering for same-species creatures

**Files to Create:**
```
src/engine/animation/
├── Skeleton.h/cpp               # Bone hierarchy
├── AnimationController.h/cpp    # NN-driven animation
├── MeshDeformer.h/cpp           # Genetic vertex animation
├── AnimationBlender.h/cpp       # Combine animation layers
└── AnimationState.h             # State machine definitions
```

**Files to Modify:**
- `GeneticsGameEngine/src/graphics/PBRVertex.hlsl`: Add bone matrix uniform buffer
- `GeneticsGameEngine/src/engine/procedural/mesh/ProceduralMeshRenderer.h`: Add skeletal support
- `GeneticsGameEngine/src/engine/genetics/taxonomy/*.h`: Add skeletal generation methods

---

## Phase 8: Game Simulation & Creature Behaviors

### Objective
Implement autonomous creature behaviors driven by neural networks, creating a living ecosystem.

### Implementation Steps

**8.1: Behavior System**
- Base `Behavior` class with virtual methods
- Implement behaviors:
  - **Wander**: Random movement with NN steering
  - **Seek**: Move toward target (food, mate, shelter)
  - **Flee**: Avoid predators/threats
  - **Feed**: Consume food sources, gain energy
  - **Rest**: Recover energy when idle
  - **Mate**: Seek compatible breeding partners

**8.2: Neural Network Integration**
- NN outputs control behavior selection and execution
- Example architecture:
  - Input layer: Sensor data (nearby creatures, food, threats)
  - Hidden layer: Decision making
  - Output layer: Movement commands, behavior triggers
- Reinforcement learning: Reward survival, energy gain, successful breeding

**8.3: World Interaction**
- Creatures interact with environment:
  - Food spawning system (plants, prey)
  - Energy system (creatures lose energy over time, must eat)
  - Death/respawn (creatures die when energy hits 0)
  - Territory marking (pheromone trails, visual markers)

**8.4: Population Dynamics**
- Birth/death rates based on resources
- Genetic diversity tracking
- Species competition (predator-prey relationships)
- Ecosystem balance monitoring

**8.5: Simulation Events**
- Log significant events (births, deaths, mutations, breeding)
- Event viewer in UI panel
- Statistics tracking (population graphs, genetic diversity charts)

**Files to Create:**
```
src/game/behaviors/
├── Behavior.h/cpp               # Base behavior class
├── WanderBehavior.h/cpp
├── SeekBehavior.h/cpp
├── FeedBehavior.h/cpp
├── MatingBehavior.h/cpp
└── BehaviorController.h/cpp     # Behavior selection logic
src/game/simulation/
├── WorldSimulation.h/cpp        # Main simulation loop
├── FoodSystem.h/cpp             # Food spawning/consumption
├── PopulationTracker.h/cpp      # Statistics tracking
└── SimulationEvent.h            # Event logging
```

---

## Phase 9: World & Environment Systems

### Objective
Create the world creatures inhabit: terrain, ecosystems, environmental effects.

### Implementation Steps

**9.1: Terrain Generation**
- Procedural terrain using noise functions
- Multiple biomes (forest, desert, mountains, water)
- Genetic mapping: terrain parameters driven by world seed
- LOD system for terrain rendering

**9.2: Ecosystem Zones**
- Different areas support different species
- Resource distribution (food, water, shelter)
- Climate system (affects creature behavior)
- Dynamic weather (rain, wind, temperature)

**9.3: Environmental Effects**
- Particle systems (rain, snow, dust, fireflies)
- Water bodies (lakes, rivers, oceans with reflection)
- Vegetation (trees, grass, bushes - procedural placement)
- Day/night cycle UI enhancement (visible sun/moon, stars)

**9.4: World Persistence**
- Save/load world state
- Creature genetic lineage tracking across sessions
- World evolution over time (seasons, ecological succession)

**9.5: Camera Integration**
- Multiple camera modes (free, follow creature, overview)
- Smooth transitions between cameras
- Screenshot/video recording functionality

**Files to Create:**
```
src/game/world/
├── TerrainGenerator.h/cpp       # Procedural terrain
├── BiomeSystem.h/cpp            # Biome definitions
├── WeatherSystem.h/cpp          # Dynamic weather
├── VegetationSystem.h/cpp       # Procedural plants
└── WorldState.h/cpp             # Save/load system
src/engine/effects/
├── ParticleSystem.h/cpp         # Particle effects
├── WaterRenderer.h/cpp          # Water rendering
└── SkySystem.h/cpp              # Enhanced sky (expand current)
```

---

## Phase 10: Advanced Rendering & Effects (Deferred)

### Objective
Fix shadow mapping and add post-processing effects for visual polish.

### Implementation Steps

**10.1: Shadow Map Debugging**
- Diagnose `DXGI_ERROR_DEVICE_REMOVED` GPU crash
- Systematic investigation:
  - Test shadow map PSO with minimal geometry
  - Validate resource barriers with D3D12 debug layer
  - Check root signature compatibility
  - Verify shader input/output matching
- Implement working shadow mapping once root cause found

**10.2: Post-Processing Pipeline**
- Bloom effect (HDR glow)
- Screen Space Ambient Occlusion (SSAO)
- Tone mapping (Reinhard, ACES)
- Color grading (contrast, saturation, vignette)
- Depth of field (bokeh effect)

**10.3: Visual Enhancements**
- Anti-aliasing (TAA or FXAA)
- Motion blur (for fast-moving creatures)
- God rays (volumetric lighting from sun)
- Atmospheric scattering (sky dome enhancement)

**Files to Modify:**
- `GeneticsGameEngine/src/graphics/GraphicsEngine.cpp`: Shadow map fixes
- Create: `src/engine/rendering/postprocess/` directory for effects

---

## Implementation Order & Dependencies

```
Phase 6 (UI Tools) 
    ↓
Phase 7 (Animation) - Requires: Phase 6 for debugging
    ↓
Phase 8 (Simulation) - Requires: Phase 7 (animated creatures), Phase 6 (controls)
    ↓
Phase 9 (World) - Requires: Phase 8 (creatures to populate world)
    ↓
Phase 10 (Rendering) - Independent, can be done anytime
```

## Estimated Timeline

- **Phase 6**: 2-3 weeks (UI integration + panels)
- **Phase 7**: 3-4 weeks (skeletal rig + animation + deformation)
- **Phase 8**: 3-4 weeks (behaviors + simulation + population)
- **Phase 9**: 4-5 weeks (terrain + ecosystem + effects)
- **Phase 10**: 2-3 weeks (shadows + post-processing)

**Total**: 14-19 weeks for complete game systems

---

## Technical Debt to Address

1. **Shadow Mapping**: Currently disabled, needs debugging (Phase 10)
2. **Performance Optimization**: Profile and optimize rendering pipeline
3. **Memory Management**: Implement proper resource pooling for creatures
4. **Testing**: Add unit tests for genetics, neural networks, behaviors

---

## Future Considerations (Beyond Phase 10)

- Multiplayer support (networked creature simulation)
- Mod support (custom species, behaviors, world generators)
- VR integration (immersive creature inspection)
- Mobile port (touch controls, optimized rendering)
- Sound system (creature vocalizations, ambient audio)

---

## Cross-Cutting Concerns

### Performance Requirements
- Target: 60 FPS at 1080p with 100+ creatures
- GPU memory: < 2GB VRAM usage
- CPU: < 70% utilization on modern quad-core CPUs

### Folder Organization Philosophy
- `src/engine/`: Core engine systems (rendering, physics, audio, input)
- `src/game/`: Game-specific logic (genetics, creatures, world, gameplay)
- `src/framework/`: Cross-cutting utilities (memory management, math, containers)
- `src/tools/`: Development tools (profiling, debugging, asset importers)

### Build System Requirements
- CMake configuration with Visual Studio 2022 support
- Windows SDK 10.0.26100.0 or later
- DirectX 12 API targeting
- Static linking for release builds

### Quality Assurance
- Automated unit tests for genetic operations
- Performance regression testing for rendering pipeline
- Visual validation tests for PBR materials
- Stress testing for neural network scalability