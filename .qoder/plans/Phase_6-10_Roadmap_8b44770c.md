# Phase 6-10: Complete Game Systems Roadmap

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
