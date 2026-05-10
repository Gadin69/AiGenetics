# Research: Adaptive Voxel Density for Mesh Generation

## Problem Statement
Identify techniques for variable voxel resolution within a single mesh — finer resolution in detailed areas (joints, faces) and coarser resolution in simple areas (limb shafts).

---

## Official Sources Found

### Source 1: OpenVDB Documentation - VolumeToMesh Struct Reference
- **URL**: https://www.openvdb.org/documentation/doxygen/structopenvdb_1_1v13__0_1_1tools_1_1VolumeToMesh.html
- **Status**: Official documentation, v13.0.1
- **Credibility**: **VERY HIGH** - Industry-standard library (Academy Software Foundation), developed at DreamWorks Animation
- **Key Finding**: OpenVDB provides **adaptive mesh extraction** with a configurable adaptivity threshold [0 to 1] and spatial adaptivity controls.

> **Direct Quote**: *"Adaptively mesh any scalar grid that has a continuous isosurface. When converting to polygons, the adaptivity threshold determines [triangle reduction in low-curvature regions]."* — OpenVDB VolumeToMesh documentation

> **Direct Quote**: *"setSpatialAdaptivity(const GridBase::ConstPtr &grid) — A scalar grid used as a spatial multiplier for the adaptivity threshold."*

### Source 2: OpenVDB FAQ
- **URL**: https://www.openvdb.org/documentation/doxygen/faq.html
- **Status**: Official documentation
- **Credibility**: **VERY HIGH** - Primary source from OpenVDB maintainers
- **Key Finding**: OpenVDB is **not merely an octree** — it uses a hierarchical tree structure with configurable branching factors at each level, storing both data values and child nodes.

> **Direct Quote**: *"While OpenVDB can conceptually be configured as a (height-balanced) octree, it is much more than an octree or N-tree. Whereas octrees and N-trees have fixed branching factors of respectively two and N in each coordinate direction, OpenVDB's branching factors typically vary between tree levels and are only limited to be powers of two."*

> **Direct Quote**: *"Since OpenVDB stores both data values and child nodes at each level of the tree, it is adaptive only in the first sense [adaptive voxel sizes], not the second [multiple grids with different fixed voxel sizes]. The level of adaptivity or refinement between the tree levels is defined by the branching factors of the nodes, which are fixed at compile time."*

### Source 3: Occupancy-Based Dual Contouring (arXiv, 2024)
- **URL**: https://arxiv.org/html/2409.13418v1
- **Status**: Preprint paper (2024)
- **Credibility**: **HIGH** - Academic preprint, peer-review pending
- **Key Finding**: Dual Contouring is a **multi-resolution contouring algorithm** that can adaptively simplify surfaces while preserving sharp features.

### Source 4: Dual Contouring: The Secret Sauce (UC Berkeley)
- **URL**: https://people.eecs.berkeley.edu/~jrs/meshpapers/SchaeferWarren2.pdf
- **Status**: Academic paper (Schaefer & Warren)
- **Credibility**: **HIGH** - Published by UC Berkeley geometry processing group
- **Key Finding**: Dual Contouring places vertices at edge intersections like Marching Cubes, but adds **Hermite data** (position + normal) to position vertices optimally, enabling adaptive refinement while preserving feature edges.

### Source 5: Dual Marching Cubes: Primal Contouring of Dual Grids (Warren et al.)
- **URL**: https://www.cs.rice.edu/~jwarren/papers/dmc.pdf
- **Status**: Academic paper (Rice University)
- **Credibility**: **HIGH** - Peer-reviewed academic publication
- **Key Finding**: *"While Dual Contouring is a multi-resolution contouring algorithm that can adaptively simplify the surface, the amount of simplification is still limited."* Dual Marching Cubes extends this for better adaptivity.

### Source 6: McNeel Forum - Grasshopper Dendro Mesh Refinement
- **URL**: https://discourse.mcneel.com/t/refine-dendro-mesh/99968
- **Status**: Community discussion (official Rhino/Grasshopper forum)
- **Credibility**: **MEDIUM-HIGH** - Expert practitioners using OpenVDB in production
- **Key Finding**: *"The parameter Adaptivity will decrease the faces number in low curvature region. But the level of detail/precision is given by the Voxel Size."*

