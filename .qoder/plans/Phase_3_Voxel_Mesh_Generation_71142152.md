# Phase 3: Voxel-Based Procedural Mesh Generation

## Implementation Strategy

**Core Principle**: Build and verify each component individually before moving to the next. Use tables from `tables.txt` exclusively - copy-paste all lookup tables, never from memory.

**Architecture**: Genome → GeneticMapper → CreatureParams → VoxelGrid → GPU Compute Shader (Marching Cubes) → Mesh → DX12 Rendering → LOD System → Mesh Optimization

---

## Task 1: VoxelGrid Data Structure

**Files to Create**:
- `src/engine/procedural/voxel/VoxelGrid.h` - Header with Voxel and VoxelGrid structs
- `src/engine/procedural/voxel/VoxelGrid.cpp` - Implementation

**Implementation Details**:
- Copy-paste Voxel and VoxelGrid structs from **tables.txt TABLE 6** (lines 398-410)
- Flattened 3D array: `data[x + y*sizeX + z*sizeX*sizeY]`
- Support configurable grid sizes: 32x32x32, 64x64x64, 128x128x128 (from TABLE 6)
- Methods: `GetDensity(x,y,z)`, `SetDensity(x,y,z,value)`, `GetVoxel(x,y,z)`, `SetVoxel(x,y,z,voxel)`
- Include scalar field storage: `float* scalarField` for marching cubes input
- Add `AllocateGrid(size)` method to dynamically allocate grid based on genetic locus 0x7G8H

**Verification**: Write test to allocate 64x64x64 grid, set density values, read them back correctly.

---

## Task 2: CreatureParams Structure & GeneticMapper

**Files to Create**:
- `src/engine/procedural/generation/CreatureParams.h` - Parameter structure
- `src/engine/procedural/generation/GeneticMapper.h` - Header
- `src/engine/procedural/generation/GeneticMapper.cpp` - Implementation

**Implementation Details**:

**CreatureParams.h**:
```cpp
struct CreatureParams {
    // From TABLE 12 (lines 633-644): Complete genetic loci mapping
    float scaleFactor;         // Locus 0x1A2B, range 0.5-3.0
    int colorPaletteIndex;     // Locus 0x3C4D, range 0-7
    int limbCount;             // Locus 0x5E6F, range 1-8
    int voxelResolution;       // Locus 0x7G8H, values 32/64/128
    float surfaceSmoothness;   // Locus 0x9I0J, range 0-100
    float internalCavity;      // Locus 0x1K2L, range 0.0-1.0
    float roughness;           // Locus 0x3M4N, PBR parameter 0.0-1.0
    float metallic;            // Locus 0x5O6P, PBR parameter 0.0-1.0
    float subsurfaceScattering;// Locus 0x7Q8R, range 0.0-1.0
    
    // Body plan parameters (from TABLE 7)
    DirectX::XMFLOAT3 bodyCenter;
    DirectX::XMFLOAT3 bodyRadii;
    DirectX::XMFLOAT3 headCenter;
    float headRadius;
    int taxonomyType; // 0=Chordata, 1=Arthropoda, 2=Mollusca
};
```

**GeneticMapper.cpp**:
- Method: `CreatureParams MapGenomeToParams(const Genome& genome, int taxonomyType)`
- Read gene values using `genome.GetGene(locusID)->ExpressValue()` for each locus from TABLE 12
- Map normalized gene values (0.0-1.0) to parameter ranges:
  - Scale: `0.5f + geneValue * 2.5f` (range 0.5-3.0)
  - Limb count: `1 + static_cast<int>(geneValue * 7.0f)` (range 1-8)
  - Voxel resolution: if value < 0.33 → 32, < 0.66 → 64, else → 128
- Implement taxonomy-specific mappings from TABLE 12 (lines 646-660)
- Set body plan parameters based on taxonomy type (Chordata/Arthropoda/Mollusca from TABLE 7)

**Verification**: Create test genome with known gene values, verify mapped parameters are in correct ranges.

---

## Task 3: Scalar Field Density Function

**Files to Create**:
- `src/engine/procedural/generation/ScalarFieldGenerator.h` - Header
- `src/engine/procedural/generation/ScalarFieldGenerator.cpp` - Implementation

**Implementation Details**:
- Copy-paste density function from **tables.txt TABLE 8** (lines 491-512): `CreatureDensityFunction`
- Implement metaball/ellipsoid functions for creature body parts:
  - Body core: ellipsoid using `1.0f - length((pos - bodyCenter) / bodyRadii)`
  - Limbs: cylinders using `DistanceToCylinder` function
  - Head: sphere using `length(pos - headCenter) / headRadius`