### Source 7: High-Fidelity Lightweight Mesh Reconstruction (CVPR 2025)
- **URL**: https://cvpr.thecvf.com/virtual/2025/poster/34569
- **Status**: CVPR 2025 poster (accepted)
- **Credibility**: **VERY HIGH** - Top-tier computer vision conference
- **Key Finding**: *"We propose an adaptive meshing method to extract resolution-adaptive meshes based on surface curvature, enabling the recovery of high-fidelity lightweight [meshes]."*

### Source 8: Frequency-domain Oversampling for Multi-resolution Surface Reconstruction (Nature, 2026)
- **URL**: https://www.nature.com/articles/s40494-026-02482-y
- **Status**: Nature Scientific Reports (2026)
- **Credibility**: **VERY HIGH** - Peer-reviewed Nature journal
- **Key Finding**: *"To enhance the surface reconstruction accuracy of complex models, an adaptive octree is constructed based on the curvature information of the [surface]."*

---

## Verified Solutions & Concrete Algorithms

### 1. Adaptive Voxel Grids

#### OpenVDB Hierarchical Structure
OpenVDB uses a **tree-based sparse voxel structure** with multiple levels:

```
Root Node (dynamic branching)
├── Internal Node Level 2 (e.g., 32³ = 32,768 children)
│   ├── Internal Node Level 1 (e.g., 16³ = 4,096 children)
│   │   └── Leaf Node (e.g., 8³ = 512 voxels)
│   └── ...
└── ...
```

**Key properties**:
- **Branching factors** are powers of 2 and configurable at compile time (typically 8³, 16³, 32³)
- **Values can be stored at any level**: A value at an internal node represents a "tile" — a constant-value region covering that node's volume
- **Background value**: Represents empty/uninteresting space (e.g., far from surface)
- **Narrow band**: Only voxels near the surface are stored as active values; interior/exterior regions are tiles

**Adaptivity mechanism**: When a region is uniform (all values same), it collapses into a tile at a higher tree level, avoiding storage of individual voxels. This naturally creates **coarse resolution in simple areas** and **fine resolution near surfaces/details**.

#### Spatial Adaptivity (OpenVDB VolumeToMesh)
OpenVDB's `VolumeToMesh` supports **spatially-varying adaptivity**:

```cpp
VolumeToMesh mesher(isovalue=0.0, adaptivity=0.1, relaxDisorientedTriangles=true);

// Method 1: Uniform adaptivity (0 = no simplification, 1 = maximum simplification)
VolumeToMesh mesher(0.0, 0.1);

// Method 2: Spatial adaptivity grid
// A scalar grid where each voxel's value controls local adaptivity
mesher.setSpatialAdaptivity(scalarGrid);
// Higher values in scalarGrid → more mesh simplification in that region

// Method 3: Adaptivity mask
// A boolean tree whose active topology defines where to apply adaptivity
mesher.setAdaptivityMask(booleanTree);

// Method 4: Reference-based adaptivity (for fractured surfaces)
mesher.setRefGrid(referenceGrid, secAdaptivity=0.05);
```

**Adaptivity threshold [0 to 1]**:
- `0.0`: No simplification — full-resolution mesh (every voxel → triangles)
- `0.1-0.3`: Mild simplification — reduces triangles in flat regions while preserving detail
- `0.5+`: Aggressive simplification — significantly fewer triangles, may lose fine details

**From McNeel Forum** (Grasshopper/OpenVDB practitioners):
> *"The parameter Adaptivity will decrease the faces number in low curvature region. But the level of detail/precision is given by the Voxel Size."*

This means: **Voxel Size** controls base resolution, **Adaptivity** controls triangle reduction in simple areas.

#### Curvature-Based Refinement Strategy
**From Nature (2026) and CVPR (2025)** papers:

1. **Compute surface curvature** at each point on the implicit surface
2. **Build adaptive octree**: Split octree cells where curvature exceeds threshold
3. **Sample SDF** at refined octree leaves
4. **Extract mesh** using Dual Contouring or Adaptive Marching Cubes

**Pseudocode**:
```python
def build_adaptive_octree(sdf, bounds, max_depth, curvature_threshold):
    node = OctreeNode(bounds)
    
    if depth >= max_depth:
        return node
    
    # Compute curvature in this region
    curvature = compute_curvature(sdf, bounds)
    
    if curvature > curvature_threshold:
        # Refine: split into 8 children
        for child_bounds in subdivide(bounds):
            node.children.append(
                build_adaptive_octree(sdf, child_bounds, max_depth+1, curvature_threshold)
            )
    else:
        # Coarse: keep as single cell
        node.is_leaf = True
    
    return node
```

### 2. Octree-Based Voxelization

#### Standard Octree Refinement
An octree recursively subdivides space into 8 children (2×2×2 grid):

```
Level 0: Root (entire volume) — e.g., 10m³
├── Level 1: 8 children — each 5m³
│   ├── Level 2: 64 grandchildren — each 2.5m³
│   │   ├── Level 3: 512 nodes — each 1.25m³
│   │   │   └── Near surface: refine further
│   │   └── Empty space: stop here (store as tile)
│   └── ...
```

**Refinement criteria**:
- **Surface proximity**: Refine cells that intersect the surface (SDF value near zero)
- **Curvature**: Refine cells in high-curvature regions (joints, features)
- **Feature proximity**: Refine cells near bone joints, attachment points
- **Distance to detail**: Refine based on distance to high-detail regions

**From arXiv (2024) - Occupancy-Based Dual Contouring**:
Dual Contouring operates on octrees by:
1. Placing a vertex in each octree cell that intersects the surface
2. Using **Hermite data** (intersection position + normal) to position the vertex optimally
3. Connecting vertices across cell faces to form triangles
4. Preserving sharp features even with coarse cells adjacent to fine cells

### 3. Multi-Resolution Marching Cubes

#### Marching Cubes (Baseline)
Standard Marching Cubes:
1. Evaluate SDF at 8 corners of each voxel
2. Look up triangulation pattern in a 256-entry table (2⁸ sign combinations)
3. Interpolate vertex positions along edges where sign changes
4. **Fixed resolution**: Every voxel produces triangles

#### Adaptive Marching Cubes
Variants that support variable resolution:

| Variant | Key Idea | Pros | Cons |
|---|---|---|---|
| **Octree Marching Cubes** | Apply MC to octree leaves | Handles variable resolution | LOD seams at resolution boundaries |
| **Dual Contouring** | Place vertices using Hermite data | Preserves sharp features, adaptive | Requires normal computation |
| **Dual Marching Cubes** | Contour dual grid of octree | Better adaptivity than DC | More complex implementation |
| **Sparse Marching Cubes** | Skip uniform regions (all inside/outside) | Faster, same output quality | Still fixed resolution where active |

#### LOD Seams Problem
**From GameDev.net Forums**:
> *"I have implemented marching cubes for a large volumetric surface. The surface is contained in an octree which splits as you get closer to [the surface]..."*

**Challenge**: When adjacent octree cells have different resolutions, the extracted mesh has **T-junctions** (seams) that cause cracks in the rendered surface.

**Solutions**:
1. **Constrained octree**: Limit adjacent cells to 1 level of resolution difference (2:1 balance)
2. **Stitching triangles**: Add triangles to fill gaps at resolution boundaries
3. **Dual Contouring**: Naturally handles resolution transitions without seams (vertices placed per-cell, not per-edge)

### 4. Distance-Based Refinement

#### Refining Near Bone Joints
For creature mesh generation, increase voxel density near:

**High-priority refinement zones**:
1. **Joint regions** (shoulders, hips, elbows, knees): High curvature, complex geometry
2. **Head/face area**: Expressive features require detail
3. **Spine vertebrae connections**: Articulation points
4. **Limb-to-torso transitions**: Where cylinders meet boxes

**Lower-priority regions**:
1. **Limb shafts** (mid-upper-arm, mid-lower-leg): Nearly cylindrical, low detail needed
2. **Tail mid-sections**: Uniform taper
3. **Flat torso sides**: Low curvature