- Use smoothstep for smooth transitions (from TABLE 8 line 515)
- Fill VoxelGrid scalar field by evaluating density function at each voxel position
- Return `density - 0.5f` (isolevel = 0.5 from TABLE 8 line 511)

**Helper Functions**:
- `DistanceToCylinder(pos, start, end, radius)` - shortest distance from point to cylinder
- `Smoothstep(edge0, edge1, x)` - from TABLE 8 line 515

**Verification**: Generate scalar field for simple creature (sphere body + 2 limbs), verify density values > 0.5 form expected shape.

---

## Task 4: Marching Cubes CPU Implementation (Verification)

**Files to Create**:
- `src/engine/procedural/mesh/MarchingCubes.h` - Header with lookup tables
- `src/engine/procedural/mesh/MarchingCubes.cpp` - CPU implementation

**Implementation Details**:

**MarchingCubes.h**:
- Copy-paste **edgeTable[256]** from **tables.txt TABLE 1** (lines 19-52) EXACTLY
- Copy-paste **triTable[256][16]** from **tables.txt TABLE 2** (lines 67-250) EXACTLY
- Copy-paste vertex/edge indexing from **tables.txt TABLE 3** (lines 258-288) as comments
- Define `VERTEX_INTERP` function from **tables.txt TABLE 4** (lines 309-323)
- Define `CalculateNormal` function from **tables.txt TABLE 5** (lines 347-373)

**MarchingCubes.cpp**:
- Implement complete marching cubes algorithm from **tables.txt TABLE 13** (lines 674-737)
- Process each cube in VoxelGrid scalar field
- Determine cube configuration index (lines 693-701)
- Skip empty/full cubes using edgeTable (line 704)
- Intersect vertices on edges using VertexInterp (lines 708-714)
- Build triangles using triTable (lines 717-731)
- Calculate normals using central differences from TABLE 5
- Output: `struct MeshData { std::vector<XMFLOAT3> vertices; std::vector<XMFLOAT3> normals; std::vector<uint32_t> indices; }`

**Verification**: Run marching cubes on scalar field from Task 3, verify mesh has expected triangle count and topology. Visualize as wireframe in debug mode.

---

## Task 5: HLSL Compute Shaders for GPU Marching Cubes

**Files to Create**:
- `src/shaders/MarchingCubesCS.hlsl` - Compute shader

**Implementation Details**:
- Translate CPU marching cubes algorithm (Task 4) to HLSL compute shader
- Input: RWStructuredBuffer<float> scalarField, RWStructuredBuffer<float4> vertexBuffer, RWStructuredBuffer<uint32_t> indexBuffer
- Use same edgeTable and triTable (upload to GPU as constant buffers or structured buffers)
- Each thread processes one voxel cube (thread ID = cube index)
- Use groupshared memory for neighboring voxel values (reduce memory reads)
- Atomic counters for vertex/index buffer writes (InterlockedAdd)
- Output triangle count via AppendStructuredBuffer or counter buffer

**Shader Structure**:
```hlsl
[numthreads(8, 8, 8)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
    // Read 8 corner values from scalarField
    // Calculate cubeIndex
    // Lookup edgeTable, triTable
    // Intersect vertices, build triangles
    // Write to vertexBuffer, indexBuffer atomically
}
```

**Verification**: Compare CPU mesh (Task 4) with GPU mesh output - should be identical. Profile compute shader execution time.

---

## Task 6: DX12 Compute Pipeline Integration

**Files to Modify**:
- `src/graphics/GraphicsEngine.h` - Add compute pipeline members
- `src/graphics/GraphicsEngine.cpp` - Implement compute shader compilation and dispatch
- `src/graphics/GraphicsEngine.cpp` - Modify Render() to accept procedural meshes

**Files to Create**:
- `src/engine/procedural/mesh/ProceduralMeshRenderer.h` - Header
- `src/engine/procedural/mesh/ProceduralMeshRenderer.cpp` - Implementation

**Implementation Details**:

**GraphicsEngine.h additions**:
```cpp
// Compute pipeline for marching cubes
Microsoft::WRL::ComPtr<ID3D12PipelineState> m_computePipelineState;
Microsoft::WRL::ComPtr<ID3D12RootSignature> m_computeRootSignature;
Microsoft::WRL::ComPtr<ID3DBlob> m_computeShaderBlob;

// GPU buffers for compute shader
Microsoft::WRL::ComPtr<ID3D12Resource> m_scalarFieldBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBufferGPU;
Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBufferGPU;
Microsoft::WRL::ComPtr<ID3D12Resource> m_triangleCountBuffer;

// Methods
bool CreateComputePipeline();
bool CompileComputeShader();
bool CreateComputeBuffers();
void DispatchMarchingCubesComputeShader();
```