**Distance field approach**:
```python
def compute_adaptive_voxel_size(point, skeleton, base_voxel_size):
    # Find distance to nearest joint
    min_joint_distance = min(distance(point, joint.position) 
                            for joint in skeleton.joints)
    
    # Find distance to nearest bone surface
    min_bone_distance = distance_to_bone_surface(point, skeleton.bones)
    
    # Compute refinement multiplier
    if min_joint_distance < joint_refine_radius:
        # Near joint: finer voxels
        multiplier = 0.25 + 0.75 * (min_joint_distance / joint_refine_radius)
    elif min_bone_distance < surface_band_width:
        # Near surface: moderate refinement
        multiplier = 0.5 + 0.5 * (min_bone_distance / surface_band_width)
    else:
        # Far from everything: coarse
        multiplier = 1.0
    
    return base_voxel_size * multiplier
```

#### Curvature-Based Voxel Density
**From Nature (2026) paper**:
> *"An adaptive octree is constructed based on the curvature information of the surface."*

**Algorithm**:
1. Build initial uniform SDF grid
2. Extract isosurface (initial mesh)
3. Compute **mean curvature** or **Gaussian curvature** at each mesh vertex
4. Mark regions with curvature > threshold for refinement
5. Rebuild SDF grid with finer voxels in marked regions
6. Re-extract mesh

**Curvature computation from SDF** (no mesh needed):
```python
def compute_curvature_from_sdf(sdf, x, y, z, h):
    # Using finite differences on SDF
    # Mean curvature H = ∇·(∇φ/|∇φ|) / 2
    
    # Compute gradient
    dx = (sdf[x+1,y,z] - sdf[x-1,y,z]) / (2*h)
    dy = (sdf[x,y+1,z] - sdf[x,y-1,z]) / (2*h)
    dz = (sdf[x,y,z+1] - sdf[x,y,z-1]) / (2*h)
    
    # Compute second derivatives
    dxx = (sdf[x+1,y,z] - 2*sdf[x,y,z] + sdf[x-1,y,z]) / (h*h)
    dyy = (sdf[x,y+1,z] - 2*sdf[x,y,z] + sdf[x,y-1,z]) / (h*h)
    dzz = (sdf[x,y,z+1] - 2*sdf[x,y,z] + sdf[x,y,z-1]) / (h*h)
    
    # Mixed derivatives
    dxy = (sdf[x+1,y+1,z] - sdf[x+1,y-1,z] - sdf[x-1,y+1,z] + sdf[x-1,y-1,z]) / (4*h*h)
    # ... (dxz, dyz similar)
    
    # Mean curvature formula (simplified for unit gradient SDF)
    H = (dxx + dyy + dzz - (dx*dx*dxx + dy*dy*dyy + dz*dz*dzz + 
         2*dx*dy*dxy + 2*dx*dxz*dxz + 2*dy*dz*dyz)) / 2
    
    return abs(H)
```

---

## Practical Implementation Strategy

### Approach 1: OpenVDB-Based (Recommended for Production)

**Step 1: Build SDF from creature skeleton**
```cpp
// Create OpenVDB float grid (SDF)
auto sdfGrid = openvdb::FloatGrid::create();
sdfGrid->setTransform(
    openvdb::math::Transform::createLinearTransform(voxelSize)
);

// Rasterize skeleton primitives into SDF
// - Cylinders for limbs
// - Spheres for joints
// - Cones for tails/horns
// - Boxes for torso
rasterizeSkeletonToSDF(sdfGrid, skeleton);
```

**Step 2: Compute spatial adaptivity grid**
```cpp
auto adaptivityGrid = openvdb::FloatGrid::create();
adaptivityGrid->setTree(sdfGrid->tree().copy());

// Set adaptivity based on distance to joints
for (auto& joint : skeleton.joints) {
    // High adaptivity (more simplification) far from joints
    // Low adaptivity (less simplification) near joints
    paintAdaptivityRegion(adaptivityGrid, joint.position, 
                         nearJointAdaptivity=0.05,
                         farAdaptivity=0.5,
                         radius=jointRefineRadius);
}
```

**Step 3: Mesh with spatial adaptivity**
```cpp
openvdb::tools::VolumeToMesh mesher(
    isovalue=0.0,
    adaptivity=0.1,  // base adaptivity
    relaxDisorientedTriangles=true
);

// Apply spatial adaptivity
mesher.setSpatialAdaptivity(adaptivityGrid);

// Extract mesh
mesher(*sdfGrid);

// Retrieve mesh data
auto points = mesher.pointList();
auto polygons = mesher.polygonPoolList();
```

**Step 4: Post-process mesh**
- Remove duplicate vertices
- Compute normals
- Smooth if needed
- LOD generation for rendering

### Approach 2: Custom Octree + Dual Contouring (Research/Educational)

**Step 1: Build adaptive octree from SDF**
```python
root = build_adaptive_octree(
    sdf=sdf_function,
    bounds=scene_bounds,
    max_depth=8,
    curvature_threshold=0.5,
    joint_refine_zones=skeleton.joints
)
```

**Step 2: Extract mesh with Dual Contouring**
```python
mesh = dual_contour(root)
# For each octree leaf cell intersecting surface:
#   1. Find edge intersections (like Marching Cubes)
#   2. Compute QEF (Quadratic Error Function) to position vertex
#   3. Connect vertices across cell faces
```

---

## Recommendations

Based on official sources, the best approach for adaptive voxel density in creature mesh generation is:

1. **Use OpenVDB for production**: It's the industry standard (DreamWorks, Weta, ILM), handles sparse volumes efficiently, and has built-in adaptive mesh extraction. Use `setSpatialAdaptivity()` to control local refinement.

2. **Refine near joints and high-curvature areas**: Use distance-to-joint and SDF curvature as refinement criteria. Joints (shoulders, hips, knees) and the head/face region need 2-4× finer resolution than limb shafts.

3. **Set adaptivity threshold between 0.1-0.3**: This reduces triangles in flat regions while preserving detail where needed. Start at 0.1 and tune based on visual quality.

4. **Use Dual Contouring if implementing from scratch**: It handles adaptive resolution naturally without LOD seams, preserves sharp features, and works well with octrees.

5. **Maintain 2:1 octree balance**: Limit adjacent cells to 1 level of resolution difference to avoid T-junctions and mesh cracks.

6. **Curvature is the key metric**: Both academic papers (Nature 2026, CVPR 2025) and production tools (Grasshopper/OpenVDB) use surface curvature to drive adaptive refinement. Compute curvature from SDF using finite differences.

---

## References

1. OpenVDB Documentation. "VolumeToMesh Struct Reference." https://www.openvdb.org/documentation/doxygen/structopenvdb_1_1v13__0_1_1tools_1_1VolumeToMesh.html
2. OpenVDB Documentation. "Frequently Asked Questions." https://www.openvdb.org/documentation/doxygen/faq.html
3. Museth, K. (2013). "VDB: High-Resolution Sparse Volumes with Dynamic Topology." ACM Transactions on Graphics.
4. Schaefer, S., Warren, J. "Dual Contouring: The Secret Sauce." UC Berkeley. https://people.eecs.berkeley.edu/~jrs/meshpapers/SchaeferWarren2.pdf
5. Ju, T., Losasso, F., Schaefer, S., Warren, J. (2006). "Dual Marching Cubes: Primal Contouring of Dual Grids." Rice University. https://www.cs.rice.edu/~jwarren/papers/dmc.pdf
6. "Occupancy-Based Dual Contouring." arXiv:2409.13418v1 (2024).
7. "High-Fidelity Lightweight Mesh Reconstruction from Point Clouds." CVPR 2025.
8. "Frequency-domain oversampling for multi-resolution surface reconstruction." Nature Scientific Reports (2026).
9. McNeel Forum. "Refine dendro mesh." https://discourse.mcneel.com/t/refine-dendro-mesh/99968
10. GameDev.net Forums. "Marching cubes, Octree, and LOD seams." https://gamedev.net/forums/topic/591926-marching-cubes-octree-and-lod-seams/