**ProceduralMeshRenderer**:
- Manage GPU resources for creature meshes (vertex buffers, index buffers)
- Method: `RenderMesh(GraphicsEngine* engine, ID3D12GraphicsCommandList* commandList)`
- Submit triangle list to existing DX12 pipeline
- Use same vertex/pixel shaders as existing triangle rendering

**GraphicsEngine::Render() modification**:
- Accept mesh data from GeneticsIntegration
- Dispatch compute shader to generate mesh on GPU
- Read back mesh data or use GPU directly for rendering
- Render procedural meshes alongside existing ground plane

**Verification**: Render single creature mesh generated by compute shader. Verify correct triangle count and visual appearance.

---

## Task 7: LOD System Implementation

**Files to Create**:
- `src/engine/procedural/voxel/VoxelLODManager.h` - Header
- `src/engine/procedural/voxel/VoxelLODManager.cpp` - Implementation

**Implementation Details**:
- Copy-paste LODConfig struct from **tables.txt TABLE 9** (lines 545-558)
- Implement LOD level selection from TABLE 9 (lines 524-543):
  - LOD0: 0-10 units, 128x128x128 voxels
  - LOD1: 10-30 units, 64x64x64 voxels
  - LOD2: 30-80 units, 32x32x32 voxels
  - LOD3: 80+ units, bounding box only
- Pre-generate all LOD levels on GPU at creature creation time:
  - Run marching cubes compute shader 3 times (once per grid size)
  - Store 3 mesh versions per creature
- Smooth LOD transition using alpha blending (TABLE 9 line 532-534)
- Method: `int SelectLODLevel(float distanceToCamera)`
- Method: `void GenerateAllLODLevels(const CreatureParams& params)`

**Verification**: Render creature at different distances, verify LOD switches at correct thresholds. Profile memory usage for 3 mesh versions.

---

## Task 8: Mesh Optimization (Quadric Error Metrics)

**Files to Create**:
- `src/engine/procedural/mesh/MeshOptimizer.h` - Header
- `src/engine/procedural/mesh/MeshOptimizer.cpp` - Implementation

**Implementation Details**:
- Copy-paste quadric error metric formulas from **tables.txt TABLE 10** (lines 568-595)
- Copy-paste edge collapse criteria from **tables.txt TABLE 11** (lines 603-623)
- Implement QEM mesh simplification:
  1. Compute quadric Q for each vertex (sum of adjacent face planes)
  2. For each edge, compute optimal collapse point and cost
  3. Insert edges into priority queue by cost
  4. Repeatedly extract min cost edge, collapse, update neighbors
  5. Stop when target triangle count reached (from TABLE 9: LOD0=50k, LOD1=10k, LOD2=2k triangles)
- Use edge collapse constraints from TABLE 11:
  - Preserve manifold property
  - Do not invert face normals
  - Preserve sharp features (dihedral angle > 120°)
- Average attributes for collapsed vertices: normals, UVs, colors (TABLE 11 lines 616-618)

**Verification**: Simplify 50k triangle mesh to 10k triangles, verify visual quality preserved. Compare with LOD levels from Task 7.

---

## Task 9: Integration with GeneticsIntegration & main.cpp

**Files to Modify**:
- `src/genetics/GeneticsIntegration.h` - Add procedural mesh generation members
- `src/genetics/GeneticsIntegration.cpp` - Implement mesh generation pipeline
- `src/core/main.cpp` - Update Application::Run() to use new systems

**Implementation Details**:

**GeneticsIntegration.h additions**:
```cpp
#include "../engine/procedural/voxel/VoxelGrid.h"
#include "../engine/procedural/generation/GeneticMapper.h"
#include "../engine/procedural/generation/ScalarFieldGenerator.h"
#include "../engine/procedural/mesh/MarchingCubes.h"
#include "../engine/procedural/mesh/ProceduralMeshRenderer.h"
#include "../engine/procedural/voxel/VoxelLODManager.h"
#include "../engine/procedural/mesh/MeshOptimizer.h"

struct CreatureInstance {
    std::unique_ptr<Organism> organism;
    CreatureParams params;
    VoxelGrid voxelGrid;
    std::vector<MeshData> lodMeshes; // One per LOD level
    DirectX::XMFLOAT3 position;
};

std::vector<CreatureInstance> m_creatureInstances;
GeneticMapper m_geneticMapper;
ScalarFieldGenerator m_scalarFieldGen;
MarchingCubes m_marchingCubes;
ProceduralMeshRenderer m_meshRenderer;
VoxelLODManager m_lodManager;
MeshOptimizer m_meshOptimizer;
```

**GeneticsIntegration::Update() pipeline**:
1. For each creature: `params = m_geneticMapper.MapGenomeToParams(organism->GetGenome(), taxonomyType)`
2. Allocate VoxelGrid: `voxelGrid.AllocateGrid(params.voxelResolution)`
3. Generate scalar field: `m_scalarFieldGen.GenerateField(voxelGrid, params)`
4. Generate meshes on GPU via compute shader (or CPU for testing)
5. Generate LOD levels: `m_lodManager.GenerateAllLODLevels(params)`
6. Optimize meshes: `m_meshOptimizer.SimplifyMesh(mesh, targetTriangleCount)`
7. Update creature position based on camera distance for LOD selection

**GeneticsIntegration::Render() pipeline**:
1. For each creature: `int lodLevel = m_lodManager.SelectLODLevel(distanceToCamera)`
2. Render selected LOD mesh: `m_meshRenderer.RenderMesh(lodMeshes[lodLevel])`

**main.cpp modifications**:
- Pass camera position to GeneticsIntegration::Update() for LOD calculations
- Call m_geneticsIntegration->Render(m_graphicsEngine) after existing render calls

**Verification**: Create 5 sample creatures with different genomes, verify all render correctly with LOD switching. Test breeding creates visually distinct offspring.

---

## Task 10: CMake Configuration & Build System

**Files to Modify**:
- `src/engine/procedural/CMakeLists.txt` - Create new file
- `src/engine/procedural/voxel/CMakeLists.txt` - Create new file
- `src/engine/procedural/mesh/CMakeLists.txt` - Create new file
- `src/engine/procedural/generation/CMakeLists.txt` - Create new file
- `src/engine/CMakeLists.txt` - Add procedural subdirectory
- `src/graphics/CMakeLists.txt` - Add shader compilation
- `src/CMakeLists.txt` - Link new libraries

**Implementation Details**:
- Create INTERFACE libraries for each procedural subsystem (matching existing genetics pattern)
- Add procedural headers to ENGINE_HEADERS in `src/engine/CMakeLists.txt`
- Add shader compilation step to GraphicsEngine CMake:
  ```cmake
  # Compile compute shader
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/shaders/MarchingCubesCS.cso
      COMMAND fxc /T cs_5_0 /Fo ${CMAKE_BINARY_DIR}/shaders/MarchingCubesCS.cso
              ${CMAKE_CURRENT_SOURCE_DIR}/shaders/MarchingCubesCS.hlsl
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/MarchingCubesCS.hlsl
  )
  ```
- Link d3d12.lib, dxgi.lib, d3dcompiler.lib to GraphicsEngine

**Verification**: Build succeeds without errors. All new files compile. Shader compiles to .cso file.

---

## Task 11: End-to-End Testing & Performance Validation

**Testing Steps**:
1. Create 5 creatures with distinct genomes (Chordata, Arthropoda, Mollusca)
2. Verify marching cubes generates smooth meshes (not blocky)
3. Test LOD switching: move camera away/toward creatures, verify smooth transitions
4. Profile performance: target 60 FPS with 100 creatures at 1080p
5. Test breeding: create offspring, verify mesh reflects combined genetic traits
6. Stress test: 100+ creatures, measure GPU memory usage (< 2GB VRAM per PLAN.md line 241)
7. Validate mesh optimization: compare simplified vs original mesh visual quality

**Success Criteria**:
- [ ] 100+ creatures rendering at 60 FPS
- [ ] LOD switching at correct distance thresholds (10, 30, 80 units)
- [ ] Mesh generation from genetic parameters works for all 3 taxonomies
- [ ] GPU compute shader produces identical mesh to CPU implementation
- [ ] Quadric error mesh simplification preserves visual quality
- [ ] VRAM usage < 2GB with 100 creatures
- [ ] CPU usage < 70% on modern quad-core (PLAN.md line 242)

---

## Implementation Order & Checkpoints

**Phase 3.1**: Tasks 1-3 (VoxelGrid + GeneticMapper + ScalarField)
- Checkpoint: Can generate scalar field from genome parameters

**Phase 3.2**: Tasks 4-5 (CPU + GPU Marching Cubes)
- Checkpoint: CPU and GPU produce identical meshes

**Phase 3.3**: Tasks 6-7 (DX12 Integration + LOD)
- Checkpoint: Creatures render with LOD switching

**Phase 3.4**: Tasks 8-9 (Mesh Optimization + Full Integration)
- Checkpoint: 100 creatures at 60 FPS with optimized meshes

**Phase 3.5**: Tasks 10-11 (Build System + Testing)
- Checkpoint: All tests pass, performance targets met

---

## Critical Constraints

1. **ALWAYS copy-paste from tables.txt** - Never write lookup tables from memory
2. **Verify each component** before proceeding to next task
3. **Maintain existing functionality** - Ground plane and FPS camera must still work
4. **Use tables.txt references explicitly** in code comments (e.g., "From tables.txt TABLE 1")
5. **Test incrementally** - Small working pieces over large broken systems